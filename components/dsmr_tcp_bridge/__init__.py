import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_HOST, CONF_ID, CONF_PORT

AUTO_LOAD = ["network"]
DEPENDENCIES = ["network"]

CONF_RECONNECT_INTERVAL = "reconnect_interval"
CONF_STALE_TIMEOUT = "stale_timeout"
CONF_STALE_STRATEGY = "stale_strategy"

STALE_STRATEGIES = ["hold_last", "nan", "zero"]

ns = cg.esphome_ns.namespace("dsmr_tcp_bridge")
DsmrTcpBridge = ns.class_("DsmrTcpBridge", cg.Component)
StaleStrategy = ns.enum("StaleStrategy")

STALE_STRATEGY_ENUM = {
    "hold_last": StaleStrategy.STALE_STRATEGY_HOLD_LAST,
    "nan": StaleStrategy.STALE_STRATEGY_NAN,
    "zero": StaleStrategy.STALE_STRATEGY_ZERO,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DsmrTcpBridge),
        cv.Required(CONF_HOST): cv.string_strict,
        cv.Optional(CONF_PORT, default=82): cv.port,
        cv.Optional(CONF_RECONNECT_INTERVAL, default="3s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_STALE_TIMEOUT, default="10s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_STALE_STRATEGY, default="hold_last"): cv.one_of(*STALE_STRATEGIES, lower=True),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_reconnect_interval(config[CONF_RECONNECT_INTERVAL].total_milliseconds))
    cg.add(var.set_stale_timeout(config[CONF_STALE_TIMEOUT].total_milliseconds))
    cg.add(var.set_stale_strategy(STALE_STRATEGY_ENUM[config[CONF_STALE_STRATEGY]]))
