#include "aw87559.h"

namespace esphome {
namespace aw87559 {

static const char *TAG = "aw87559";

#define AW87559_ERROR_CHECK(err, msg) \
  if (!(err)) { 
    ESP_LOGE(TAG, msg); 
    this->mark_failed(); 
    return; 
  }

void AW87559::setup() {
    ESP_LOGCONFIG(TAG, "Setting up AW87559...");

    uint8_t chip_id;
    //芯片ID寄存器
    AW87559_ERROR_CHECK(this->read_byte(AW87559_REG_CHIPID, &chip_id), "Failed to read AW87559 CHIP ID");

    if (chip_id != 0x5A) {
        ESP_LOGE(TAG, "Chip ID mismatch! Expected 0x5A, got 0x%02X", chip_id);
        this->mark_failed();
        return;
    }
    //系统控制寄存器
    AW87559_ERROR_CHECK(this->write_byte(AW87559_REG_SYSCTRL, 0x38), "Failed to configure SYSCTRL register");
    //电池保护寄存器
    AW87559_ERROR_CHECK(this->write_byte(AW87559_REG_BATSAFE, 0x09), "Failed to configure BATSAFE register");
    //升压控制寄存器
    AW87559_ERROR_CHECK(this->write_byte(AW87559_REG_BATSAFE, 0x0c), "Failed to configure BATSAFE register");

    ESP_LOGI(TAG, "AW87559 setup completed successfully.");


    
} // namespace aw87559
} // namespace esphome