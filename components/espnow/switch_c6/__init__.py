from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import wifi, switch
from esphome.const import CONF_ID, CONF_NAME, CONF_CHANNEL

espnow_ns = cg.esphome_ns.namespace("espnow")
ESPNowComponent = espnow_ns.class_("ESPNowComponent", cg.Component)
ESPNowSwitchC6 = espnow_ns.class_("ESPNowSwitchC6", switch.Switch, cg.Component)

CODEOWNERS = ["@jesserockz"]
AUTO_LOAD = ["socket"]


def _validate_mac_hyphen(value):
    value = cv.string_strict(value)
    # Accept XXXX-XXXX-XXXX uppercase hex
    if len(value) == 14 and value[4] == "-" and value[9] == "-":
        return value.upper()
    raise cv.Invalid("mac must be in 'XXXX-XXXX-XXXX' hyphen uppercase format")


CONFIG_SCHEMA = switch.switch_schema(ESPNowSwitchC6).extend(
    {
        cv.GenerateID(): cv.declare_id(ESPNowSwitchC6),
        cv.GenerateID("espnow_id"): cv.use_id(ESPNowComponent),
        cv.Required("mac"): _validate_mac_hyphen,
        cv.Optional(CONF_CHANNEL, default=1): wifi.validate_channel,
        cv.Optional("need_response", default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)

    # Parent espnow component
    await cg.register_parented(var, config["espnow_id"])

    cg.add(var.set_mac(config["mac"]))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    cg.add(var.set_need_response(config["need_response"]))
