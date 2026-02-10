# BentoBrain ESPHome Implementation

Simple smart fan controller for 3D printers with Home Assistant integration. Provides percentage-based PWM control and real-time RPM monitoring.

## Overview

This ESPHome version provides a basic smart fan controller that:
- Exposes a percentage-controllable fan entity (0-100%) in Home Assistant
- Monitors actual fan speed via TACH pin (RPM sensor)
- Detects fan failure (binary sensor)
- All automation logic handled in Home Assistant (not on device)

## Hardware Requirements

- **Platform**: ESP32 microcontroller (ESP32-C3 compatible)
- **Fan Control**: PWM output on **GPIO5** (25 kHz)
- **Fan Monitoring**: TACH input on **GPIO6** (for RPM sensing)
- **Fan Type**: 4-pin PWM fan with TACH output
- **Communication**: WiFi (2.4GHz)

### Wiring

```
ESP32 GPIO5  → Fan PWM pin (Yellow wire)
ESP32 GPIO6  → Fan TACH pin (Blue/Yellow wire)
ESP32 GND    → Fan Ground (Black wire)
External PSU → Fan +12V/24V (Red wire)
```

## Prerequisites

**Required before deployment:**
1. ✅ Home Assistant installed and running
2. ✅ ESPHome addon or CLI installed
3. ✅ 4-pin PWM fan with TACH output

**Optional (for automation):**
- Bambu Lab integration for printer status-based automation

## File Structure

```
esphome/
├── bentobrain.yaml          # Main ESPHome configuration
├── secrets.yaml             # WiFi and API credentials (gitignored)
└── secrets.yaml.example     # Template for secrets file
```

## Features

### Manual Fan Control
- Percentage-based speed control (0-100%) from Home Assistant
- PWM output at 25 kHz for smooth operation
- On/Off toggle
- Restores to OFF state on reboot

### RPM Monitoring
- Real-time fan speed monitoring via TACH pin
- Updates every 5 seconds
- Displays actual RPM in Home Assistant
- Supports standard 2-pulse-per-revolution TACH signal

### Fan Failure Detection
- Binary sensor indicates if fan is actually spinning
- Triggers OFF when RPM drops below 100
- 10-second delay to avoid false alarms during spin-up/down
- Useful for creating failure alert automations

### Home Assistant Integration
- Native API integration (auto-discovered)
- Fan entity with percentage slider
- RPM sensor for monitoring
- Fan running binary sensor for status/alerts

## Setup Instructions

### 1. Configure Secrets

Copy and customize the secrets file:

```bash
cd esphome
cp secrets.yaml.example secrets.yaml
```

Edit `secrets.yaml` with your credentials:

```yaml
wifi_ssid: "your_actual_ssid"
wifi_password: "your_actual_password"
api_encryption_key: ""  # Leave blank for auto-generation
ota_password: "choose_secure_password"
```

### 2. Compile and Upload

```bash
# Navigate to ESPHome directory
cd esphome

# Validate configuration
esphome config bentobrain.yaml

# Compile firmware
esphome compile bentobrain.yaml

# Initial upload via USB
esphome run bentobrain.yaml

# Subsequent updates via OTA
esphome run bentobrain.yaml --device bentobrain-v2.local
```

### 3. Add to Home Assistant

The device will be auto-discovered:
1. Navigate to **Settings** → **Devices & Services** → **ESPHome**
2. Look for "BentoBrain v2" in discovered devices
3. Click **Configure** and complete setup
4. **IMPORTANT**: After discovery, add the device to a device entry in Home Assistant for sensors to populate

## Home Assistant Entities

After integration, the following entities are created:

| Entity | Type | Description |
|--------|------|-------------|
| `fan.bentobrain_fan` | Fan | Fan control with percentage slider (0-100%) |
| `sensor.fan_rpm` | Sensor | Current fan speed in RPM |
| `binary_sensor.fan_running` | Binary Sensor | ON when fan is spinning (RPM > 100) |

## Usage

### Manual Control

**From Home Assistant UI:**
1. Navigate to the BentoBrain device
2. Use the fan entity to:
   - Toggle fan on/off
   - Adjust speed with percentage slider (0-100%)
3. Monitor RPM in real-time
4. Check "Fan Running" status

