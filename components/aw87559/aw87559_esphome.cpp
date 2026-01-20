#include "aw87559_esphome.h"

namespace esphome {
namespace aw87559 {

static const char *TAG = "AW87559";

void AW87559Component::setup() {
  ESP_LOGD(TAG, "Setting up AW87559...");

  if (reset_gpio_ != nullptr) {
    reset_gpio_->setup();
    reset_gpio_->digital_write(false);
    delay(2);
    reset_gpio_->digital_write(true);
    delay(2);
  }

  uint8_t id;
  if (!read_reg(AW87559_REG_CHIPID, &id)) {
    ESP_LOGE(TAG, "AW87559 not responding - read ID failed at address 0x%02X", address_);
    mark_failed();
    return;
  }

  if (id != AW87559_CHIPID) {
    ESP_LOGW(TAG, "AW87559 ID mismatch: expected 0x%02X, got 0x%02X", AW87559_CHIPID, id);
  }

  ESP_LOGI(TAG, "AW87559 found with ID: 0x%02X", id);

  if (!write_reg(AW87559_REG_SYSCTRL, 0x78)) {
    ESP_LOGE(TAG, "Failed to enable AW87559 PA");
    mark_failed();
    return;
  }

  ESP_LOGI(TAG, "AW87559 Audio Amplifier initialized @0x%02X", address_);
}

void AW87559Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AW87559 Audio Amplifier:");
  LOG_I2C_DEVICE(this);
  if (reset_gpio_ != nullptr) {
    LOG_PIN("  Reset GPIO: ", reset_gpio_);
  }
}

// 其他函数实现保持不变（write_bytes, write_reg, read_reg, apply_table）
...
}  // namespace aw87559
}  // namespace esphome
