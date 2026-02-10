# BentoBrain - Smart 3D Printer Fan Controller

## Project Overview

BentoBrain is an ESP32-based smart fan controller designed for 3D printers (specifically Bambu Lab printers). It automatically controls a fan based on printer activity to help with cooling during and after printing.

**Two implementations are available:**
1. **Arduino** - Feature-rich standalone version with manual control
2. **ESPHome** - Ultra-simple Home Assistant integration

## Hardware

- **Platform**: ESP32 microcontroller
- **Fan Control**: PWM-based (25 kHz, 8-bit resolution)
- **Default Fan Pin**: GPIO 6
- **Communication**: WiFi (2.4GHz)

## Implementation Comparison

| Feature | Arduino | ESPHome |
|---------|---------|---------|
| **Data Source** | Direct MQTT from printer | HA Bambu Lab integration |
| **Standalone** | ✅ Yes | ❌ Requires Home Assistant |
| **Manual Control** | ✅ Web UI + REST API | ❌ Automatic only |
| **Temperature Monitoring** | ✅ Direct nozzle temp | ❌ Print status only |
| **Configuration** | Web interface | Home Assistant UI |
| **Complexity** | Higher (more features) | Lower (simpler) |
| **Setup Difficulty** | Moderate (MQTT config) | Easy (WiFi only) |
| **Persistence** | ✅ Settings saved | ❌ Resets on reboot |

## Choose Your Implementation

### Arduino Version → [arduino/README.md](arduino/README.md)

**Choose this if you:**
- Want standalone operation (no Home Assistant required)
- Need manual fan control via web interface
- Want direct temperature monitoring from printer
- Need REST API for external integrations
- Prefer feature-rich solution

**Key Features:**
- 🌡️ Temperature-based automatic control
- 🖱️ Manual override via web interface
- 🔧 Web-based configuration page
- 📡 REST API for automation
- 💾 Settings persisted in NVS
- 🔐 Direct MQTT connection to printer

**Quick Start:**
```bash
cd arduino
# Edit config.h with your credentials
# Open bentobrain.ino in Arduino IDE and upload
```

---

### ESPHome Version → [esphome/README.md](esphome/README.md)

**Choose this if you:**
- Already use Home Assistant
- Have Bambu Lab integration configured
- Want minimal setup and maintenance
- Prefer "set it and forget it" automation
- Don't need manual control

**Key Features:**
- 🏠 Native Home Assistant integration
- 📊 Uses existing Bambu Lab integration
- ⚡ Auto-discovery in HA
- 🎯 Single-purpose automation
- 🔄 Simple YAML configuration
- 🚫 No MQTT complexity

**Quick Start:**
```bash
cd esphome
cp secrets.yaml.example secrets.yaml
# Edit secrets.yaml
esphome run bentobrain.yaml
```

## Project Structure

```
BentoBrain/
├── arduino/              # Arduino/C++ implementation
│   ├── bentobrain.ino   # Main Arduino code
│   ├── config.h         # Configuration (gitignored)
│   ├── mqtt.h           # MQTT utilities
│   └── README.md        # Arduino documentation
│
├── esphome/             # ESPHome/YAML implementation
│   ├── bentobrain.yaml  # Main ESPHome config
│   ├── secrets.yaml     # Credentials (gitignored)
│   ├── secrets.yaml.example
│   └── README.md        # ESPHome documentation
│
└── CLAUDE.md            # This file - project overview
```

## Common Use Cases

### Arduino is Best For:
1. **Standalone operation** - No Home Assistant available
2. **Manual control** - Need to adjust fan speed on demand
3. **External automation** - REST API for scripts/integrations
4. **Direct monitoring** - Want precise temperature readings
5. **Feature flexibility** - Need all options available

### ESPHome is Best For:
1. **Home Assistant users** - Already have HA setup
2. **Simplicity** - Want minimal configuration
3. **Automation** - Pure automatic operation
4. **Integration** - Leverage existing Bambu Lab setup
5. **Maintenance** - Easy updates via OTA