**From Automations/Scripts:**
```yaml
# Turn fan on at 75% speed
service: fan.turn_on
target:
  entity_id: fan.bentobrain_fan
data:
  percentage: 75

# Turn fan off
service: fan.turn_off
target:
  entity_id: fan.bentobrain_fan
```

### Example Automations

#### Automatic Print Cooling

```yaml
automation:
  # Turn fan on when printing starts
  - alias: "BentoBrain - Fan On When Printing"
    trigger:
      - platform: state
        entity_id: sensor.x1c_print_status
        to: "running"
    action:
      - service: fan.turn_on
        target:
          entity_id: fan.bentobrain_fan
        data:
          percentage: 100

  # Turn fan off 5 minutes after print finishes
  - alias: "BentoBrain - Fan Off After Print"
    trigger:
      - platform: state
        entity_id: sensor.x1c_print_status
        from: "running"
        to: "idle"
        for:
          minutes: 5
    action:
      - service: fan.turn_off
        target:
          entity_id: fan.bentobrain_fan
```

#### Fan Failure Alert

```yaml
automation:
  - alias: "BentoBrain - Fan Failure Alert"
    trigger:
      - platform: state
        entity_id: binary_sensor.fan_running
        to: "off"
        for:
          seconds: 30
    condition:
      - condition: state
        entity_id: fan.bentobrain_fan
        state: "on"
    action:
      - service: notify.mobile_app
        data:
          title: "Fan Failure"
          message: "BentoBrain fan has stopped! RPM: {{ states('sensor.fan_rpm') }}"
```

#### Speed-Based Cooling

```yaml
automation:
  # Low speed during prepare phase
  - alias: "BentoBrain - Low Speed Prepare"
    trigger:
      - platform: state
        entity_id: sensor.x1c_print_status
        to: "prepare"
    action:
      - service: fan.turn_on
        target:
          entity_id: fan.bentobrain_fan
        data:
          percentage: 30

  # Full speed during printing
  - alias: "BentoBrain - Full Speed Printing"
    trigger:
      - platform: state
        entity_id: sensor.x1c_print_status
        to: "running"
    action:
      - service: fan.turn_on
        target:
          entity_id: fan.bentobrain_fan
        data:
          percentage: 100
```

## Monitoring

### Dashboard Card

Add a card to your Home Assistant dashboard:

```yaml
type: entities
title: BentoBrain Fan
entities:
  - entity: fan.bentobrain_fan
  - entity: sensor.fan_rpm
    icon: mdi:speedometer
  - entity: binary_sensor.fan_running
    icon: mdi:fan-alert
```

### Logs

View real-time logs via ESPHome:
```bash
esphome logs bentobrain.yaml
```

Or via Home Assistant:
- **Settings** → **Devices & Services** → **ESPHome** → **BentoBrain v2** → **Logs**

## Troubleshooting

### Device Not Discovered in HA
1. Check ESPHome logs for WiFi connection
2. Verify device and HA are on same network
3. Check firewall settings (port 6053 for mDNS)
4. Manually add via IP if auto-discovery fails
5. **IMPORTANT**: Ensure device is added to Devices in Home Assistant (not just ESPHome integration)

### Fan Not Responding
1. Verify fan pin (GPIO5) is correctly wired
2. Check power supply to fan (12V or 24V external PSU)
3. Test fan control from Home Assistant UI
4. Check ESPHome logs for errors

### RPM Shows 0 or Incorrect Values
1. **Check wiring**: GPIO6 should connect to TACH pin (usually yellow or blue wire)
2. **Verify fan type**: Must be 4-pin PWM fan with TACH output
3. **Check TACH pulses**: Most fans use 2 pulses/revolution
   - If RPM is double actual speed: Change `multiply: 0.5` to `multiply: 1.0`
   - If RPM is half actual speed: Change `multiply: 0.5` to `multiply: 0.25`
4. **Try without pullup**: Remove `pullup: true` if TACH signal is inverted

### Fan Running Sensor Always OFF
1. Turn fan on at high speed (>50%)
2. Wait 10 seconds for delayed_off filter
3. Check if RPM sensor is working (should show >100 RPM)
4. Adjust threshold in YAML if needed (change `> 100` to different value)

## Customization

