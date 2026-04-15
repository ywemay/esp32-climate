// Configuration file for ESP32-C3 Climate Sensor
// All sensitive credentials are stored in NVS (Preferences) at runtime
// This file contains only non-sensitive defaults and constants

#ifndef CONFIG_CLIMATE_H
#define CONFIG_CLIMATE_H

// GPIO Configuration
#define CONFIG_BUTTON_PIN 2  // GPIO pin for AP mode trigger (connect to 3.3V for AP mode)
#define DHT_PIN 5            // GPIO pin for DHT22 sensor data
#define DHT_TYPE DHT11       // DHT sensor type (DHT11, DHT22, DHT21)

// Default Values (used when NVS is empty)
#define DEFAULT_READING_INTERVAL 30  // Default sensor reading interval in seconds (5-3600)
#define DEFAULT_TEMP_OFFSET 0.0      // Default temperature calibration offset
#define DEFAULT_HUMIDITY_OFFSET 0.0  // Default humidity calibration offset

// Debug Settings
#define SERIAL_BAUD_RATE 115200
#define DEBUG_MODE true  // Set to false to reduce serial output

// NVS Namespace for Preferences storage
#define NVS_NAMESPACE "climate_config"

// Web Server Configuration
#define WEB_SERVER_PORT 80
#define CONFIG_PASSWORD_MIN_LENGTH 4  // Minimum length for config password

// AP Mode Configuration
#define AP_SSID_PREFIX "ESP32-CLIMATE-"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4

// DNS Server for Captive Portal
#define DNS_PORT 53

// HTML Page Buffer Size
#define HTML_BUFFER_SIZE 4096

// Sensor reading limits
#define MIN_READING_INTERVAL 5      // Minimum 5 seconds between readings
#define MAX_READING_INTERVAL 3600   // Maximum 1 hour between readings
#define MIN_TEMP_OFFSET -5.0        // Minimum temperature offset
#define MAX_TEMP_OFFSET 5.0         // Maximum temperature offset
#define MIN_HUMIDITY_OFFSET -10.0   // Minimum humidity offset
#define MAX_HUMIDITY_OFFSET 10.0    // Maximum humidity offset

#endif
