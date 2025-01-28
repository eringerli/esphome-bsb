import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID
)
from esphome import automation

CODEOWNERS = ["@eringerli"]
MULTI_CONF = True

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "text_sensor"] #, "switch", "binary_sensor"
CONF_BSB_ID = "bsb_id"
CONF_PARAMETER_NUMBER = "parameter_number"
CONF_SOURCE_ADDRESS = "source_address"
CONF_DESTINATION_ADDRESS = "destination_address"
CONF_QUERY_INTERVAL = "query_interval"
CONF_BSB_TYPE= "type"

CONF_BSB_TYPE_ENUM = {
    "UINT8":0,
    "INT8":1,
    "INT16":2,
    "INT32":3,
    "TEMPERATURE":4,
    "ROOMTEMPERATURE":5
}

bsb_ns = cg.esphome_ns.namespace("bsb")
BsbComponent = bsb_ns.class_(
    "BsbComponent", cg.Component, uart.UARTDevice
)

BsbTimeoutTrigger = bsb_ns.class_(
    "BsbTimeoutTrigger", automation.Trigger
)
BsbWaitNextReadoutTrigger = bsb_ns.class_(
    "BsbWaitNextReadoutTrigger", automation.Trigger
)

def validate_baud_rate(value):
    if value > 0:
        baud_rates = [ 4800 ]
        if value not in baud_rates:
            raise cv.Invalid(f"Non standard baud rate {value}. Use one of {baud_rates}")

    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BsbComponent),
            cv.Optional(CONF_QUERY_INTERVAL, default="0.25s"): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_SOURCE_ADDRESS, default="66"
            ): cv.positive_int,
            cv.Optional(
                CONF_DESTINATION_ADDRESS, default="0"
            ): cv.positive_int,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_QUERY_INTERVAL in config:
        cg.add(var.set_query_interval(config[CONF_QUERY_INTERVAL]))

    if CONF_SOURCE_ADDRESS in config:
        cg.add(var.set_source_address(config[CONF_SOURCE_ADDRESS]))

    if CONF_DESTINATION_ADDRESS in config:
        cg.add(var.set_destination_address(config[CONF_DESTINATION_ADDRESS]))
