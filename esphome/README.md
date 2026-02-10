# BentoBrain ESPHome Implementation

Ultra-simple, Home Assistant-centric smart fan controller for 3D printers. Automatically controls fan based on print status from Home Assistant's Bambu Lab integration.

## Overview

This ESPHome version provides "set it and forget it" automation by:
- Leveraging Home Assistant's existing Bambu Lab integration
- Eliminating direct MQTT subscription (no TLS, credentials, or JSON parsing)
- Providing fully automatic operation (no manual control)
- Removing local storage (no persistence needed)

## Hardware Requirements

- **Platform**: ESP32 microcontroller
- **Fan Control**: PWM-based (25 kHz)
- **Default Fan Pin**: GPIO 6
- **Communication**: WiFi (2.4GHz)

## Prerequisites

**Required before deployment:**
1. ✅ Home Assistant installed and running
2. ✅ Bambu Lab integration configured in HA
3. ✅ Sensor `sensor.x1c_print_status` available and working
4. ✅ ESPHome addon or CLI installed

## File Structure

```
esphome/
├── bentobrain.yaml          # Main ESPHome configuration
├── secrets.yaml             # WiFi and API credentials (gitignored)
└── secrets.yaml.example     # Template for secrets file
```

## Features

### Automatic Print-Based Control
- Monitors print status via Home Assistant's `sensor.x1c_print_status`
- Fan turns **ON** at 100% when printer is "printing" or "paused"
- Fan remains **ON** for configurable delay after printing stops
- Fan turns **OFF** after delay expires
- No temperature threshold - based purely on print state

### Home Assistant Integration
- Native API integration (auto-discovered)
- Fan entity exposed for monitoring
- Configurable delay via number entity
- Print status text sensor imported from HA

### Configuration
- **Fan Off Delay**: Adjustable from Home Assistant (default: 300 seconds)
- **No Manual Control**: Fully automatic operation only
- **No Persistence**: Settings reset to defaults on reboot

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

### 2. Verify Home Assistant Sensor

Ensure the Bambu Lab integration sensor exists:
1. Go to **Settings** → **Devices & Services** → **Bambu Lab**
2. Verify `sensor.x1c_print_status` is available
3. Check that it updates correctly when printing starts/stops

### 3. Compile and Upload

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
esphome run bentobrain.yaml --device bentobrain.local
```

### 4. Add to Home Assistant

The device will be auto-discovered:
1. Navigate to **Settings** → **Devices & Services**
2. Look for "BentoBrain" in discovered devices
3. Click **Configure** and enter the API encryption key (if not auto-generated)

## Home Assistant Entities

After integration, the following entities are created:

| Entity | Type | Description |
|--------|------|-------------|
| `fan.bentobrain_fan` | Fan | Fan status (read-only, automatic control) |
| `text_sensor.printer_status` | Text Sensor | Print status imported from HA |
| `number.fan_off_delay` | Number | Delay before fan turns off (seconds) |

## Configuration

### Adjusting Fan Off Delay

1. In Home Assistant, navigate to the BentoBrain device
2. Find **Fan Off Delay** number entity
3. Adjust value (0-3600 seconds, 30-second increments)
4. Changes take effect immediately
5. **Note**: Value resets to 300s on device reboot

### Monitoring Operation

- Check fan entity state to see if fan is running
- View printer status to see current print state
- ESPHome logs show detailed operation (accessible via web or HA)

## How It Works

### Automatic Control Logic

The device polls every 5 seconds and executes this logic:

```
IF printer status = "printing" OR "paused":
    Turn fan ON at 100%
    Record current time
ELSE:
    Calculate time since printing stopped
    IF time < fan_off_delay:
        Keep fan ON (cooling period)
    ELSE:
        Turn fan OFF
