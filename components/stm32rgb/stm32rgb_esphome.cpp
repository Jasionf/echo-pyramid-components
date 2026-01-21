#include "stm32rgb_esphome.h"

namespace esphome {
namespace stm32rgb {

static const char *const TAG = "stm32rgb";

void STM32RGBLight::setup() {
  ESP_LOGCONFIG(TAG, "Setting up STM32RGBLight @ 0x%02X strip %d", this->address_, this->strip_);

  delay(50);  // 等待硬件稳定

  // 简单测试通信
  if (!this->write_byte(0xFF, 0xFF)) {  // 可以换成任意寄存器读写测试
    this->mark_failed();
    ESP_LOGE(TAG, "Failed to communicate with STM32RGB");
    return;
  }

  // 可选：上电默认全灭或默认亮度
  this->write_brightness(0);

  ESP_LOGI(TAG, "STM32RGBLight initialized");
}

void STM32RGBLight::dump_config() {
  ESP_LOGCONFIG(TAG, "STM32RGBLight:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Strip: %d", this->strip_);
  ESP_LOGCONFIG(TAG, "  LEDs per strip: %d", NUM_LEDS_PER_GROUP);
  ESP_LOGCONFIG(TAG, "  State: %s", this->status_has_warning() ? "⚠ FAILED" : "OK");
}

light::LightTraits STM32RGBLight::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  traits.set_min_brightness(0.01f);   // 建议保留一点最低亮度
  traits.set_max_brightness(1.0f);
  // 如果你想支持亮度 0-100 整数，可以在这里做调整，但 ESPHome 内部用 0-1.0f 更方便
  return traits;
}

void STM32RGBLight::write_state(light::LightState *state) {
  if (this->is_failed()) return;

  float brightness = state->current_values.get_brightness() * 100.0f;
  uint8_t bri = static_cast<uint8_t>(brightness + 0.5f);  // 四舍五入到整数 0-100
  bri = std::min(bri, (uint8_t)100);

  // 先设置全局亮度
  if (!this->write_brightness(bri)) {
    ESP_LOGW(TAG, "Failed to set brightness");
    return;
  }

  if (bri == 0 || !state->current_values.is_on()) {
    // 全灭处理（可以优化为只写一次 0）
    uint8_t zero_buf[NUM_LEDS_PER_GROUP * BYTES_PER_LED] = {0};
    this->write_leds(zero_buf, sizeof(zero_buf));
    return;
  }

  // 准备颜色缓冲
  uint8_t buf[NUM_LEDS_PER_GROUP * BYTES_PER_LED]{};

  auto call = state->make_call();
  call.set_rgb(state->current_values.get_red(),
               state->current_values.get_green(),
               state->current_values.get_blue());

  for (uint8_t i = 0; i < NUM_LEDS_PER_GROUP; i++) {
    float r, g, b;
    call.get_rgb(&r, &g, &b);  // 统一颜色（单色灯带情况）

    uint8_t hardware_idx = this->remap_led_index(this->strip_ * 2, i);  // strip0 → ch0&1, strip1 → ch2&3

    // 每个组7颗，先写第一组 (ch even)
    size_t offset1 = i * BYTES_PER_LED;
    buf[offset1 + 0] = static_cast<uint8_t>(b * 255.0f);  // B
    buf[offset1 + 1] = static_cast<uint8_t>(g * 255.0f);  // G
    buf[offset1 + 2] = static_cast<uint8_t>(r * 255.0f);  // R
    buf[offset1 + 3] = 0;

    // 第二组 (ch odd) 用相同颜色（如果你想两条灯带不同颜色，需要再做两个 LightOutput）
    size_t offset2 = (NUM_LEDS_PER_GROUP + i) * BYTES_PER_LED;
    buf[offset2 + 0] = buf[offset1 + 0];
    buf[offset2 + 1] = buf[offset1 + 1];
    buf[offset2 + 2] = buf[offset1 + 2];
    buf[offset2 + 3] = 0;
  }

  this->write_leds(buf, sizeof(buf));
}

bool STM32RGBLight::write_brightness(uint8_t brightness) {
  uint8_t reg = (this->strip_ == 0) ? RGB1_BRIGHTNESS_REG_ADDR : RGB2_BRIGHTNESS_REG_ADDR;
  return this->write_byte(reg, brightness);
}

bool STM32RGBLight::write_leds(const uint8_t *colors_buffer, size_t len) {
  // 因为硬件分成两组寄存器，所以要分两次写
  const uint8_t *p = colors_buffer;

  // 第一组 (ch0 or ch2)
  uint8_t base1 = this->get_channel_base_addr(this->strip_ * 2);
  if (!this->write_bytes(base1, p, NUM_LEDS_PER_GROUP * BYTES_PER_LED))
    return false;

  // 第二组 (ch1 or ch3)
  uint8_t base2 = this->get_channel_base_addr(this->strip_ * 2 + 1);
  p += NUM_LEDS_PER_GROUP * BYTES_PER_LED;
  if (!this->write_bytes(base2, p, NUM_LEDS_PER_GROUP * BYTES_PER_LED))
    return false;

  return true;
}

uint8_t STM32RGBLight::get_channel_base_addr(uint8_t logical_channel) const {
  switch (logical_channel) {
    case 0: return RGB_CH1_I1_COLOR_REG_ADDR;
    case 1: return RGB_CH2_I1_COLOR_REG_ADDR;
    case 2: return RGB_CH4_I1_COLOR_REG_ADDR;  // 注意硬件交换
    case 3: return RGB_CH3_I1_COLOR_REG_ADDR;  // 注意硬件交换
    default: return 0xFF;  // error
  }
}

uint8_t STM32RGBLight::remap_led_index(uint8_t logical_channel, uint8_t index) const {
  // 只有 channel 0 和 1 需要反转索引
  if (logical_channel == 0 || logical_channel == 1) {
    return NUM_LEDS_PER_GROUP - 1 - index;
  }
  return index;
}

}  // namespace stm32rgb
}  // namespace esphome