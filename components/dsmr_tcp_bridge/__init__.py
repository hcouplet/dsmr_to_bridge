import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_HOST, CONF_ID, CONF_PORT

AUTO_LOAD = ["network"]
DEPENDENCIES = ["network"]

CONF_RECONNECT_INTERVAL = "reconnect_interval"

ns = cg.esphome_ns.namespace("dsmr_tcp_bridge")
DsmrTcpBridge = ns.class_("DsmrTcpBridge", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DsmrTcpBridge),
        cv.Required(CONF_HOST): cv.string_strict,
        cv.Optional(CONF_PORT, default=82): cv.port,
        cv.Optional(CONF_RECONNECT_INTERVAL, default="3s"): cv.positive_time_period_milliseconds,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_reconnect_interval(config[CONF_RECONNECT_INTERVAL].total_milliseconds))
