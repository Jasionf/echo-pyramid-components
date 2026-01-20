#pragma once

#include "esphome/components/audio_dac/audio_dac.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"


namespace esphome {
namespace aw87559 {
    // AW87559 Register Addresses
static const uint8_t AW87559_REG_CHIPID = 0x00;
static const uint8_t AW87559_REG_SYSCTRL = 0x01;
static const uint8_t AW87559_REG_BATSAFE = 0x02;
static const uint8_t AW87559_REG_BSTOVR = 0x03;
static const uint8_t AW87559_REG_BSTVPR = 0x04;
static const uint8_t AW87559_REG_PAGR = 0x05;
static const uint8_t AW87559_REG_PAGC3OPR = 0x06;
static const uint8_t AW87559_REG_PAGC3PR = 0x07;
static const uint8_t AW87559_REG_PAGC2OPR = 0x08;
static const uint8_t AW87559_REG_PAGC2PR = 0x09;
static const uint8_t AW87559_REG_PAGC1PR = 0x0A;

static const uint8_t AW87559_REG_PAGC1PR = 0x0B;  
static const uint8_t AW87559_REG_ADP_MODE = 0x0C;   
static const uint8_t AW87559_REG_ADPBST_TIME1  = 0x0D;   
static const uint8_t AW87559_REG_ADPBST_VTH  = 0x0F;       

static const uint8_t AW87559_CHIPID  = 0x5B;

//定义那些寄存器可读可写
static const uint8_t AW87559_REG_MAX = 16;              // 这个芯片总共只暴露了 16 个寄存器（0~15）

static const uint8_t AW87559_REG_NONE_ACCESS = 0;       // 权限：无（既不能读也不能写）
static const uint8_t AW87559_REG_RD_ACCESS   = (1 << 0); // 权限：可读         （bit0 = 1）
static const uint8_t AW87559_REG_WR_ACCESS   = (1 << 1); // 权限：可写         （bit1 = 1）

// 下面这个数组就是核心：每个元素对应一个寄存器的访问权限
const unsigned char AW87559_reg_access[AW87559_REG_MAX] = {
    [AW87559_REG_CHIPID]    = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,  // 可读可写
    [AW87559_REG_SYSCTRL]   = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_BATSAFE]   = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_BSTOVR]    = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_BSTVPR]    = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGR]      = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGC3OPR]  = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGC3PR]   = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGC2OPR]  = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGC2PR]   = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
    [AW87559_REG_PAGC1PR]   = AW87559_REG_RD_ACCESS | AW87559_REG_WR_ACCESS,
};



struct sr_map_t {
    uint32_t hz;    // Sample rate
    uint8_t  code;  // Corresponding bit
};

class AW87559 : public audio_dac::AudioDac, public Component, public i2c::I2CDevice {
public:
    void setup() override;
    void loop() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::DATA; }
   
    bool set_mute_off() override { return this->set_mute_state_(false); }

    /// @brief Enables mute for audio out
    /// @return True if successful and false otherwise
    bool set_mute_on() override { return this->set_mute_state_(true); }
    bool is_muted() override { return this->is_muted_; }


    float volume() override;

    bool set_volume(float volume) override;

    // Set sample rate for AW88298
    void set_sample_rate(uint32_t sample_rate) { this->sample_rate_ = sample_rate; }
protected:
    bool set_mute_state_(bool mute_state);

private:
    bool initialized_{false};
    float volume_{1.0f};
    bool is_muted_{false};
    uint32_t sample_rate_;
};

}  // namespace aw87559
}  // namespace esphome

