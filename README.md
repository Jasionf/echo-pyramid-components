# Echo Pyramid ESPHome Components 🎛️💡🎤

A friendly bundle of custom ESPHome bits that powers the Echo Pyramid on ESP32‑S3: RGB lighting via an onboard STM32 controller, touch inputs with swipe gestures, audio amp + DAC/ADC, and a clock generator. Sprinkle in polished example YAMLs and you're off to the races.

---

## What’s Inside

| Area | Component | Path | Bus/Address | Quick Note |
|---|---|---|---|---|
| RGB Strips | pyramidrgb (parent + outputs) | `components/pyramidrgb` | I2C @ 0x1A | Drives 2 strips × 2 groups × 7 LEDs (per-LED writes) |
| Touch | pyramidtouch | `components/pyramidtouch` | I2C @ 0x1A | Touch1–4 + optional swipe events (left/right up/down) |
| LED Driver | lp5562 | `components/lp5562` | I2C @ 0x30 | TI RGBW driver with PWM/current + engine map |
| Audio Amp | aw87559 | `components/aw87559` | I2C @ 0x5B | Minimal init, logs setup status |
| Clock Gen | si5351 | `components/si5351` | I2C @ 0x60 | Sets up outputs with preset params |

Tips:
- Two folders exist for PyramidRGB (`PyramidRGB/` and `pyramidrgb/`); use the lowercase one to avoid confusion.
- Example dashboards live in `examples/`.

---

## Quick Start

1. Drop this folder into your ESPHome config directory.
2. Point `external_components` to `source: github://Jasionf/echo-pyramid-components@main` (or `local`).
3. Add I2C + logger:

```yaml
external_components:
  - source: github://Jasionf/echo-pyramid-components@main
    components: [aw87559, si5351, lp5562, pyramidrgb, pyramidtouch]
    refresh: 0s

i2c:
  - id: bsp_bus
    sda: GPIO45
    scl: GPIO0
    scan: true

logger:
  level: DEBUG
```

4. See the full voice assistant demo in [examples/HomeAssistantVoice.yaml](examples/HomeAssistantVoice.yaml).

---

## PyramidRGB (Lights)

Parent component (`PyramidRGBComponent`) controls an STM32 at 0x1A. Child outputs (`PyramidRGBOutput`) map per-channel/per-color FloatOutputs to the parent. Hardware layout:
- Strips: 2
- Groups per strip: 2
- LEDs per group: 7
- Brightness regs: `0x10` (strip 1), `0x11` (strip 2)
- Per-LED color bytes: `[B, G, R, 0x00]`
- Channel base addresses:
  - ch0 → `0x20` (Strip 1, Group 1)
  - ch1 → `0x3C` (Strip 1, Group 2)
  - ch2 → `0x7C` (Strip 2, Group 2)
  - ch3 → `0x60` (Strip 2, Group 1)
- LED order for ch0/ch1 is reversed to match wiring.

Add parent(s):

```yaml
pyramidrgb:
  - id: pyramid_rgb1
    i2c_id: bsp_bus
    strip: 1
    # Optional tuning
    brightness: 100     # 0–100
    initial_white: 0    # 0–255 (fill on boot)
    # Optional dimming/scaling (if enabled in your build)
    # logarithmic_dimming: true
    # gamma: 2.2
```

Map outputs → lights:

```yaml
output:
  - platform: pyramidrgb
    id: rgb1_ch0_red
    pyramidrgb_id: pyramid_rgb1
    channel: 0
    color: red
  - platform: pyramidrgb
    id: rgb1_ch0_green
    pyramidrgb_id: pyramid_rgb1
    channel: 0
    color: green
  - platform: pyramidrgb
    id: rgb1_ch0_blue
    pyramidrgb_id: pyramid_rgb1
    channel: 0
    color: blue

light:
  - platform: rgb
    name: "Strip1 Group1"
    red: rgb1_ch0_red
    green: rgb1_ch0_green
    blue: rgb1_ch0_blue
```

Brightness sliders (nice UX):

```yaml
number:
  - platform: template
    name: "RGB Master Brightness"
    id: rgb_master_brightness
    min_value: 0
    max_value: 100
    step: 1
    set_action:
      - lambda: |-
          uint8_t b = (uint8_t) x;
          id(pyramid_rgb1).set_strip_brightness(1, b);
          id(pyramid_rgb2).set_strip_brightness(2, b);
```

