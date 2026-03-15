#include "dsmr_tcp_bridge.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace dsmr_tcp_bridge {

void DsmrTcpBridge::setup() {}

void DsmrTcpBridge::dump_config() {
  ESP_LOGCONFIG(TAG, "DSMR TCP Bridge:");
  ESP_LOGCONFIG(TAG, "  Host: %s", this->host_.c_str());
  ESP_LOGCONFIG(TAG, "  Port: %u", this->port_);
  ESP_LOGCONFIG(TAG, "  Reconnect interval: %ums", this->reconnect_interval_ms_);
  ESP_LOGCONFIG(TAG, "  Stale timeout: %ums", this->stale_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Stale strategy: %s",
                this->stale_strategy_ == STALE_STRATEGY_HOLD_LAST ? "hold_last" :
                this->stale_strategy_ == STALE_STRATEGY_NAN ? "nan" : "zero");
}

bool DsmrTcpBridge::starts_with_(const std::string &s, const char *prefix) { return s.rfind(prefix, 0) == 0; }

float DsmrTcpBridge::extract_number_(const std::string &s) {
  auto p1 = s.find('(');
  if (p1 == std::string::npos)
    return NAN;

  auto p2 = s.find('*', p1 + 1);
  if (p2 == std::string::npos)
    p2 = s.find(')', p1 + 1);
  if (p2 == std::string::npos)
    return NAN;

  return atof(s.substr(p1 + 1, p2 - p1 - 1).c_str());
}

float DsmrTcpBridge::safe_pf_(float p_w, float s_va) {
  if (s_va < 1.0f)
    return 0.0f;
  float pf = p_w / s_va;
  if (pf > 1.0f)
    pf = 1.0f;
  if (pf < -1.0f)
    pf = -1.0f;
  return pf;
}

float DsmrTcpBridge::safe_q_(float p_w, float s_va) {
  if (s_va <= 0.0f)
    return 0.0f;
  float x = (s_va * s_va) - (p_w * p_w);
  if (x < 0.0f)
    x = 0.0f;
  return sqrtf(x);
}

uint32_t DsmrTcpBridge::get_age_ms() const {
  if (!this->data_valid_)
    return UINT32_MAX;
  return millis() - this->last_telegram_ms_;
}

bool DsmrTcpBridge::is_stale_() const {
  if (!this->data_valid_)
    return true;
  return (millis() - this->last_telegram_ms_) > this->stale_timeout_ms_;
}

float DsmrTcpBridge::apply_stale_strategy_(float value) const {
  if (!this->is_stale_())
    return value;

  switch (this->stale_strategy_) {
    case STALE_STRATEGY_NAN:
      return NAN;
    case STALE_STRATEGY_ZERO:
      return 0.0f;
    case STALE_STRATEGY_HOLD_LAST:
    default:
      return value;
  }
}

void DsmrTcpBridge::finalize_telegram_() {
  this->p_l1_w_ = (this->p_import_l1_kw_ - this->p_export_l1_kw_) * 1000.0f;
  this->p_l2_w_ = (this->p_import_l2_kw_ - this->p_export_l2_kw_) * 1000.0f;
  this->p_l3_w_ = (this->p_import_l3_kw_ - this->p_export_l3_kw_) * 1000.0f;
  this->p_total_w_ = (this->p_import_total_kw_ - this->p_export_total_kw_) * 1000.0f;

  this->s_l1_va_ = this->v_l1_ * this->i_l1_;
  this->s_l2_va_ = this->v_l2_ * this->i_l2_;
  this->s_l3_va_ = this->v_l3_ * this->i_l3_;

  this->q_l1_var_ = safe_q_(fabsf(this->p_l1_w_), this->s_l1_va_);
  this->q_l2_var_ = safe_q_(fabsf(this->p_l2_w_), this->s_l2_va_);
  this->q_l3_var_ = safe_q_(fabsf(this->p_l3_w_), this->s_l3_va_);

  this->pf_l1_ = safe_pf_(this->p_l1_w_, this->s_l1_va_);
  this->pf_l2_ = safe_pf_(this->p_l2_w_, this->s_l2_va_);
  this->pf_l3_ = safe_pf_(this->p_l3_w_, this->s_l3_va_);

  this->frequency_hz_ = 50.0f;
  this->import_kwh_ = this->import_t1_kwh_ + this->import_t2_kwh_;
  this->export_kwh_ = this->export_t1_kwh_ + this->export_t2_kwh_;
  this->last_telegram_ms_ = millis();
  this->data_valid_ = true;

  ESP_LOGD(TAG, "Updated from telegram: Ptot=%.1fW V=[%.1f %.1f %.1f] I=[%.2f %.2f %.2f]",
           this->p_total_w_, this->v_l1_, this->v_l2_, this->v_l3_, this->i_l1_, this->i_l2_, this->i_l3_);
}

void DsmrTcpBridge::parse_line_(const std::string &line) {
  if (starts_with_(line, "1-0:1.8.1(")) {
    this->import_t1_kwh_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:1.8.2(")) {
    this->import_t2_kwh_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:2.8.1(")) {
    this->export_t1_kwh_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:2.8.2(")) {
    this->export_t2_kwh_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:1.7.0(")) {
    this->p_import_total_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:2.7.0(")) {
    this->p_export_total_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:21.7.0(")) {
    this->p_import_l1_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:41.7.0(")) {
    this->p_import_l2_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:61.7.0(")) {
    this->p_import_l3_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:22.7.0(")) {
    this->p_export_l1_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:42.7.0(")) {
    this->p_export_l2_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:62.7.0(")) {
    this->p_export_l3_kw_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:32.7.0(")) {
    this->v_l1_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:52.7.0(")) {
    this->v_l2_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:72.7.0(")) {
    this->v_l3_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:31.7.0(")) {
    this->i_l1_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:51.7.0(")) {
    this->i_l2_ = extract_number_(line);
  } else if (starts_with_(line, "1-0:71.7.0(")) {
    this->i_l3_ = extract_number_(line);
  } else if (!line.empty() && line[0] == '!') {
    this->finalize_telegram_();
  }
}

void DsmrTcpBridge::loop() {
  const uint32_t now = millis();

  if (!this->client_.connected()) {
    if (now - this->last_connect_attempt_ms_ > this->reconnect_interval_ms_) {
      this->last_connect_attempt_ms_ = now;
      this->client_.stop();
      ESP_LOGW(TAG, "TCP disconnected, reconnecting to %s:%u", this->host_.c_str(), this->port_);
      this->client_.connect(this->host_.c_str(), this->port_);
    }
    return;
  }

  while (this->client_.available()) {
    char c = static_cast<char>(this->client_.read());
    if (c == '\r')
      continue;

    if (c == '\n') {
      if (!this->line_.empty()) {
        this->parse_line_(this->line_);
        this->line_.clear();
      }
    } else {
      if (this->line_.size() < 512)
        this->line_.push_back(c);
    }
  }
}

}  // namespace dsmr_tcp_bridge
}  // namespace esphome
