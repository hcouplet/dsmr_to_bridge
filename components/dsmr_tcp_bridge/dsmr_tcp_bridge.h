#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <WiFiClient.h>
#include <cmath>
#include <string>

namespace esphome {
namespace dsmr_tcp_bridge {

class DsmrTcpBridge : public Component {
 public:
  void set_host(const std::string &host) { this->host_ = host; }
  void set_port(uint16_t port) { this->port_ = port; }
  void set_reconnect_interval(uint32_t reconnect_interval_ms) { this->reconnect_interval_ms_ = reconnect_interval_ms; }

  float get_v_l1() const { return this->v_l1_; }
  float get_v_l2() const { return this->v_l2_; }
  float get_v_l3() const { return this->v_l3_; }

  float get_i_l1() const { return this->i_l1_; }
  float get_i_l2() const { return this->i_l2_; }
  float get_i_l3() const { return this->i_l3_; }

  float get_p_l1_w() const { return this->p_l1_w_; }
  float get_p_l2_w() const { return this->p_l2_w_; }
  float get_p_l3_w() const { return this->p_l3_w_; }
  float get_p_total_w() const { return this->p_total_w_; }

  float get_s_l1_va() const { return this->s_l1_va_; }
  float get_s_l2_va() const { return this->s_l2_va_; }
  float get_s_l3_va() const { return this->s_l3_va_; }

  float get_q_l1_var() const { return this->q_l1_var_; }
  float get_q_l2_var() const { return this->q_l2_var_; }
  float get_q_l3_var() const { return this->q_l3_var_; }

  float get_pf_l1() const { return this->pf_l1_; }
  float get_pf_l2() const { return this->pf_l2_; }
  float get_pf_l3() const { return this->pf_l3_; }

  float get_frequency_hz() const { return this->frequency_hz_; }
  float get_import_kwh() const { return this->import_kwh_; }
  float get_export_kwh() const { return this->export_kwh_; }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  static constexpr const char *const TAG = "dsmr_tcp_bridge";

  std::string host_;
  uint16_t port_{82};
  uint32_t reconnect_interval_ms_{3000};
  uint32_t last_connect_attempt_ms_{0};
  WiFiClient client_;
  std::string line_;

  // Exposed SDM630-like values
  float v_l1_{0.0f};
  float v_l2_{0.0f};
  float v_l3_{0.0f};
  float i_l1_{0.0f};
  float i_l2_{0.0f};
  float i_l3_{0.0f};
  float p_l1_w_{0.0f};
  float p_l2_w_{0.0f};
  float p_l3_w_{0.0f};
  float p_total_w_{0.0f};
  float s_l1_va_{0.0f};
  float s_l2_va_{0.0f};
  float s_l3_va_{0.0f};
  float q_l1_var_{0.0f};
  float q_l2_var_{0.0f};
  float q_l3_var_{0.0f};
  float pf_l1_{0.0f};
  float pf_l2_{0.0f};
  float pf_l3_{0.0f};
  float frequency_hz_{50.0f};
  float import_kwh_{0.0f};
  float export_kwh_{0.0f};

  // Per-telegram DSMR values
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

  static bool starts_with_(const std::string &s, const char *prefix);
  static float extract_number_(const std::string &s);
  static float safe_pf_(float p_w, float s_va);
  static float safe_q_(float p_w, float s_va);
};

}  // namespace dsmr_tcp_bridge
}  // namespace esphome
