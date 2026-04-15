/*
  ESP32-C3 Climate Sensor - Simple Version
  Temperature and humidity sensor with web interface
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DHT.h>

// Include configuration
#include "config_climate.h"

// Global objects
Preferences preferences;
WebServer server(WEB_SERVER_PORT);
DHT dht(DHT_PIN, DHT_TYPE);

// Configuration stored in NVS
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

// Forward declarations
void saveConfigToNVS();
void loadConfigFromNVS();
void connectToWiFi();
void readSensorData();
void handleRoot();
void handleConfig();
void handleSave();
void handleSensorData();

/*
 * Save all configuration to NVS
 */
void saveConfigToNVS() {
  preferences.begin(NVS_NAMESPACE, false);
  preferences.putString("wifiSSID", wifiSSID);
  preferences.putString("wifiPassword", wifiPassword);
  preferences.putFloat("tempOffset", temperatureOffset);
  preferences.putFloat("humidityOffset", humidityOffset);
  preferences.putUInt("readingInterval", readingInterval);
  preferences.end();
  
  #ifdef DEBUG_MODE
  Serial.println("Configuration saved to NVS");
  #endif
}

/*
 * Load all configuration from NVS
 */
void loadConfigFromNVS() {
  preferences.begin(NVS_NAMESPACE, false);
  wifiSSID = preferences.getString("wifiSSID", "");
  wifiPassword = preferences.getString("wifiPassword", "");
  temperatureOffset = preferences.getFloat("tempOffset", 0.0);
  humidityOffset = preferences.getFloat("humidityOffset", 0.0);
  readingInterval = preferences.getUInt("readingInterval", DEFAULT_READING_INTERVAL);
  preferences.end();
  
  #ifdef DEBUG_MODE
  Serial.println("Configuration loaded from NVS");
  #endif
}

/*
 * Connect to WiFi using stored credentials
 */
void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  if (wifiSSID.length() == 0) {
    #ifdef DEBUG_MODE
    Serial.println("No WiFi credentials configured");
    #endif
    wifiConnected = false;
    return;
  }
  
  #ifdef DEBUG_MODE
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifiSSID);
  #endif
  
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    #ifdef DEBUG_MODE
    Serial.print(".");
    #endif
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    #ifdef DEBUG_MODE
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    #endif
  } else {
    wifiConnected = false;
    #ifdef DEBUG_MODE
    Serial.println("\nFailed to connect to WiFi");
    #endif
  }
}

/*
 * Read sensor data from DHT22
 */
void readSensorData() {
  unsigned long currentTime = millis();
  unsigned long intervalMillis = readingInterval * 1000UL;
  
  // Check if it's time to read the sensor
  if (currentTime - lastReadingTime >= intervalMillis) {
    #ifdef DEBUG_MODE
    Serial.println("Reading sensor data...");
    #endif
    
    // Read temperature and humidity
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    
    // Check if readings failed
    if (isnan(temp) || isnan(hum)) {
      #ifdef DEBUG_MODE
      Serial.println("Failed to read from DHT sensor!");
      #endif
    } else {
      // Apply calibration offsets
      currentTemperature = temp + temperatureOffset;
      currentHumidity = hum + humidityOffset;
      
      // Clamp humidity to valid range (0-100%)
      if (currentHumidity < 0) currentHumidity = 0;
      if (currentHumidity > 100) currentHumidity = 100;
      
      lastReadingTime = currentTime;
      
      #ifdef DEBUG_MODE
      Serial.print("Temperature: ");
      Serial.print(currentTemperature);
      Serial.print("°C, Humidity: ");
      Serial.print(currentHumidity);
      Serial.println("%");
      #endif
    }
  }
}

/*
 * Handle root page
 */
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Climate Sensor</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:20px auto;padding:20px;}";
  html += "h1{color:#333;} .sensor-value{font-size:48px;font-weight:bold;text-align:center;margin:20px 0;}";
  html += ".temp{color:#dc3545;} .humidity{color:#2196F3;} .unit{font-size:24px;color:#666;}</style></head><body>";
  
  html += "<h1>🌡️ ESP32 Climate Sensor</h1>";
  
  if (wifiConnected) {
    html += "<div style='background:#d4edda;padding:15px;border-radius:4px;margin-bottom:20px;'>";
    html += "<strong>✓ WiFi Connected</strong><br>IP: " + WiFi.localIP().toString() + "</div>";
  } else {
    html += "<div style='background:#f8d7da;padding:15px;border-radius:4px;margin-bottom:20px;'>";
    html += "<strong>✗ WiFi Disconnected</strong></div>";
  }
  
  html += "<div class='sensor-value temp'>" + String(currentTemperature, 1) + "<span class='unit'>°C</span></div>";
  html += "<div class='sensor-value humidity'>" + String(currentHumidity, 1) + "<span class='unit'>%</span></div>";
  
  html += "<div style='margin-top:30px;'>";
  html += "<a href='/config' style='display:block;background:#4CAF50;color:white;padding:15px;text-align:center;text-decoration:none;border-radius:4px;margin:10px 0;'>📝 Configure</a>";
  html += "<a href='/api/sensor' style='display:block;background:#2196F3;color:white;padding:15px;text-align:center;text-decoration:none;border-radius:4px;margin:10px 0;'>🔗 API Data</a>";
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

