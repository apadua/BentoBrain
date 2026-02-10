#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include "config.h"

// Function declarations
void handleConfigPage();
void handleSetFanSpeedAPI();
void loadSettings();
void saveSettings();



Preferences preferences;
WiFiClientSecure espClient;
PubSubClient client(espClient);
WebServer server(80);

unsigned long lastTimeBelowThreshold = 0; // Time when temperature dropped below the threshold
unsigned long messageCount = 0;  // Counter for the number of messages received
unsigned long MQTTconnectCount = 0;  // Counter for the number of reconnections
String lastBSSID = ""; // Global variable to store the last connected BSSID
long lastRSSI = 0;     // Global variable to store the last RSSI value
bool isFanOn = false;
int manualFanSpeed = -1;  // Used for WebServer logic
const int defaultFanSpeed = -1;  // Used for Webserver logic
float nozzle_temper = 0;
#define PWM_CHANNEL 0
#define PWM_FREQ 25000 // 25 kHz
#define PWM_RESOLUTION 8 // 8-bit resolution


// Function to generate HTML content based on manualFanSpeed
String generateHtmlPage() {
  String htmlContent = 
    "<!DOCTYPE html>"
    "<html><head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<style>"
    "body { background-color: #FFFFFF; font-family: Arial, sans-serif; margin: 0; padding: 40px; font-size: 20px; color: rgb(255,90,44); }"
    "h1 { font-size: 2em; }"
    "input[type=range] { -webkit-appearance: none; margin: 18px 0; width: 100%; height: 25px; background: rgb(255,90,44); outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s; }"
    "input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #000000; cursor: pointer; }"
    "input[type=range]::-moz-range-thumb { width: 35px; height: 35px; background: #000000; cursor: pointer; }"
    "button[type=button] { background-color: rgb(255,90,44); color: #FFFFFF; font-size: 1em; padding: 10px; border: none; cursor: pointer; }"
    "input[type=submit] { display: none; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>BentoBrain<br/>Fan Control</h1>"
    "<p>Current Value: " + (manualFanSpeed == -1 ? "Automatic" : (String(manualFanSpeed) + "%")) + "</p>"
    "<form action=\"/set\" method=\"POST\">"
    "<input type=\"range\" id=\"slider\" class=\"slider\" name=\"value\" min=\"0\" max=\"100\" step=\"10\" value=\"" + (manualFanSpeed == -1 ? "0" : String(manualFanSpeed)) + "\" oninput=\"updateValue(value)\" onchange=\"sendValue()\" style=\"width:300px;\"><br>"
    "<input type=\"submit\" value=\"Set Fan Percent\">"
    "<button type=\"button\" style=\"width:300px;\" onclick=\"setAutomatic()\">Set to Automatic</button>"
    "</form>"
    "<a href=\"/config\" style=\"display:block; width:300px; text-align:center; background-color: rgb(255,90,44); color: #FFFFFF; font-size: 1em; padding: 10px; border: none; cursor: pointer; text-decoration: none; margin-top: 10px;\">Config</a>"

    "<script>"
    "function updateValue(val) {"
    "  document.querySelector('p').textContent = 'Current Value: ' + val + '%';"
    "}"
    "function sendValue() {"
    "  var val = document.getElementById('slider').value;"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('POST', '/set', true);"
    "  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
    "  xhr.send('value=' + val);"
    "}"
    "function setAutomatic() {"
    "  document.querySelector('p').textContent = 'Current Value: Automatic';"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('POST', '/set', true);"
    "  xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
    "  xhr.send('value=-1');"
    "  document.getElementById('slider').value = -1;"
    "}"
    "</script>"
    "</body>"
    "</html>";

  return htmlContent;
}
//String that generates the Config Page
String getConfigPageHTML() {
    String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 Config Page</title></head><body>";
    html += "<h2>ESP32 Configuration</h2>";
    html += "<form method='post' action='/config'>";

    // Network Credentials
    html += "<h3>Network Credentials</h3>";
    html += "SSID:<br><input type='text' name='ssid' value='" + String(ssid) + "'><br>";
    html += "Password:<br><input type='text' name='password' value='" + String(password) + "'><br>";

    // Printer Settings
    html += "<h3>Printer Settings</h3>";
    html += "MQTT Broker:<br><input type='text' name='mqtt_broker' value='" + String(mqtt_broker) + "'><br>";
    html += "Topic:<br><input type='text' name='topic' value='" + String(topic) + "'><br>";
    html += "MQTT Username:<br><input type='text' name='mqtt_username' value='" + String(mqtt_username) + "'><br>";
    html += "MQTT Password:<br><input type='text' name='mqtt_password' value='" + String(mqtt_password) + "'><br>";
    html += "MQTT Port:<br><input type='number' name='mqtt_port' value='" + String(mqtt_port) + "'><br>";

    // Other Settings
    html += "<h3>Other Settings</h3>";
    html += "Fan Pin:<br><input type='number' name='fanPin' value='" + String(fanPin) + "'><br>";
    html += "Temperature Threshold:<br><input type='number' name='tempThreshold' value='" + String(tempThreshold) + "' step='0.1'><br>";
    html += "Fan Off Delay:<br><input type='number' name='fanOffDelay' value='" + String(fanOffDelay) + "'><br>";

    // API Key
    html += "<h3>API Key</h3>";
    html += "Auth Token:<br><input type='text' name='authToken' value='" + authToken + "'><br>";
    html += "<input type='submit' name='action' value='Save'>";
    html += "<input type='submit' name='action' value='Save and Restart'>";
    html += "</form></body></html>";
    return html;
}

