/*
  ESP32-C3 Climate Sensor with Web Interface
  Starts in AP mode for WiFi configuration
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <DHT.h>

// Include configuration
#include "config_climate.h"

// Global objects
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;
DHT dht(DHT_PIN, DHT_TYPE);

// Configuration
String wifiSSID = "";
String wifiPassword = "";
float temperatureOffset = 0.0;
float humidityOffset = 0.0;
unsigned int readingInterval = DEFAULT_READING_INTERVAL;

// Runtime state
float currentTemperature = 0.0;
float currentHumidity = 0.0;
unsigned long lastReadingTime = 0;
bool wifiConnected = false;
bool apMode = false;

void saveConfig() {
  preferences.begin(NVS_NAMESPACE, false);
  preferences.putString("wifiSSID", wifiSSID);
  preferences.putString("wifiPassword", wifiPassword);
  preferences.putFloat("tempOffset", temperatureOffset);
  preferences.putFloat("humidityOffset", humidityOffset);
  preferences.putUInt("readingInterval", readingInterval);
  preferences.end();
  Serial.println("Configuration saved");
}

void loadConfig() {
  preferences.begin(NVS_NAMESPACE, false);
  wifiSSID = preferences.getString("wifiSSID", "");
  wifiPassword = preferences.getString("wifiPassword", "");
  temperatureOffset = preferences.getFloat("tempOffset", 0.0);
  humidityOffset = preferences.getFloat("humidityOffset", 0.0);
  readingInterval = preferences.getUInt("readingInterval", DEFAULT_READING_INTERVAL);
  preferences.end();
  Serial.println("Configuration loaded");
}

void connectToWiFi() {
  if (wifiSSID.length() == 0) {
    wifiConnected = false;
    return;
  }
  
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifiSSID);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  for (int i = 0; i < 30; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  wifiConnected = false;
  Serial.println("\nFailed to connect");
}

void startAPMode() {
  apMode = true;
  String apSSID = String(AP_SSID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);
  apSSID.toUpperCase();
  
  Serial.print("Starting AP Mode with SSID: ");
  Serial.println(apSSID);
  WiFi.softAP(apSSID.c_str());
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void readSensor() {
  if (millis() - lastReadingTime < readingInterval * 1000) return;
  
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp) && !isnan(hum)) {
    currentTemperature = temp + temperatureOffset;
    currentHumidity = hum + humidityOffset;
    lastReadingTime = millis();
    Serial.print("Temperature: ");
    Serial.print(currentTemperature);
    Serial.print("°C, Humidity: ");
    Serial.print(currentHumidity);
    Serial.println("%");
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Climate Sensor</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:Arial;max-width:600px;margin:20px auto;padding:20px;}</style></head><body>";
  html += "<h1>🌡️ ESP32 Climate Sensor</h1>";
  
  if (apMode) {
    html += "<div style='background:#e7f3fe;padding:15px;border-radius:4px;margin-bottom:20px;'>";
    html += "<strong>Setup Mode</strong><br>Connect to WiFi to configure</div>";
  } else if (wifiConnected) {
    html += "<div style='background:#d4edda;padding:15px;border-radius:4px;margin-bottom:20px;'>";
    html += "<strong>Connected</strong><br>IP: " + WiFi.localIP().toString() + "</div>";
  }
  
  html += "<div style='font-size:48px;text-align:center;margin:30px 0;'>";
  html += "<div style='color:#dc3545;'>" + String(currentTemperature, 1) + "°C</div>";
  html += "<div style='color:#2196F3;'>" + String(currentHumidity, 1) + "%</div>";
  html += "</div>";
  
  html += "<a href='/config' style='display:block;background:#4CAF50;color:white;padding:15px;text-align:center;text-decoration:none;border-radius:4px;margin:10px 0;'>⚙️ Configure</a>";
  html += "<a href='/api/sensor' style='display:block;background:#2196F3;color:white;padding:15px;text-align:center;text-decoration:none;border-radius:4px;margin:10px 0;'>🔗 API</a>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleConfig() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Configuration</title>";
  html += "<style>body{font-family:Arial;max-width:600px;margin:20px auto;padding:20px;}</style></head><body>";
  html += "<h1>⚙️ Configuration</h1>";
  html += "<form method='POST' action='/save'>";
  html += "<label>WiFi SSID:</label><br><input type='text' name='ssid' value='" + wifiSSID + "' style='width:100%;padding:8px;margin:5px 0 15px 0;'><br>";
  html += "<label>WiFi Password:</label><br><input type='password' name='password' value='" + wifiPassword + "' style='width:100%;padding:8px;margin:5px 0 15px 0;'><br>";
  html += "<label>Temperature Offset (°C):</label><br><input type='number' name='tempOffset' value='" + String(temperatureOffset, 1) + "' step='0.1' style='width:100%;padding:8px;margin:5px 0 15px 0;'><br>";
  html += "<label>Humidity Offset (%):</label><br><input type='number' name='humidityOffset' value='" + String(humidityOffset, 1) + "' step='0.1' style='width:100%;padding:8px;margin:5px 0 15px 0;'><br>";
  html += "<label>Reading Interval (seconds):</label><br><input type='number' name='readingInterval' value='" + String(readingInterval) + "' min='5' max='3600' style='width:100%;padding:8px;margin:5px 0 15px 0;'><br>";
  html += "<button type='submit' style='background:#4CAF50;color:white;padding:15px;width:100%;border:none;border-radius:4px;font-size:16px;'>💾 Save</button>";
  html += "</form>";
  html += "<hr><a href='/' style='display:block;text-align:center;color:#2196F3;'>← Back to Home</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("ssid")) wifiSSID = server.arg("ssid");
  if (server.hasArg("password")) wifiPassword = server.arg("password");
  if (server.hasArg("tempOffset")) temperatureOffset = server.arg("tempOffset").toFloat();
  if (server.hasArg("humidityOffset")) humidityOffset = server.arg("humidityOffset").toFloat();
  if (server.hasArg("readingInterval")) readingInterval = server.arg("readingInterval").toInt();
  
  saveConfig();
  
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Saved</title>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "<style>body{font-family:Arial;max-width:600px;margin:20px auto;padding:20px;text-align:center;}</style></head><body>";
  html += "<h1 style='color:#28a745;'>✓ Configuration Saved</h1>";
  html += "<p>Redirecting to home page...</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSensorData() {
  String json = "{";
  json += "\"temperature\":" + String(currentTemperature, 1) + ",";
  json += "\"humidity\":" + String(currentHumidity, 1) + ",";
  json += "\"timestamp\":" + String(millis() / 1000);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\nESP32-C3 Climate Sensor");
  Serial.println("========================");
  
  pinMode(CONFIG_BUTTON_PIN, INPUT_PULLDOWN);
  dht.begin();
  loadConfig();
  
  // Check if we should start in AP mode
  bool gpioTrigger = (digitalRead(CONFIG_BUTTON_PIN) == HIGH);
  bool hasCredentials = (wifiSSID.length() > 0);
  
  if (!hasCredentials || gpioTrigger) {
    Serial.println("Starting in AP mode");
    startAPMode();
  } else {
    Serial.println("Attempting to connect to WiFi");
    connectToWiFi();
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/api/sensor", handleSensorData);
  server.begin();
  Serial.println("Web server started");
  
  // Initial sensor reading
  readSensor();
}

void loop() {
  if (apMode) {
    dnsServer.processNextRequest();
  }
  server.handleClient();
  
  // Handle WiFi reconnection
  if (!apMode && WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    connectToWiFi();
  }
  
  readSensor();
  delay(100);
}