import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT

DEPENDENCIES = ["network"]
AUTO_LOAD = []

CONF_HOST = "host"
CONF_RECONNECT_INTERVAL = "reconnect_interval"
CONF_STALE_TIMEOUT = "stale_timeout"
CONF_STALE_STRATEGY = "stale_strategy"

dsmr_tcp_bridge_ns = cg.esphome_ns.namespace("dsmr_tcp_bridge")
DsmrTcpBridge = dsmr_tcp_bridge_ns.class_("DsmrTcpBridge", cg.Component)

STALE_STRATEGIES = {
    "hold_last": dsmr_tcp_bridge_ns.enum("StaleStrategy").STALE_STRATEGY_HOLD_LAST,
    "nan": dsmr_tcp_bridge_ns.enum("StaleStrategy").STALE_STRATEGY_NAN,
    "zero": dsmr_tcp_bridge_ns.enum("StaleStrategy").STALE_STRATEGY_ZERO,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DsmrTcpBridge),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_PORT): cv.port,
        cv.Optional(CONF_RECONNECT_INTERVAL, default="3s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_STALE_TIMEOUT, default="10s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_STALE_STRATEGY, default="hold_last"): cv.one_of(
            "hold_last", "nan", "zero", lower=True
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_reconnect_interval(config[CONF_RECONNECT_INTERVAL]))
    cg.add(var.set_stale_timeout(config[CONF_STALE_TIMEOUT]))
    cg.add(var.set_stale_strategy(STALE_STRATEGIES[config[CONF_STALE_STRATEGY]]))
    