void controlFan(float nozzle_temper) {
  if (manualFanSpeed >= 0 && manualFanSpeed <= 100) {
    // Manual control: Convert 1-100 range to 0-255 PWM value
    ledcWrite(PWM_CHANNEL, map(manualFanSpeed, 0, 100, 0, 255));
    isFanOn = true;
  } else {
    // Automatic control based on temperature
    if (nozzle_temper > tempThreshold) {
      ledcWrite(PWM_CHANNEL, 255);
      isFanOn = true;
    } else if (nozzle_temper < tempThreshold && isFanOn) {
      lastTimeBelowThreshold = millis();
      isFanOn = false;
    }
    
    // If fan is off and manualFanSpeed is -1 (automatic mode), turn off the fan immediately
    if (!isFanOn) {
      ledcWrite(PWM_CHANNEL, 0);
    }
  }
}

void setup_wifi() {
  delay(10);
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

   // OTA event handlers setup
  ArduinoOTA.onStart([]() {
    // Code to run on start of OTA update
    Serial.println("OTA Update Start");
  });
  ArduinoOTA.onEnd([]() {
    // Code to run on end of OTA update
    Serial.println("\nOTA Update End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Code to run during OTA update progress
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    // Code to run on OTA update error
    Serial.printf("OTA Error [code %u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  // Initialize OTA
  ArduinoOTA.begin();

  Serial.println("");
  Serial.print("Connected to SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Connection Quality (RSSI)): ");
  Serial.println(WiFi.RSSI());
  Serial.println("");
  Serial.print("WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  messageCount++;  // Increment the message count

  Serial.print("Message ");
  Serial.print(messageCount);
  Serial.print(" arrived on MQTT Connection #");
  Serial.println(MQTTconnectCount);

  // Reduce the size of the JSON document
  DynamicJsonDocument doc(1024); // Adjust this size

  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  // Extract the device temperatures
  nozzle_temper = doc["print"]["nozzle_temper"];

}

void reconnect() {
  if (client.connected()) {
    return;
  }
  MQTTconnectCount++;  // Increment the reconnect counter
  messageCount = 0;  // Reset the message count on reconnect

  Serial.print("Connection attempt #");
  Serial.println(MQTTconnectCount);

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("BentoBrain", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker!");
      // Subscribe to the topic
      client.subscribe(topic);
      Serial.println("Subscribed to topic: " + String(topic));
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handleConfigPage() {
    if (server.method() == HTTP_GET) {
        server.send(200, "text/html", getConfigPageHTML());
    } else if (server.method() == HTTP_POST) {
        // Parse and save new settings
        strncpy(ssid, server.arg("ssid").c_str(), sizeof(ssid));
        strncpy(password, server.arg("password").c_str(), sizeof(password));
        strncpy(mqtt_broker, server.arg("mqtt_broker").c_str(), sizeof(mqtt_broker));
        strncpy(topic, server.arg("topic").c_str(), sizeof(topic));
        strncpy(mqtt_username, server.arg("mqtt_username").c_str(), sizeof(mqtt_username));
        strncpy(mqtt_password, server.arg("mqtt_password").c_str(), sizeof(mqtt_password));
        mqtt_port = server.arg("mqtt_port").toInt();
        fanPin = server.arg("fanPin").toInt();
        tempThreshold = server.arg("tempThreshold").toFloat();
        fanOffDelay = server.arg("fanOffDelay").toInt();
        authToken = server.arg("authToken");

        // Save settings to NVS
        saveSettings();
        if (server.hasArg("action")) {
            String action = server.arg("action");
            if (action == "Save") {
                // Just save the settings
                Serial.println("Settings Saved.");
            } else if (action == "Save and Restart") {
                // Save the settings and schedule a restart
                Serial.println("Settings Saved. Device will restart in 5 seconds.");
                delay(5000); // Wait for 5 seconds
                ESP.restart(); // Restart the ESP32
            }
    }
    // Redirect back to config page or indicate that the settings have been saved
    server.sendHeader("Location", "/config");
    server.send(303);

        // Redirect back to config page or a confirmation page
        server.sendHeader("Location", "/config");
        server.send(303);
    } else {
        server.send(405);
    }
}

void handleSetFanSpeedAPI() {
  // Check if API access is disabled
  if (authToken == "-1") {
    server.send(403, "text/plain", "API Access is disabled");
    return;
  }
  
  // Proceed with the existing logic
  if (server.hasArg("token") && server.hasArg("speed")) {
    String token = server.arg("token");
    if (token == authToken) {
      int speed = server.arg("speed").toInt();
      // Add your code to set the fan speed here
      manualFanSpeed = speed;
      server.send(200, "text/plain", "Fan speed set to " + String(manualFanSpeed) + "%");
    } else {
      server.send(403, "text/plain", "Forbidden: Invalid Token");
    }
  } else {
    server.send(400, "text/plain", "Bad Request: Missing 'token' or 'speed' parameter");
  }
}

// Function to load settings from NVS
void loadSettings() {
    preferences.begin("settings", true); // ReadOnly
    String temp; // Temporary String to hold the preference values

    temp = preferences.getString("ssid", ssid);
    strncpy(ssid, temp.c_str(), sizeof(ssid) - 1);
    ssid[sizeof(ssid) - 1] = '\0'; // Ensure null termination
    

    temp = preferences.getString("password", password);
    strncpy(password, temp.c_str(), sizeof(password) - 1);
    password[sizeof(password) - 1] = '\0'; // Ensure null termination

    temp = preferences.getString("mqtt_broker", mqtt_broker);
    strncpy(mqtt_broker, temp.c_str(), sizeof(mqtt_broker) - 1);
    mqtt_broker[sizeof(mqtt_broker) - 1] = '\0'; // Ensure null termination

    temp = preferences.getString("topic", topic);
    Serial.print("MQTT Topic: ");
    Serial.println(temp);
    strncpy(topic, temp.c_str(), sizeof(topic) - 1);
    topic[sizeof(topic) - 1] = '\0'; // Ensure null termination

    temp = preferences.getString("mqtt_username", mqtt_username);
    strncpy(mqtt_username, temp.c_str(), sizeof(mqtt_username) - 1);
    mqtt_username[sizeof(mqtt_username) - 1] = '\0'; // Ensure null termination

    temp = preferences.getString("mqtt_password", mqtt_password);
    strncpy(mqtt_password, temp.c_str(), sizeof(mqtt_password) - 1);
    mqtt_password[sizeof(mqtt_password) - 1] = '\0'; // Ensure null termination

    fanPin = preferences.getInt("fanPin", fanPin);
    tempThreshold = preferences.getFloat("tempThreshold", tempThreshold);
    fanOffDelay = preferences.getULong("fanOffDelay", fanOffDelay);
    authToken = preferences.getString("authToken", authToken);
    preferences.end();
}

// Function to save settings to NVS
void saveSettings() {
    preferences.begin("settings", false); // ReadWrite
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("mqtt_broker", mqtt_broker);
    preferences.putString("topic", topic);
    preferences.putString("mqtt_username", mqtt_username);
    preferences.putString("mqtt_password", mqtt_password);
    preferences.putInt("mqtt_port", mqtt_port);
    preferences.putInt("fanPin", fanPin);
    preferences.putFloat("tempThreshold", tempThreshold);
    preferences.putULong("fanOffDelay", fanOffDelay);
    preferences.putString("authToken", authToken);
    preferences.end();
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(12000);

  // Load settings from NVS
    loadSettings();

  // Set the keep-alive interval (e.g., 60 seconds)
  client.setKeepAlive(60);

  // Configure WiFiClientSecure to accept self-signed certificates
  espClient.setInsecure();

  //Configure PWM Details
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(fanPin, PWM_CHANNEL);

  // Define server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", generateHtmlPage());
  });
  
  server.on("/set", HTTP_POST, []() {
    if (server.hasArg("value")) {
      manualFanSpeed = server.arg("value").toInt();
    }
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/reset", HTTP_GET, []() {
    manualFanSpeed = defaultFanSpeed;
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.on("/config", handleConfigPage);
  server.on("/api/setFanSpeed", handleSetFanSpeedAPI);

  // Start the server
  server.begin();

}

void loop() {
    server.handleClient();

  // Handle OTA
  ArduinoOTA.handle();
  // Handle manual fan speed changes instantly
  if (manualFanSpeed != -1 || (manualFanSpeed == -1 && !isFanOn)) {
    controlFan(nozzle_temper);  // You need to ensure nozzle_temper is available here, or fetch it before this line.
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Turn off the fan after the specified delay duration if in automatic mode
  if (manualFanSpeed == -1 && isFanOn && millis() - lastTimeBelowThreshold > fanOffDelay) {
    ledcWrite(PWM_CHANNEL, 0);
    isFanOn = false;
  }

}
