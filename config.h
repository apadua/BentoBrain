// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Replace with your WIFI Network name and Password
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Use Printer IP Address and LAN Password grabbed from the LCD screen
const char* mqtt_broker = "printer_address";
const char* mqtt_password = "printer_lan_password";

// Other Adjustments 
const int fanPin = 6; // PWM pin for the fan is connected to pin 6
const float tempThreshold = 180.0; // Temperature threshold for fan control. 180 will trigger fan ON for any filament type
const unsigned long fanOffDelay = 300000; // Delay duration before fan shutoff in milliseconds (300000 is 5 minutes)



// Only change these if you know what you're doing
const char* topic = "device/#";
const char* mqtt_username = "bblp";
const int mqtt_port = 8883; // MQTT port (usually 8883 for SSL)
