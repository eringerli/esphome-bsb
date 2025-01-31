import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from . import BsbComponent, bsb_ns, CONF_BSB_ID, CONF_BSB_TYPE_ENUM, CONF_BSB_TYPE, CONF_PARAMETER_NUMBER

from esphome.const import (
    CONF_ID, CONF_NAME,CONF_MAX_VALUE, CONF_MIN_VALUE, CONF_STEP, CONF_UPDATE_INTERVAL
)

CONF_FIELD_ID = "field_id"
CONF_DIVISOR = "divisor"
CONF_FACTOR = "factor"
CONF_ENABLE_BYTE = "enable_byte"
CONF_BROADCAST = "broadcast"

BsbNumber = bsb_ns.class_("BsbNumber", number.Number)

# CONFIG_SCHEMA = number.NUMBER_SCHEMA.extend({
#     cv.GenerateID(): cv.declare_id(BsbNumber),
#     cv.GenerateID(CONF_BSB_ID): cv.use_id(BsbComponent),
#     cv.Required(CONF_FIELD_ID): cv.positive_int,
#     cv.Optional(CONF_UPDATE_INTERVAL, default="15min"): cv.update_interval,
# })


CONFIG_SCHEMA = cv.All(
    number.number_schema(
        BsbNumber,
    ).extend(
        {
            cv.GenerateID(CONF_BSB_ID): cv.use_id(BsbComponent),
            cv.Required(CONF_FIELD_ID): cv.positive_int,
            cv.Optional(CONF_ENABLE_BYTE, default="1"): cv.hex_int_range(0x00,0xff), cv.Optional(CONF_PARAMETER_NUMBER, default="0"): cv.positive_int,
            cv.Required(CONF_BSB_TYPE): cv.enum(CONF_BSB_TYPE_ENUM, upper=True, space="_"),
            cv.Optional(CONF_BROADCAST, default=False): cv.boolean,
            cv.Optional(CONF_UPDATE_INTERVAL, default="15min"): cv.update_interval,
            cv.Required(CONF_MIN_VALUE): cv.float_,
            cv.Required(CONF_MAX_VALUE): cv.float_,
            cv.Required(CONF_STEP): cv.positive_float,
            cv.Optional(CONF_DIVISOR, default="1"): cv.float_,
            cv.Optional(CONF_FACTOR, default="1"): cv.float_,
        }
    ),
    cv.has_exactly_one_key(CONF_FIELD_ID),
)

async def to_code(config):
    component = await cg.get_variable(config[CONF_BSB_ID])

    config.min_value = 0
    config.max_value = 100
    config.step = 0.1
    var = await number.new_number(config,min_value=config[CONF_MIN_VALUE], max_value=config[CONF_MAX_VALUE], step=config[CONF_STEP])


    if CONF_FIELD_ID in config:
        cg.add(var.set_field_id(config[CONF_FIELD_ID]))

    if CONF_ENABLE_BYTE in config:
        cg.add(var.set_enable_byte(config[CONF_ENABLE_BYTE]))

    if CONF_BROADCAST in config:
        cg.add(var.set_broadcast(config[CONF_BROADCAST]))

    # if CONF_PARAMETER_NUMBER in config:
    #     cg.add(var.set_parameter_number(config[CONF_PARAMETER_NUMBER]))

    if CONF_DIVISOR in config:
        cg.add(var.set_divisor(config[CONF_DIVISOR]))

    if CONF_FACTOR in config:
        cg.add(var.set_factor(config[CONF_FACTOR]))

    if CONF_BSB_TYPE in config:
        cg.add(var.set_value_type(config[CONF_BSB_TYPE]))

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    cg.add(component.register_number(var))
    cg.add(var.set_retry_interval(component.get_retry_interval()))
    cg.add(var.set_retry_count(component.get_retry_count()))

