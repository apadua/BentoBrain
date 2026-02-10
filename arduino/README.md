# BentoBrain Arduino Implementation

ESP32-based smart fan controller for 3D printers with MQTT integration, manual control, and web interface.

## Hardware Requirements

- **Platform**: ESP32 microcontroller
- **Fan Control**: PWM-based (25 kHz, 8-bit resolution)
- **Default Fan Pin**: GPIO 6
- **Communication**: WiFi (2.4GHz), MQTT over TLS

## Features

### 1. Automatic Temperature-Based Fan Control
- Monitors nozzle temperature from MQTT messages
- Turns fan ON when temperature exceeds threshold (default: 180°C)
- Turns fan OFF after a configurable delay (default: 5 minutes) when temperature drops below threshold
- PWM control provides precise fan speed adjustment

### 2. Manual Fan Control via Web Interface
- Web server on port 80
- Accessible at `http://<esp32-ip>/`
- Features:
  - Slider for manual fan speed control (0-100%)
  - "Automatic" mode button to return to temperature-based control
  - Visual feedback with BentoBrain branding (orange: rgb(255,90,44))

### 3. Configuration Interface
- Web-based config page at `http://<esp32-ip>/config`
- Configurable settings:
  - WiFi credentials (SSID, password)
  - MQTT broker details (IP, port, username, password, topic)
  - Fan settings (pin, temperature threshold, off-delay)
  - API authentication token
- Settings stored in ESP32 NVS (Non-Volatile Storage)
- "Save" or "Save and Restart" options

### 4. REST API
- Endpoint: `/api/setFanSpeed`
- Parameters: `token` (auth), `speed` (0-100)
- Can be disabled by setting authToken to "-1"
- Used for external automation/integration

### 5. OTA Updates
- Over-The-Air firmware updates supported
- ArduinoOTA library integration
- Progress and error reporting via Serial

## File Structure

```
arduino/
├── bentobrain.ino    # Main application code
├── config.h          # Configuration variables (gitignored)
└── mqtt.h            # MQTT utilities header
```

## Setup

### 1. Install Dependencies

Install the following libraries via Arduino Library Manager:
- WiFi (ESP32 core)
- WiFiClientSecure (ESP32 core)
- PubSubClient
- ArduinoJson
- WebServer (ESP32 core)
- ESPmDNS (ESP32 core)
- ArduinoOTA (ESP32 core)
- Preferences (ESP32 core)

### 2. Configure Credentials

Edit [config.h](config.h) with your settings:

```cpp
char ssid[32] = "your_wifi_ssid";
char password[64] = "your_wifi_password";
char mqtt_broker[64] = "192.168.1.66";  // Bambu Lab printer IP
char topic[64] = "device/+/report";
char mqtt_username[32] = "bblp";
char mqtt_password[32] = "your_printer_password";  // From printer LCD
int mqtt_port = 8883;
int fanPin = 6;
float tempThreshold = 180.0;
unsigned long fanOffDelay = 300000;  // 5 minutes in ms
String authToken = "-1";  // API token or "-1" to disable
```

### 3. Upload Firmware

1. Open `bentobrain.ino` in Arduino IDE
2. Select **Board**: ESP32 Dev Module
3. Select appropriate **Port**
4. Click **Upload**
5. Monitor serial output at 115200 baud

### 4. Access Web Interface

After successful connection, access:
- Main control: `http://<esp32-ip>/`
- Configuration: `http://<esp32-ip>/config`

## MQTT Integration - Bambu Lab Printer

### Connection Details
- **Protocol**: MQTT over TLS (port 8883)
- **Topic Pattern**: `device/+/report` (subscribes to all devices)
- **Authentication**: Username `bblp` + printer password (obtain from LCD screen: Settings → Network → Access Code)
- **Buffer Size**: 12KB (handles large JSON payloads)
- **Keep-Alive**: 60 seconds
- **TLS**: Self-signed certificates accepted (`espClient.setInsecure()`)

### Expected JSON Structure

```json
{
  "print": {
    "nozzle_temper": 210.5
  }
}
```

The controller extracts `nozzle_temper` from the `print` object and uses it for automatic fan control.

## Key Functions

