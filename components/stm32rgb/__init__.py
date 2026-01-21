import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, light
from esphome.const import (
    CONF_ID,
    CONF_CHANNEL,
    CONF_BRIGHTNESS,
    CONF_COLOR,
    CONF_RED,
    CONF_GREEN,
    CONF_BLUE,
)

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@your-github-username"]   # 可改成你自己的

stm32rgb_ns = cg.esphome_ns.namespace("stm32rgb")
STM32RGBLight = stm32rgb_ns.class_(
    "STM32RGBLight", cg.Component, i2c.I2CDevice, light.LightOutput
)

# ====================
#       驗證器
# ====================

CONF_STRIP = "strip"           # 選擇第幾條燈帶 (0 或 1)

CONFIG_SCHEMA = cv.All(
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(STM32RGBLight),
            cv.Optional(CONF_STRIP, default=0): cv.enum(
                {
                    0: "Strip 1 (channels 0+1)",
                    1: "Strip 2 (channels 2+3)",
                },
                int=True,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x1A))   # 你的 STM32RGB 固定地址
)


# ====================
#     產生 C++ 程式碼
# ====================

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await light.register_light(var, config)

    # 設定是第幾條燈帶 (0 或 1)
    cg.add(var.set_strip(config[CONF_STRIP]))

    # 如果你之後要加 reset pin 或其他硬體控制腳位，可以像這樣擴充
    # if CONF_RESET_PIN in config:
    #     reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    #     cg.add(var.set_reset_pin(reset_pin))