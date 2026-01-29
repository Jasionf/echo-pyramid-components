#pragma once

#ifdef USE_ESP32

#include "espnow_component.h"

#include "esphome/components/switch/switch.h"
#include "espnow_packet.h"

#include <string>
#include <vector>

namespace esphome::espnow {

class ESPNowSwitchC6 : public switch_::Switch, public Component, public Parented<ESPNowComponent> {
 public:
  void setup() override {}
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  void set_mac(const std::string &mac) { this->normalized_mac_ = mac; }
  void set_channel(uint8_t ch) { this->channel_ = ch; }
  void set_need_response(bool v) { this->need_response_ = v; }

 protected:
  void write_state(bool state) override;

 private:
  bool ensure_broadcast_peer_added();
  std::string normalized_mac_;
  uint8_t channel_{1};
  bool need_response_{true};
  bool broadcast_peer_added_{false};
};

}  // namespace esphome::espnow

#endif  // USE_ESP32
