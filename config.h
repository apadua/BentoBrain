// config.h
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// MQTT Broker settings
const char* mqtt_broker = "broker_address";
const char* mqtt_password = "your_password";


//Fan Control Settings
const int fanPin = 6; // PWM pin for the fan is connected to pin 6
const float tempThreshold = 200.0; // Temperature threshold for fan control. 200 triggers fan ON for any filament type
const unsigned long fanOffDelay = 300000; // Delay duration before fan shutoff in milliseconds (300000 is 5 minutes)
#endif
