import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from . import BsbComponent, bsb_ns, CONF_BSB_ID, CONF_PARAMETER_NUMBER, CONF_BSB_TYPE_ENUM, CONF_BSB_TYPE

from esphome.const import (
    CONF_UPDATE_INTERVAL
)

CONF_FIELD_ID = "field_id"
CONF_ON_VALUE = "on_value"
CONF_OFF_VALUE = "off_value"
CONF_ENABLE_BYTE = "enable_byte"

BsbSwitch = bsb_ns.class_("BsbSwitch", switch.Switch)

CONFIG_SCHEMA = cv.All(
    switch.switch_schema(
        BsbSwitch,
    ).extend(
        {
            cv.GenerateID(CONF_BSB_ID): cv.use_id(BsbComponent),
            cv.Required(CONF_FIELD_ID): cv.positive_int,
            cv.Optional(CONF_ENABLE_BYTE, default="1"): cv.hex_int_range(0x00,0xff),
            cv.Optional(CONF_PARAMETER_NUMBER, default="0"): cv.positive_int,
            cv.Optional(CONF_BSB_TYPE, default="INT8"): cv.enum(CONF_BSB_TYPE_ENUM, upper=True, space="_"),
            cv.Optional(CONF_UPDATE_INTERVAL, default="15min"): cv.update_interval,
            cv.Optional(CONF_OFF_VALUE, default="0"): cv.float_,
            cv.Optional(CONF_ON_VALUE, default="1"): cv.float_,
        }
    ),
    cv.has_exactly_one_key(CONF_FIELD_ID),
)


async def to_code(config):
    component = await cg.get_variable(config[CONF_BSB_ID])
    var = await switch.new_switch(config)

    if CONF_FIELD_ID in config:
        cg.add(var.set_field_id(config[CONF_FIELD_ID]))

    if CONF_ON_VALUE in config:
        cg.add(var.set_on_value(config[CONF_ON_VALUE]))

    if CONF_OFF_VALUE in config:
        cg.add(var.set_off_value(config[CONF_OFF_VALUE]))

    if CONF_ENABLE_BYTE in config:
        cg.add(var.set_enable_byte(config[CONF_ENABLE_BYTE]))

    if CONF_BSB_TYPE in config:
        cg.add(var.set_value_type(config[CONF_BSB_TYPE]))

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    cg.add(component.register_number(var))
    cg.add(var.set_retry_interval(component.get_retry_interval()))
    cg.add(var.set_retry_count(component.get_retry_count()))
