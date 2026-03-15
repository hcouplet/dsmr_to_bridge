#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <WiFiClient.h>
#include <cmath>
#include <string>

namespace esphome {
namespace dsmr_tcp_bridge {

enum StaleStrategy {
  STALE_STRATEGY_HOLD_LAST = 0,
  STALE_STRATEGY_NAN = 1,
  STALE_STRATEGY_ZERO = 2,
};

struct MeterSnapshot {
  float voltage_l1{0};
  float voltage_l2{0};
  float voltage_l3{0};

  float current_l1{0};
  float current_l2{0};
  float current_l3{0};

  float power_l1{0};
  float power_l2{0};
  float power_l3{0};
  float power_total{0};

  float apparent_power_l1{0};
  float apparent_power_l2{0};
  float apparent_power_l3{0};

  float reactive_power_l1{0};
  float reactive_power_l2{0};
  float reactive_power_l3{0};

  float power_factor_l1{0};
  float power_factor_l2{0};
  float power_factor_l3{0};

  float frequency{50.0f};

  float import_energy{0};
  float export_energy{0};
};

class DsmrTcpBridge : public Component {
 public:
  void set_host(const std::string &host) { this->host_ = host; }
  void set_port(uint16_t port) { this->port_ = port; }
  void set_reconnect_interval(uint32_t reconnect_interval_ms) { this->reconnect_interval_ms_ = reconnect_interval_ms; }
  void set_stale_timeout(uint32_t stale_timeout_ms) { this->stale_timeout_ms_ = stale_timeout_ms; }
  void set_stale_strategy(StaleStrategy stale_strategy) { this->stale_strategy_ = stale_strategy; }

  float get_v_l1() const { return this->apply_stale_strategy_(this->current_.voltage_l1); }
  float get_v_l2() const { return this->apply_stale_strategy_(this->current_.voltage_l2); }
  float get_v_l3() const { return this->apply_stale_strategy_(this->current_.voltage_l3); }

  float get_i_l1() const { return this->apply_stale_strategy_(this->current_.current_l1); }
  float get_i_l2() const { return this->apply_stale_strategy_(this->current_.current_l2); }
  float get_i_l3() const { return this->apply_stale_strategy_(this->current_.current_l3); }

  float get_p_l1_w() const { return this->apply_stale_strategy_(this->current_.power_l1); }
  float get_p_l2_w() const { return this->apply_stale_strategy_(this->current_.power_l2); }
  float get_p_l3_w() const { return this->apply_stale_strategy_(this->current_.power_l3); }
  float get_p_total_w() const { return this->apply_stale_strategy_(this->current_.power_total); }

  float get_s_l1_va() const { return this->apply_stale_strategy_(this->current_.apparent_power_l1); }
  float get_s_l2_va() const { return this->apply_stale_strategy_(this->current_.apparent_power_l2); }
  float get_s_l3_va() const { return this->apply_stale_strategy_(this->current_.apparent_power_l3); }

  float get_q_l1_var() const { return this->apply_stale_strategy_(this->current_.reactive_power_l1); }
  float get_q_l2_var() const { return this->apply_stale_strategy_(this->current_.reactive_power_l2); }
  float get_q_l3_var() const { return this->apply_stale_strategy_(this->current_.reactive_power_l3); }

  float get_pf_l1() const { return this->apply_stale_strategy_(this->current_.power_factor_l1); }
  float get_pf_l2() const { return this->apply_stale_strategy_(this->current_.power_factor_l2); }
  float get_pf_l3() const { return this->apply_stale_strategy_(this->current_.power_factor_l3); }

  float get_frequency_hz() const { return this->apply_stale_strategy_(this->current_.frequency); }
  float get_import_kwh() const { return this->apply_stale_strategy_(this->current_.import_energy); }
  float get_export_kwh() const { return this->apply_stale_strategy_(this->current_.export_energy); }

  bool is_tcp_connected() { return this->client_.connected(); }
  bool has_valid_data() const { return this->data_valid_; }
  bool is_data_fresh() const { return !this->is_stale_(); }

  uint32_t get_age_ms() const;
  float get_age_s() const {
    if (!this->data_valid_)
      return NAN;
    return this->get_age_ms() / 1000.0f;
  }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  static constexpr const char *const TAG = "dsmr_tcp_bridge";

  std::string host_;
  uint16_t port_{82};
  uint32_t reconnect_interval_ms_{3000};
  uint32_t stale_timeout_ms_{10000};
  StaleStrategy stale_strategy_{STALE_STRATEGY_HOLD_LAST};
  uint32_t last_connect_attempt_ms_{0};
  uint32_t last_telegram_ms_{0};
  bool data_valid_{false};
  WiFiClient client_;
  std::string line_;

  // Snapshot atomique exposé au Modbus
  MeterSnapshot current_;
  MeterSnapshot pending_;

  // Valeurs DSMR temporaires d'un télégramme
  float import_t1_kwh_{0.0f};
  float import_t2_kwh_{0.0f};
  float export_t1_kwh_{0.0f};
  float export_t2_kwh_{0.0f};
  float p_import_total_kw_{0.0f};
  float p_export_total_kw_{0.0f};
  float p_import_l1_kw_{0.0f};
  float p_import_l2_kw_{0.0f};
  float p_import_l3_kw_{0.0f};
  float p_export_l1_kw_{0.0f};
  float p_export_l2_kw_{0.0f};
  float p_export_l3_kw_{0.0f};

  void parse_line_(const std::string &line);
  void finalize_telegram_();
  bool is_stale_() const;
  float apply_stale_strategy_(float value) const;

  static bool starts_with_(const std::string &s, const char *prefix);
  static float extract_number_(const std::string &s);
  static float safe_pf_(float p_w, float s_va);
  static float safe_q_(float p_w, float s_va);
};

}  // namespace dsmr_tcp_bridge
}  // namespace esphome
