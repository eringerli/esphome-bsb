import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import BsbComponent, bsb_ns, CONF_BSB_ID, CONF_PARAMETER_NUMBER, CONF_BSB_TYPE_ENUM, CONF_BSB_TYPE

from esphome.const import (
    CONF_UPDATE_INTERVAL
)

CONF_FIELD_ID = "field_id"

BsbTextSensor = bsb_ns.class_("BsbTextSensor", text_sensor.TextSensor)

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(
        BsbTextSensor,
    ).extend(
        {
            cv.GenerateID(CONF_BSB_ID): cv.use_id(BsbComponent),
            cv.Required(CONF_FIELD_ID): cv.positive_int,
            cv.Optional(CONF_UPDATE_INTERVAL, default="15min"): cv.update_interval,
            cv.Optional(CONF_PARAMETER_NUMBER, default="0"): cv.positive_int,
        }
    ),
    cv.has_exactly_one_key(CONF_FIELD_ID),
)


async def to_code(config):
    component = await cg.get_variable(config[CONF_BSB_ID])
    var = await text_sensor.new_text_sensor(config)

    if CONF_FIELD_ID in config:
        cg.add(var.set_field_id(config[CONF_FIELD_ID]))

    if CONF_BSB_TYPE in config:
        cg.add(var.set_value_type(config[CONF_BSB_TYPE]))

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    cg.add(component.register_sensor(var))