## Hardware Setup (Both Versions)

### Wiring
```
ESP32 GPIO6 → Fan PWM Input (Yellow wire on 4-pin fan)
ESP32 GND   → Fan Ground (Black wire)
External PS → Fan +12V/24V (Red wire)
External PS → ESP32 VIN (if powering ESP32)
```

### Notes
- 4-pin PWM fans are recommended
- PWM frequency: 25 kHz
- Fan voltage: Match your fan specs (usually 12V or 24V)
- ESP32 provides PWM signal only (low voltage)
- Use external power supply for fan power

## Development Guidelines

### When Adding Features

1. **Identify the right implementation**
   - Arduino: Add to [arduino/](arduino/)
   - ESPHome: Add to [esphome/bentobrain.yaml](esphome/bentobrain.yaml)

2. **Follow existing patterns**
   - Arduino: C++ with Web UI and NVS storage
   - ESPHome: YAML with Home Assistant entities

3. **Update documentation**
   - Arduino changes → Update [arduino/README.md](arduino/README.md)
   - ESPHome changes → Update [esphome/README.md](esphome/README.md)

### Security Considerations

- **Credentials**: Both versions gitignore sensitive files
- **Arduino**: `arduino/config.h` is gitignored
- **ESPHome**: `esphome/secrets.yaml` is gitignored
- Always update credentials before deploying
- Consider network security (VLANs, firewalls)

### For AI Assistants

- **This is production hardware code** - Changes affect physical devices
- **Two separate implementations** - Don't mix Arduino and ESPHome code
- **Check context** - Determine which version user is working with
- **Test carefully** - PWM changes can affect fan lifespan
- **Respect design philosophy**:
  - Arduino: Feature-rich, flexible
  - ESPHome: Simple, automatic

## Typical Workflow

### Initial Setup
1. Choose implementation (Arduino or ESPHome)
2. Wire ESP32 to fan
3. Configure credentials
4. Upload firmware
5. Verify operation

### Daily Use
- **Arduino**: Access web interface for manual control
- **ESPHome**: Monitor via Home Assistant dashboard

### Configuration Changes
- **Arduino**: Use web config page or edit config.h
- **ESPHome**: Edit secrets.yaml or adjust in Home Assistant

### Firmware Updates
- **Arduino**: Use Arduino IDE or OTA
- **ESPHome**: Use ESPHome CLI or HA integration

## Troubleshooting Quick Reference

| Issue | Arduino Solution | ESPHome Solution |
|-------|-----------------|------------------|
| WiFi won't connect | Check config.h | Check secrets.yaml |
| Fan not responding | Test via web UI | Check HA logs |
| No temperature data | Verify MQTT connection | Verify HA sensor |
| Can't access web UI | Check serial for IP | Access via HA |
| Settings lost | Check NVS writes | Expected (no persistence) |

For detailed troubleshooting, see implementation-specific README files.

## Additional Resources

- [Arduino Documentation](arduino/README.md) - Complete Arduino guide
- [ESPHome Documentation](esphome/README.md) - Complete ESPHome guide
- [ESPHome Official Docs](https://esphome.io)
- [Home Assistant Bambu Lab Integration](https://www.home-assistant.io/integrations/bambu_lab/)

## Contributing

When contributing to this project:
1. Identify which implementation you're modifying
2. Follow the existing code style
3. Update the appropriate README.md
4. Test thoroughly with actual hardware
5. Don't commit sensitive credentials

## License

This project is open source. See individual files for license information.

## Support

- **Arduino Issues**: See [arduino/README.md](arduino/README.md)
- **ESPHome Issues**: See [esphome/README.md](esphome/README.md)
- **General Questions**: Check this overview first

---

**Quick Decision Guide:**
- Have Home Assistant? → Try [ESPHome](esphome/README.md) first
- Need standalone? → Use [Arduino](arduino/README.md)
- Want simplicity? → Choose [ESPHome](esphome/README.md)
- Want all features? → Choose [Arduino](arduino/README.md)