Notes:
- Setting brightness alone won’t light LEDs — set color or `initial_white`.
- Per-LED writes favor compatibility; not all controllers like long auto-increment bursts.
- A 7-bit brightness mapping is used to avoid mid-range saturation.

---

## PyramidTouch (Gestures)

Reads touch1–4 from the same STM32 and can publish swipe events.

Minimal config:

```yaml
sensor:
  - platform: pyramidtouch
    i2c_id: bsp_bus
    address: 0x1A
    update_interval: 50ms
    publish_swipe_event: true
    swipe_timeout_ms: 500
    touch1:
      name: "Touch 1"
    touch2:
      name: "Touch 2"
    touch3:
      name: "Touch 3"
    touch4:
      name: "Touch 4"
    swipe_event:
      name: "Touch Swipe Event"
      entity_category: diagnostic
```

Swipe codes: `1=Left Up`, `2=Left Down`, `3=Right Up`, `4=Right Down`.

Tie swipe gestures to local volume:

```yaml
globals:
  - id: current_volume
    type: float
    restore_value: true
    initial_value: '0.3'

media_player:
  - platform: speaker
    id: echo_pyramid_player
    volume_min: 0.0
    volume_max: 1.0
    volume_initial: 0.10

sensor:
  - platform: pyramidtouch
    # ...
    swipe_event:
      on_value:
        then:
          - lambda: |-
              const float step = 0.05f;
              float v = id(current_volume);
              const int ev = (int) x;
              if (ev == 1 || ev == 3) v = std::min(1.0f, v + step);
              else if (ev == 2 || ev == 4) v = std::max(0.0f, v - step);
              auto call = id(echo_pyramid_player).make_call();
              call.set_volume(v);
              call.perform();
              id(current_volume) = v;
```

---

## LP5562 (RGBW LED Driver)

Full-featured TI LP5562 control: PWM brightness, current per channel, and engine/source mapping.

Example:

```yaml
lp5562:
  id: lp5562_led
  i2c_id: bsp_bus
  use_internal_clk: true
  high_pwm_freq: true
  white_current: 17.5

output:
  - platform: lp5562
    id: lp5562_white_channel
    lp5562_id: lp5562_led
    channel: white

light:
  - platform: monochromatic
    name: "LCD Backlight"
    output: lp5562_white_channel
```

---

## AW87559 (Audio Amplifier)

Lightweight initializer with helpful logs.

```yaml
aw87559:
  id: audio_amp
  i2c_id: bsp_bus
  address: 0x5B
```

---

## Si5351 (Clock Generator)

Configures a Si5351 at 0x60 and enables outputs.

```yaml
si5351:
  id: clock_gen
  i2c_id: bsp_bus
  address: 0x60
```

---

## Examples

- Full voice assistant + UI: [examples/HomeAssistantVoice.yaml](examples/HomeAssistantVoice.yaml)
- RGB + touch patterns: roll your own with the snippets above; the demo file combines media, touch, and lights.

Pro tip: Add a volume slider for UX delight:

```yaml
number:
  - platform: template
    name: "Master Volume"
    id: master_volume
    min_value: 0.0
    max_value: 1.0
    step: 0.01
    set_action:
      - lambda: |-
          float v = x;
          auto call = id(echo_pyramid_player).make_call();
          call.set_volume(v);
          call.perform();
          id(current_volume) = v;
```

---

## Troubleshooting

- LEDs not lighting:
  - Ensure `brightness > 0` and you’ve set a color or `initial_white`.
  - Check I2C address 0x1A is found (`i2c.scan: true`).
- "Green/Red only" behavior:
  - Controller expects `[B, G, R, 0x00]`; use the provided outputs to match.
- Volume slider too loud at low values:
  - Verify `media_player.volume_min/max` are 0.0/1.0 and not clamping.
- Brightness saturates at ~50:
  - The 7‑bit mapping resolves mid-range saturation; keep the slider at 0–100.

---

## Repo Layout

```
components/
  aw87559/
  lp5562/
  pyramidrgb/
  pyramidtouch/
  si5351/
examples/
  HomeAssistantVoice.yaml
```

---

## Dev Notes

- Use `logger.level: DEBUG` to inspect I2C traffic.
- Keep I2C at 400 kHz unless your bus requires lower speed.
- Want per-strip sliders, fancy effects, or scenes? Just add more `number.template` entities and reference them in automations.

Have fun, build cool stuff, and ping if you want extra helpers like per‑group brightness or color presets! ✨