### Changing Fan PWM Pin

Edit [bentobrain.yaml](bentobrain.yaml):

```yaml
output:
  - platform: ledc
    pin: GPIO5  # Change to your pin
    id: fan_pwm_output
```

### Changing TACH Pin

Edit [bentobrain.yaml](bentobrain.yaml):

```yaml
sensor:
  - platform: pulse_counter
    pin:
      number: GPIO6  # Change to your pin
```

### Adjusting TACH Pulses Per Revolution

If your fan uses 1 pulse/revolution instead of 2:

```yaml
filters:
  - multiply: 1.0  # Change from 0.5 to 1.0
```

### Changing Update Intervals

```yaml
sensor:
  - platform: pulse_counter
    update_interval: 10s  # Change from 5s to 10s
```

### Adjusting Fan Failure Threshold

```yaml
binary_sensor:
  - platform: template
    lambda: |-
      return id(fan_rpm).state > 200;  # Change from 100 to 200
```

## Architecture Comparison

| Feature | Arduino Version | ESPHome Version |
|---------|-----------------|-----------------|
| **Data Source** | Direct MQTT from printer | Manual/HA Automations |
| **Fan Control** | Manual + Automatic | Manual (via HA) |
| **RPM Monitoring** | No | Yes (TACH pin) |
| **Manual Control** | Web UI + REST API | Home Assistant UI |
| **Automation Logic** | Built-in C++ code | Home Assistant YAML |
| **Configuration UI** | Custom web pages | Home Assistant |
| **Settings Storage** | NVS (persistent) | HA automations |
| **Standalone Operation** | Yes | No (requires HA) |
| **Complexity** | Higher (more code) | Lower (simple YAML) |

## Benefits

1. ✅ **Simple Configuration** - Just WiFi credentials needed
2. ✅ **Flexible Automation** - All logic in HA (easy to modify)
3. ✅ **RPM Monitoring** - Real-time fan speed feedback
4. ✅ **Failure Detection** - Alert when fan stops
5. ✅ **Percentage Control** - Smooth speed adjustment (0-100%)
6. ✅ **Easy Updates** - OTA built into ESPHome
7. ✅ **Native HA Integration** - Entities, automation, history

## Trade-offs

1. ⚠️ **Requires Home Assistant** - Cannot operate standalone
2. ⚠️ **No Built-in Automation** - Must create HA automations
3. ⚠️ **Manual Setup Required** - Device must be added to HA Devices
4. ⚠️ **4-Pin Fan Required** - TACH monitoring needs 4-pin PWM fan

## Design Philosophy

The ESPHome version is designed as a **simple building block**:

- **Device provides**: PWM control, RPM monitoring, status reporting
- **Home Assistant provides**: Automation logic, scheduling, conditions
- **Result**: Maximum flexibility with minimal device complexity

This makes the ESPHome version ideal for users who:
- ✅ Already use Home Assistant
- ✅ Want flexible, customizable automation
- ✅ Need RPM monitoring and failure detection
- ✅ Prefer declarative YAML over embedded C++
- ✅ Want easy configuration changes without reflashing

## Additional Resources

- [ESPHome Documentation](https://esphome.io)
- [ESPHome Fan Component](https://esphome.io/components/fan/speed.html)
- [ESPHome Pulse Counter](https://esphome.io/components/sensor/pulse_counter.html)
- [Home Assistant Automations](https://www.home-assistant.io/docs/automation/)
- [Home Assistant Bambu Lab Integration](https://www.home-assistant.io/integrations/bambu_lab/)

## Alternative: Arduino Version

A more feature-rich Arduino implementation is available in the `../arduino/` directory that:
- Supports standalone operation (no HA required)
- Includes built-in automatic print detection via MQTT
- Provides manual control via web interface
- Offers REST API for external control
- Has temperature-based triggering

See [../arduino/README.md](../arduino/README.md) for details.

## Notes for AI Assistants

When working with the ESPHome version:
- **Intentionally minimal** - Device is just a smart fan controller
- **Automation happens in HA** - Don't add complex logic to device
- **YAML changes require reflashing** - Not live-reloadable
- **RPM monitoring is optional** - Works without TACH pin connected
- **Test with actual hardware** - PWM and TACH signals need physical verification