- `controlFan(float nozzle_temper)` - Core fan control logic (manual or automatic)
- `setup_wifi()` - WiFi initialization and OTA setup
- `reconnect()` - MQTT connection/reconnection handling
- `callback(char* topic, byte* payload, unsigned int length)` - MQTT message handler
- `handleConfigPage()` - Web config interface handler
- `handleSetFanSpeedAPI()` - REST API endpoint handler
- `loadSettings()` / `saveSettings()` - NVS persistence

## State Management

- `manualFanSpeed`: -1 for automatic, 0-100 for manual control
- `isFanOn`: Current fan state
- `nozzle_temper`: Latest temperature reading from MQTT
- `lastTimeBelowThreshold`: Timestamp for delay calculation

## API Usage

### Set Fan Speed via API

```bash
# Set fan to 75%
curl "http://<esp32-ip>/api/setFanSpeed?token=your_token&speed=75"

# Turn off fan
curl "http://<esp32-ip>/api/setFanSpeed?token=your_token&speed=0"

# Return to automatic mode
curl "http://<esp32-ip>/api/setFanSpeed?token=your_token&speed=-1"
```

## Development

### Common Tasks

#### Modifying Temperature Thresholds
- Update `tempThreshold` in [config.h](config.h)
- OR change via web interface at `/config`

#### Changing Fan Control Logic
- Edit `controlFan()` function in [bentobrain.ino](bentobrain.ino:125-145)
- Consider both manual and automatic modes

#### Adding New MQTT Fields
- Update `callback()` in [bentobrain.ino](bentobrain.ino:196-217)
- Add JSON extraction for new fields
- May need to increase JSON document size

#### Updating Web UI
- HTML generation in `generateHtmlPage()` for main page
- `getConfigPageHTML()` for config page
- Both use inline CSS for styling

### Security Considerations

⚠️ **Important Security Notes:**
- WiFi credentials stored in plaintext in NVS
- MQTT password exposed in config.h (change before deployment)
- Self-signed TLS certificates accepted (vulnerable to MITM)
- API token can be disabled or set to custom value
- No HTTPS on web server (credentials sent unencrypted)

### Known Limitations

1. Single WiFi network support (no multi-network or fallback)
2. No error recovery for WiFi disconnection (requires restart)
3. No temperature sensor validation (trusts MQTT data)
4. Web interface doesn't update automatically (manual refresh required)
5. No logging or diagnostics beyond Serial output

## Serial Debug Output

The device outputs diagnostic information at 115200 baud:
- WiFi connection status and IP address
- MQTT connection attempts and message counts
- OTA update progress
- Settings loaded from NVS

## Typical Use Cases

1. **Cooling ABS/ASA prints** - Automatically ventilates enclosure during high-temp printing
2. **Post-print cooling** - Keeps fan running for 5 minutes after print completes
3. **Manual ventilation** - Use slider for quick air circulation
4. **External control** - HomeAssistant/NodeRED integration via REST API

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password in config.h
- Ensure ESP32 is within WiFi range
- Check that network is 2.4GHz (ESP32 doesn't support 5GHz)

### MQTT Connection Failures
- Verify printer IP address
- Confirm MQTT port (8883 for TLS)
- Check access code from printer LCD screen
- Ensure printer and ESP32 are on same network

### Fan Not Responding
- Verify fan pin (GPIO6) matches your wiring
- Check PWM frequency (25kHz) is compatible with your fan
- Test fan with manual control via web interface
- Check power supply to fan

### OTA Update Fails
- Ensure ESP32 is on same network as computer
- Check that port 3232 is not blocked by firewall
- Verify sufficient flash space for new firmware

## Alternative: ESPHome Version

A simpler ESPHome implementation is available in the `../esphome/` directory that:
- Eliminates MQTT complexity (uses Home Assistant integration)
- Removes manual control (automatic only)
- Simplifies configuration (YAML vs C++)
- Requires Home Assistant

See [../esphome/README.md](../esphome/README.md) for details.

## Notes for AI Assistants

- **This is production hardware code** - Changes affect physical devices
- **Test MQTT JSON parsing carefully** - Malformed updates can break temperature monitoring
- **PWM changes affect fan lifespan** - Respect 25kHz frequency for 4-pin PWM fans
- **NVS has limited write cycles** - Don't save settings too frequently
- **The config.h contains sensitive data** - Remind users to update credentials before deployment