```

### Example Timeline

```
00:00 - Print starts → Fan turns ON
01:30 - Print completes → Fan stays ON (cooling)
01:35 - 5 minutes elapsed → Fan turns OFF
```

## Architecture Comparison

| Feature | Arduino Version | ESPHome Version |
|---------|-----------------|-----------------|
| **Data Source** | Direct MQTT from printer | HA Bambu Lab integration |
| **Temperature Monitoring** | MQTT nozzle_temper | Print status only |
| **MQTT Client** | PubSubClient + TLS | None |
| **Manual Control** | Web UI + REST API | None (automatic only) |
| **Configuration UI** | Custom web pages | Home Assistant UI |
| **Settings Storage** | NVS (persistent) | RAM (non-persistent) |
| **Credentials** | WiFi + MQTT (6 values) | WiFi only (2 values) |
| **Standalone Operation** | Yes | No (requires HA) |
| **Complexity** | Feature-rich, complex | Ultra-simple, single-purpose |

## Benefits

1. ✅ **Simpler Setup** - Only WiFi credentials needed (no MQTT config)
2. ✅ **No TLS Complexity** - HA handles printer connection
3. ✅ **Declarative Config** - YAML instead of C++ code
4. ✅ **Native HA Integration** - Entities, automation, history built-in
5. ✅ **Easier Updates** - OTA built into ESPHome
6. ✅ **Better Logging** - Structured logs via API
7. ✅ **No Manual Control Needed** - Pure automation

## Trade-offs

1. ⚠️ **Requires Home Assistant** - Cannot operate standalone
2. ⚠️ **Requires Bambu Lab Integration** - Must have configured HA integration
3. ⚠️ **No Manual Override** - Cannot manually control fan speed
4. ⚠️ **No Persistence** - Delay setting resets to 300s on reboot
5. ⚠️ **Print Status Only** - No direct temperature monitoring
6. ⚠️ **Single Use Case** - Optimized for automatic print cooling only

## Design Philosophy

The ESPHome version intentionally trades **flexibility** for **simplicity**:

- **Arduino**: Feature-rich standalone device with manual control, temperature monitoring, and persistence
- **ESPHome**: Simple HA-dependent automation that "just works"

This makes the ESPHome version ideal for users who:
- ✅ Already use Home Assistant
- ✅ Have Bambu Lab integration configured
- ✅ Want minimal configuration and maintenance
- ✅ Don't need manual control or standalone operation
- ✅ Prefer "set it and forget it" automation

## Troubleshooting

### Device Not Discovered in HA
1. Check ESPHome logs for WiFi connection
2. Verify device and HA are on same network
3. Check firewall settings (port 6053 for mDNS)
4. Manually add via IP if auto-discovery fails

### Fan Not Turning On
1. Verify `sensor.x1c_print_status` exists in HA
2. Check ESPHome logs for print status updates
3. Ensure printer is actually "printing" or "paused"
4. Verify fan pin (GPIO6) is correct for your wiring

### Fan Won't Turn Off
1. Check if delay is set too high
2. Verify print status is not "printing" or "paused"
3. Review ESPHome logs for timing information
4. Restart device if timing seems stuck

### Settings Don't Persist
This is **expected behavior** - settings are not persisted to flash. Fan off delay resets to 300 seconds on reboot. If persistence is needed, consider using the Arduino version instead.

### Print Status Not Updating
1. Verify Bambu Lab integration is working in HA
2. Check that sensor entity name matches exactly
3. Review HA logs for integration issues
4. Restart Bambu Lab integration if needed

## Customization

### Changing Fan Pin

Edit [bentobrain.yaml](bentobrain.yaml) and change the GPIO pin:

```yaml
output:
  - platform: ledc
    pin: GPIO5  # Change to your pin
    id: fan_pwm_output
```

### Changing Polling Interval

Edit the interval section:

```yaml
interval:
  - interval: 10s  # Change from 5s to 10s
    then:
      - lambda: |-
```

### Adding Additional Print States

Edit the lambda to include more states:

```yaml
if (status == "printing" || status == "paused" || status == "prepare") {
```

## Advanced: Home Assistant Automations

You can create HA automations to extend functionality:

### Example: Notification When Fan Turns On

```yaml
automation:
  - alias: "BentoBrain Fan Started"
    trigger:
      platform: state
      entity_id: fan.bentobrain_fan
      to: 'on'
    action:
      service: notify.mobile_app
      data:
        message: "BentoBrain fan activated - print detected"
```

### Example: Override Delay Based on Filament

```yaml
automation:
  - alias: "BentoBrain ABS Extended Cooling"
    trigger:
      platform: state
      entity_id: sensor.x1c_print_status
      to: 'printing'
    condition:
      condition: template
      value_template: "{{ 'ABS' in state_attr('sensor.x1c_print_name', 'filament') }}"
    action:
      service: number.set_value
      target:
        entity_id: number.fan_off_delay
      data:
        value: 600  # 10 minutes for ABS
```

## Future Enhancements

Possible additions while maintaining simplicity:
- Status LED for visual feedback
- Additional environmental sensors (temperature, humidity)
- Multiple fan control (exhaust, intake, filter)
- Home Assistant automations for advanced control

## Additional Resources

- [ESPHome Documentation](https://esphome.io)
- [ESPHome Fan Component](https://esphome.io/components/fan/speed.html)
- [Home Assistant Bambu Lab Integration](https://www.home-assistant.io/integrations/bambu_lab/)
- [ESPHome Home Assistant Sensor](https://esphome.io/components/text_sensor/homeassistant.html)

## Alternative: Arduino Version

A more feature-rich Arduino implementation is available in the `../arduino/` directory that:
- Supports standalone operation (no HA required)
- Includes manual control via web interface
- Provides direct MQTT temperature monitoring
- Offers REST API for external control

See [../arduino/README.md](../arduino/README.md) for details.

## Notes for AI Assistants

When working with the ESPHome version:
- **Architecture is intentionally simple** - Don't add complexity
- **No manual control by design** - This is automatic-only
- **No persistence by design** - Device depends on HA connectivity
- **Print status drives everything** - No temperature monitoring
- **YAML changes require recompilation** - Not live-reloadable
- **Test with actual HA integration** - Mock data won't work the same
