#ifdef USE_ESP32

#include "switch_c6_switch.h"

#include <cstdio>

namespace esphome::espnow {

bool ESPNowSwitchC6::ensure_broadcast_peer_added() {
  if (this->broadcast_peer_added_) return true;
  if (this->parent_ == nullptr) return false;
  auto err = this->parent_->add_peer(ESPNOW_BROADCAST_ADDR);
  if (err == ESP_OK) {
    this->broadcast_peer_added_ = true;
    return true;
  }
  return false;
}

void ESPNowSwitchC6::dump_config() {
  ESP_LOGCONFIG("espnow_switch_c6", "SwitchC6 MAC: %s, channel: %u, need_response: %s", this->normalized_mac_.c_str(),
                this->channel_, this->need_response_ ? "true" : "false");
}

void ESPNowSwitchC6::write_state(bool state) {
  if (this->parent_ == nullptr || this->normalized_mac_.empty()) {
    this->publish_state(state);  // optimistic
    return;
  }

  // Apply channel
  this->parent_->set_wifi_channel(this->channel_);
  this->parent_->apply_wifi_channel();

  // Build payload like Arduino: "MAC=1|0;ch=n[;]"
  char buf[64];
  int n = std::snprintf(buf, sizeof(buf), "%s=%s;ch=%u%s", this->normalized_mac_.c_str(), state ? "1" : "0",
                        this->channel_, this->need_response_ ? ";" : "");
  if (n <= 0) {
    this->publish_state(state);
    return;
  }
  std::vector<uint8_t> payload(buf, buf + n);

  // Ensure broadcast peer exists and send
  (void) this->ensure_broadcast_peer_added();
  esp_err_t res = this->parent_->send(ESPNOW_BROADCAST_ADDR, payload);

  // Optimistic publish (matches user's preference for immediate control)
  this->publish_state(state);
  (void) res;
}

}  // namespace esphome::espnow

#endif  // USE_ESP32
