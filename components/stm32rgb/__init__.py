# my_component/stm32rgb/__init__.py

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, light
from esphome.const import CONF_ID

DEPENDENCIES = ['i2c', 'light']

stm32rgb_ns = cg.esphome_ns.namespace('stm32rgb')
STM32RGBLight = stm32rgb_ns.class_(
    'STM32RGBLight', cg.Component, light.LightOutput, i2c.I2CDevice
)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(STM32RGBLight),
    cv.Optional('strip', default=0): cv.int_range(min=0, max=1)
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_strip(config.get('strip', 0)))
    yield cg.register_component(var)