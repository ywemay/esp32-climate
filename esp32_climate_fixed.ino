/*
  ESP32-C3 Climate Sensor with Web Interface
  Starts in AP mode for WiFi configuration
  Features WiFi network scanning for easy setup
  Version 1.2 - Fixed WiFi connection issues
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

// WiFi reconnection state
unsigned long lastConnectionAttempt = 0;
int connectionRetryCount = 0;
const unsigned long MIN_RECONNECT_INTERVAL = 30000; // 30 seconds between reconnection attempts
const int MAX_CONNECTION_RETRIES = 5; // After 5 failures, fall back to AP mode

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
  
  // Check if we should attempt reconnection
  unsigned long now = millis();
  if (connectionRetryCount > 0 && (now - lastConnectionAttempt) < MIN_RECONNECT_INTERVAL) {
    return; // Too soon to retry
  }
  
  // After too many failures, fall back to AP mode
  if (connectionRetryCount >= MAX_CONNECTION_RETRIES) {
    Serial.println("Too many connection failures, falling back to AP mode");
    if (!apMode) {
      startAPMode();
    }
    return;
  }
  
  connectionRetryCount++;
  lastConnectionAttempt = now;
  
  Serial.print("Connecting to WiFi (attempt ");
  Serial.print(connectionRetryCount);
  Serial.print("/");
  Serial.print(MAX_CONNECTION_RETRIES);
  Serial.print("): ");
  Serial.println(wifiSSID);
  
  // Disconnect first to ensure clean state
  WiFi.disconnect(true);
  delay(100);
  
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  // Try for up to 30 seconds (60 * 500ms)
  for (int i = 0; i < 60; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      connectionRetryCount = 0; // Reset on success
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(500);
    if (i % 10 == 0) Serial.print(".");
  }
  
  wifiConnected = false;
  Serial.println("\nFailed to connect");
  
  // If this was the first attempt and it failed immediately, 
  // check if credentials might be wrong or network unavailable
  if (connectionRetryCount == 1) {
    Serial.println("First connection attempt failed. Possible issues:");
    Serial.println("1. WiFi credentials incorrect");
    Serial.println("2. Network not in range");
    Serial.println("3. Router issues");
  }
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
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:Arial;max-width:600px;margin:20px auto;padding:20px;}";
  html += "input,select{width:100%;padding:8px;margin:5px 0 15px 0;border:1px solid #ddd;border-radius:4px;box-sizing:border-box;}";
  html += "button{background:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;width:100%;font-size:16px;}";
  html += "button:hover{background:#45a049;} button:disabled{background:#ccc;}";
  html += ".section{margin-bottom:20px;padding:15px;border:1px solid #ddd;border-radius:4px;}";
  html += ".section h2{margin-top:0;color:#555;}";
  html += ".signal-bars{display:inline-block;width:60px;vertical-align:middle;margin-left:10px;}";
  html += ".bar{height:4px;margin:1px;background:#ddd;border-radius:1px;display:inline-block;}";
  html += ".bar.filled{background:#4CAF50;}";
  html += "#scanStatus{margin:5px 0;font-size:14px;}";
  html += ".scanning{color:#2196F3;} .success{color:#4CAF50;} .error{color:#dc3545;}";
  html += "</style></head><body>";
  
  html += "<h1>⚙️ Configuration</h1>";
  html += "<form method='POST' action='/save'>";
  
  // WiFi Section with Scanning
  html += "<div class='section'><h2>📶 WiFi Configuration</h2>";
  html += "<label>WiFi SSID:</label>";
  html += "<input type='text' id='wifiSSID' name='ssid' value='" + wifiSSID + "' required>";
  
  html += "<label>Or select from scanned networks:</label>";
  html += "<select id='networkSelect' onchange='selectNetwork()'><option value=''>-- Scan for networks --</option></select>";
  
  html += "<button type='button' id='scanBtn' onclick='scanNetworks()' style='margin:10px 0;background:#2196F3;'>📡 Scan Networks</button>";
  html += "<div id='scanStatus'></div>";
  html += "<small>Manual entry still works for hidden networks</small>";
  
  html += "<label>WiFi Password:</label>";
  html += "<input type='password' name='password' value='" + wifiPassword + "'>";
  html += "</div>";
  
  // Sensor Calibration
  html += "<div class='section'><h2>⚖️ Sensor Calibration</h2>";
  html += "<label>Temperature Offset (°C):</label>";
  html += "<input type='number' name='tempOffset' value='" + String(temperatureOffset, 1) + "' step='0.1'>";
  html += "<label>Humidity Offset (%):</label>";
  html += "<input type='number' name='humidityOffset' value='" + String(humidityOffset, 1) + "' step='0.1'>";
  html += "</div>";
  
  // Sensor Settings
  html += "<div class='section'><h2>⚙️ Sensor Settings</h2>";
  html += "<label>Reading Interval (seconds):</label>";
  html += "<input type='number' name='readingInterval' value='" + String(readingInterval) + "' min='5' max='3600'>";
  html += "</div>";
  
  html += "<button type='submit'>💾 Save Configuration</button>";
  html += "</form>";
  
  html += "<hr><a href='/' style='display:block;text-align:center;color:#2196F3;'>← Back to Home</a>";
  
  // Add JavaScript for network scanning
  String js = "<script>";
  js += "function scanNetworks(){var btn=document.getElementById('scanBtn');var status=document.getElementById('scanStatus');var select=document.getElementById('networkSelect');btn.disabled=true;btn.innerHTML='⏳ Scanning...';status.className='scanning';status.innerHTML='Scanning...';select.innerHTML='<option>Loading...</option>';";
  js += "fetch('/scan-networks').then(function(r){return r.json();}).then(function(data){btn.disabled=false;btn.innerHTML='📡 Scan Networks';if(data.networks&&data.networks.length>0){select.innerHTML='<option value=\"\">-- Select --</option>';for(var i=0;i<data.networks.length;i++){var n=data.networks[i];var ssid=n.ssid||'(Hidden)';var bars=getSignalBars(n.rssi);var opt=document.createElement('option');opt.value=ssid;opt.innerHTML=ssid+' '+bars;select.appendChild(opt);}status.className='success';status.innerHTML='Found '+data.networks.length+' networks';}else{select.innerHTML='<option>No networks</option>';status.className='error';status.innerHTML='No networks found';}}).catch(function(e){btn.disabled=false;btn.innerHTML='📡 Scan';status.className='error';status.innerHTML='Scan failed';select.innerHTML='<option>Error</option>';});}";
  js += "function getSignalBars(rssi){var bars='';var filled=rssi>=-50?4:(rssi>=-60?3:(rssi>=-70?2:(rssi>=-80?1:0)));for(var i=0;i<4;i++){var cls=i<filled?'filled':'';bars+='<div class=\"bar '+cls+'\"></div>';}return '<div class=\"signal-bars\">'+bars+'</div>';}";
  js += "function selectNetwork(){var select=document.getElementById('networkSelect');var ssidInput=document.getElementById('wifiSSID');if(select.value){ssidInput.value=select.value;}}";
  js += "</script>";
  
  html += js;
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

void handleScanNetworks() {
  #ifdef DEBUG_MODE
  Serial.println("Scanning for WiFi networks...");
  #endif
  
  // Start scan
  int n = WiFi.scanNetworks();
  
  String jsonResponse = "{\"networks\":[";
  
  if (n == 0) {
    #ifdef DEBUG_MODE
    Serial.println("No networks found");
    #endif
    jsonResponse += "]}";
  } else {
    #ifdef DEBUG_MODE
    Serial.print(n);
    Serial.println(" networks found");
    #endif
    
    // Create array to sort by RSSI
    int indices[n];
    for (int i = 0; i < n; i++) {
      indices[i] = i;
    }
    
    // Sort by RSSI (strongest first)
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          int temp = indices[i];
          indices[i] = indices[j];
          indices[j] = temp;
        }
      }
    }
    
    // Build JSON response
    bool first = true;
    for (int i = 0; i < n; i++) {
      int idx = indices[i];
      if (!first) {
        jsonResponse += ",";
      }
      first = false;
      
      String ssid = WiFi.SSID(idx);
      int rssi = WiFi.RSSI(idx);
      
      // Escape special characters in SSID for JSON
      ssid.replace("\\", "\\\\");
      ssid.replace("\"", "\\\"");
      
      jsonResponse += "{\"ssid\":\"" + ssid + "\",\"rssi\":" + String(rssi) + "}";
      
      #ifdef DEBUG_MODE
      Serial.print("  - ");
      Serial.print(ssid);
      Serial.print(" (RSSI: ");
      Serial.print(rssi);
      Serial.println(" dBm)");
      #endif
    }
    
    jsonResponse += "]}";
    
    // Delete scan results to free memory
    WiFi.scanDelete();
  }
  
  server.send(200, "application/json", jsonResponse);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n