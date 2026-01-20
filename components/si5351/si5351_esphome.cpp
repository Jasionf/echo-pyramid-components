#pragma once

#include "si5351_esphome.h"   // ← 改成包含你自己的头文件（而不是重复写 i2c/component）

namespace esphome {
namespace si5351 {

static const char *const TAG = "si5351";

bool Si5351Component::write_reg(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

bool Si5351Component::read_reg(uint8_t reg, uint8_t *out_value) {
  return this->read_byte(reg, out_value);
}

// 可选：实现批量写（很多驱动都喜欢用）
bool Si5351Component::write_regs(uint8_t reg, const uint8_t *data, size_t len) {
  // ESPHome 的 I2CDevice 有 write() 支持多字节
  uint8_t buffer[1 + len];
  buffer[0] = reg;
  memcpy(&buffer[1], data, len);
  esp_err_t err = this->write(buffer, 1 + len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C write failed at reg 0x%02X, err=%d", reg, err);
    return false;
  }
  return true;
}

void Si5351Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Si5351...");

  // 1. 先关闭所有输出（安全起见）
  //    寄存器 3: 输出使能控制，1=禁用，0=启用
  if (!this->write_reg(REG_OUTPUT_ENABLE_CONTROL, 0xFF)) {  // 全禁用
    ESP_LOGE(TAG, "Failed to disable outputs");
    this->mark_failed();
    return;
  }

  // 2. 关闭驱动器（推荐初始化顺序）
  uint8_t output_drivers[3] = {0x80, 0x80, 0x80};  // CLK0,1,2 都关（bit7=1 表示 off）
  if (!this->write_regs(REG_OUTPUT_DRIVERS, output_drivers, 3)) {
    ESP_LOGE(TAG, "Failed to power down output drivers");
    this->mark_failed();
    return;
  }

  // 3. 设置晶振负载电容（默认 10pF，如果你板子是 8pF/12pF/0pF 可改）
  if (!this->write_reg(REG_XTAL_LOAD_CAP, 0xC0 | (0 << 6))) {  // 0b11 = 10pF
    ESP_LOGE(TAG, "Failed to set crystal load cap");
    this->mark_failed();
    return;
  }

  // 4. 配置 Multisynth NA（通常控制 CLK0）
  uint8_t multisynth_na[8] = {0xFF, 0xFD, 0x00, 0x09, 0x26, 0xF7, 0x4F, 0x72};
  if (!this->write_regs(REG_MULTISYNTH_NA, multisynth_na, 8)) {
    ESP_LOGE(TAG, "Failed to set Multisynth NA");
    this->mark_failed();
    return;
  }

  // 5. 配置 Multisynth1（CLK1）
  uint8_t multisynth1[8] = {0x00, 0x01, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x00};
  if (!this->write_regs(REG_MULTISYNTH1, multisynth1, 8)) {
    ESP_LOGE(TAG, "Failed to set Multisynth1");
    this->mark_failed();
    return;
  }

  // 6. CLK1 控制寄存器（示例值 0x4C 表示用 Multisynth1，分频整数模式）
  if (!this->write_reg(REG_CLK1_CONTROL, 0x4C)) {
    ESP_LOGE(TAG, "Failed to set CLK1 control");
    this->mark_failed();
    return;
  }

  // 7. 复位 PLL（很重要！）
  if (!this->write_reg(REG_PLL_RESET, 0xA0)) {  // 0xA0 = reset PLLA & PLLB
    ESP_LOGE(TAG, "Failed to reset PLLs");
    this->mark_failed();
    return;
  }

  delay(10);  // 等待 PLL 锁定（经验值）

  // 8. 最后打开输出
  if (!this->write_reg(REG_OUTPUT_ENABLE_CONTROL, 0x00)) {  // 0x00 = 全开
    ESP_LOGE(TAG, "Failed to enable outputs");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "Si5351 initialized successfully");
}

void Si5351Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Si5351:");
  LOG_I2C_DEVICE(this);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Setup failed!");
  } else {
    ESP_LOGCONFIG(TAG, "  Status: OK");
    if (this->reset_gpio_ != nullptr) {
      ESP_LOGCONFIG(TAG, "  Reset pin: configured");
    } else {
      ESP_LOGCONFIG(TAG, "  Reset pin: not used (NC or soft-only)");
    }
  }
}

}  // namespace si5351
}  // namespace esphome
