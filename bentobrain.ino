#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "config.h"
#include "mqtt.h"

//No User Changes in this document


// MQTT Broker settings
const char* topic = "/dev/#";
const int mqtt_port = 8883; // MQTT port (usually 8883 for SSL)
const char* mqtt_username = "bblp";


// Other Variable Definitions
unsigned long lastTimeBelowThreshold = 0; // Time when temperature dropped below the threshold
bool isFanOn = false;
int manualFanSpeed = 50;  // Default value
const int defaultValue = 50;  // Define the default value
bool userRequestedReset = false;



// HTML for the web page
const char* htmlPage = 
  "<html><body>"
  "<h1>ESP32 Variable Control</h1>"
  "<p>Current Value: %d%%</p>"
  "<form action=\"/set\" method=\"POST\">"
  "<input type=\"range\" id=\"slider\" name=\"value\" min=\"0\" max=\"100\" value=\"%d\" onchange=\"updateValue(value)\" style=\"width:300px;\">"
  "<input type=\"submit\" value=\"Set Value\">"
  "</form>"
  "<script>function updateValue(val) { document.getElementById('slider').value=val; }</script>"
  "</body></html>";

WiFiClientSecure espClient;
PubSubClient client(espClient);
WebServer server(80);

void controlFan(float nozzle_temper) {
  if (nozzle_temper > tempThreshold) {
    // Turn the fan on at 100% if temperature is above the threshold
    analogWrite(fanPin, 255);
    isFanOn = true;
  } else if (nozzle_temper < tempThreshold && isFanOn) {
    // Record the time when temperature drops below the threshold
    lastTimeBelowThreshold = millis();
    isFanOn = false;
  }
}

void setup_wifi() {
  delay(10);
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(12000);

  // Configure WiFiClientSecure to accept self-signed certificates
  espClient.setInsecure();

  pinMode(fanPin, OUTPUT);
  analogWrite(fanPin, 0); // Start with the fan off



  // Define Web Server routes
  server.on("/", HTTP_GET, []() {
    char temp[600];
    snprintf(temp, 600, htmlPage, manualFanSpeed, manualFanSpeed);
    server.send(200, "text/html", temp);
  });
  
  server.on("/set", HTTP_POST, []() {
    if (server.hasArg("value")) {
      manualFanSpeed = server.arg("value").toInt();
    }
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  // Start the server
  server.begin();
}



void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Turn off the fan after the specified delay duration after temperature drops below the threshold
  if (!isFanOn && millis() - lastTimeBelowThreshold > fanOffDelay) {
    analogWrite(fanPin, 0);
  }
  server.handleClient();
  
}
