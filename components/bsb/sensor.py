import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from . import BsbComponent, bsb_ns, CONF_BSB_ID, CONF_PARAMETER_NUMBER, CONF_BSB_TYPE_ENUM, CONF_BSB_TYPE

from esphome.const import (
    CONF_UPDATE_INTERVAL
)

CONF_FIELD_ID = "field_id"
CONF_DIVISOR = "divisor"
CONF_FACTOR = "factor"
CONF_ENABLE_BYTE = "enable_byte"

BsbSensor = bsb_ns.class_("BsbSensor", sensor.Sensor)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        BsbSensor,
    ).extend(
        {
            cv.GenerateID(CONF_BSB_ID): cv.use_id(BsbComponent),
            cv.Required(CONF_FIELD_ID): cv.positive_int,
            cv.Optional(CONF_ENABLE_BYTE, default="1"): cv.hex_int_range(0x00,0xff),
            cv.Optional(CONF_PARAMETER_NUMBER, default="0"): cv.positive_int,
            cv.Required(CONF_BSB_TYPE): cv.enum(CONF_BSB_TYPE_ENUM, upper=True, space="_"),
            cv.Optional(CONF_UPDATE_INTERVAL, default="15min"): cv.update_interval,
            cv.Optional(CONF_DIVISOR, default="1"): cv.float_,
            cv.Optional(CONF_FACTOR, default="1"): cv.float_,
        }
    ),
    cv.has_exactly_one_key(CONF_FIELD_ID),
)


async def to_code(config):
    component = await cg.get_variable(config[CONF_BSB_ID])
    var = await sensor.new_sensor(config)

    if CONF_FIELD_ID in config:
        cg.add(var.set_field_id(config[CONF_FIELD_ID]))

    if CONF_DIVISOR in config:
        cg.add(var.set_divisor(config[CONF_DIVISOR]))

    if CONF_FACTOR in config:
        cg.add(var.set_factor(config[CONF_FACTOR]))

    if CONF_ENABLE_BYTE in config:
        cg.add(var.set_enable_byte(config[CONF_ENABLE_BYTE]))

    if CONF_BSB_TYPE in config:
        cg.add(var.set_value_type(config[CONF_BSB_TYPE]))

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    cg.add(component.register_sensor(var))
    cg.add(var.set_retry_interval(component.get_retry_interval()))
    cg.add(var.set_retry_count(component.get_retry_count()))