/*
 * Handle configuration page
 */
void handleConfig() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Configuration</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:20px auto;padding:20px;}";
  html += "h1{color:#333;} input{width:100%;padding:8px;margin:5px 0 15px 0;border:1px solid #ddd;border-radius:4px;box-sizing:border-box;}";
  html += "button{background:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;width:100%;font-size:16px;}</style></head><body>";
  
  html += "<h1>⚙️ Configuration</h1>";
  html += "<form method='POST' action='/save'>";
  
  html += "<label>WiFi SSID:</label><input type='text' name='wifiSSID' value='" + wifiSSID + "' required>";
  html += "<label>WiFi Password:</label><input type='password' name='wifiPassword' value='" + wifiPassword + "'>";
  html += "<label>Temperature Offset (°C):</label><input type='number' name='tempOffset' value='" + String(temperatureOffset, 1) + "' step='0.1'>";
  html += "<label>Humidity Offset (%):</label><input type='number' name='humidityOffset' value='" + String(humidityOffset, 1) + "' step='0.1'>";
  html += "<label>Reading Interval (seconds):</label><input type='number' name='readingInterval' value='" + String(readingInterval) + "' min='5' max='3600'>";
  
  html += "<button type='submit'>💾 Save</button>";
  html += "</form>";
  
  html += "<div style='margin-top:30px;'>";
  html += "<a href='/' style='display:block;background:#6c757d;color:white;padding:15px;text-align:center;text-decoration:none;border-radius:4px;'>← Back</a>";
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

/*
 * Handle save configuration
 */
void handleSave() {
  #ifdef DEBUG_MODE
  Serial.println("Saving configuration...");
  #endif
  
  if (server.hasArg("wifiSSID")) wifiSSID = server.arg("wifiSSID");
  if (server.hasArg("wifiPassword")) wifiPassword = server.arg("wifiPassword");
  if (server.hasArg("tempOffset")) temperatureOffset = server.arg("tempOffset").toFloat();
  if (server.hasArg("humidityOffset")) humidityOffset = server.arg("humidityOffset").toFloat();
  if (server.hasArg("readingInterval")) {
    readingInterval = server.arg("readingInterval").toInt();
    if (readingInterval < 5) readingInterval = 5;
    if (readingInterval > 3600) readingInterval = 3600;
  }
  
  saveConfigToNVS();
  
  // Try to reconnect to WiFi
  if (wifiSSID.length() > 0) {
    WiFi.disconnect();
    delay(500);
    connectToWiFi();
  }
  
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Saved</title>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:20px auto;padding:20px;text-align:center;}</style></head><body>";
  html += "<h1>✓ Configuration Saved</h1>";
  html += "<p>Redirecting to home page...</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

/*
 * Handle sensor data API request
 */
void handleSensorData() {
  String json = "{";
  json += "\"temperature\":" + String(currentTemperature, 1) + ",";
  json += "\"humidity\":" + String(currentHumidity, 1) + ",";
  json += "\"temperature_offset\":" + String(temperatureOffset, 1) + ",";
  json += "\"humidity_offset\":" + String(humidityOffset, 1) + ",";
  json += "\"reading_interval\":" + String(readingInterval) + ",";
  json += "\"timestamp\":" + String(millis() / 1000);
  json += "}";
  
  server.send(200, "application/json", json);
}

/*
 * Setup function
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  
  #ifdef DEBUG_MODE
  Serial.println("\nESP32-C3 Climate Sensor - Simple Version");
  #endif
  
  // Initialize DHT sensor
  dht.begin();
  
  // Load configuration from NVS
  loadConfigFromNVS();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Start web server
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/api/sensor", handleSensorData);
  server.begin();
  
  #ifdef DEBUG_MODE
  Serial.println("Web server started");
  #endif
}

/*
 * Main loop
 */
void loop() {
  server.handleClient();
  readSensorData();
  delay(100);
}
