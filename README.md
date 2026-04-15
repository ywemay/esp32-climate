# ESP32-C3 Climate Sensor (Simple Version)

A temperature and humidity monitoring system based on ESP32-C3 with web interface and REST API.

## Features

- **Temperature & Humidity Monitoring**: Uses DHT22 sensor for accurate readings
- **Web Interface**: Built-in web server for configuration and monitoring
- **REST API**: JSON endpoints for programmatic access to sensor data
- **Secure Storage**: NVS-based credential storage (no hardcoded passwords)
- **Calibration**: Software offsets for temperature and humidity calibration
- **Configurable Intervals**: Adjustable sensor reading frequency

**Note**: This is a simplified version without Access Point mode. Configure WiFi credentials via serial monitor or edit NVS directly.

## Hardware Requirements

- ESP32-C3 development board
- DHT22 temperature/humidity sensor
- Breadboard and jumper wires
- USB cable for programming and power

## Wiring

| DHT22 Pin | ESP32-C3 Pin | Description |
|-----------|--------------|-------------|
| VCC       | 3.3V         | Power (3.3V) |
| GND       | GND          | Ground      |
| DATA      | GPIO5        | Data pin    |

**Note**: The DHT22 DATA pin requires a 4.7kΩ pull-up resistor to 3.3V.

## Software Requirements

- Arduino IDE with ESP32 board support
- Required libraries:
  - `WiFi`
  - `WebServer`
  - `DNSServer`
  - `HTTPClient`
  - `ArduinoJson`
  - `Preferences`
  - `DHT sensor library`

## Installation

1. Install the required libraries via Arduino Library Manager:
   - ArduinoJson by Benoit Blanchon
   - DHT sensor library by Adafruit

2. Configure Arduino IDE for ESP32-C3:
   - Board: "ESP32C3 Dev Module"
   - Upload Speed: "921600"
   - Flash Mode: "DIO"
   - Flash Frequency: "80MHz"
   - Partition Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"

3. Upload the sketch to your ESP32-C3

## Initial Setup

1. After uploading, the ESP32 will start in Access Point mode
2. Connect to the WiFi network: `ESP32-CLIMATE-XXXX` (where XXXX is the chip ID)
3. Open a browser and navigate to `http://192.168.4.1`
4. Configure your WiFi credentials and other settings
5. Save the configuration

## Web Interface

### Configuration Page (`/config`)
- Set WiFi credentials
- Configure sensor calibration offsets
- Set reading interval
- Set configuration password

### Status Page (`/status`)
- Real-time temperature and humidity display
- Network information
- Device status
- Quick access to configuration

### API Documentation (`/api`)
- REST API endpoint documentation
- Usage examples for various programming languages

## REST API

### Get Current Sensor Data
```
GET /api/sensor
```

**Response (JSON):**
```json
{
  "status": "ok",
  "temperature": 23.5,
  "humidity": 45.2,
  "temperature_offset": 0.0,
  "humidity_offset": 0.0,
  "reading_interval": 30,
  "timestamp": 1703275200,
  "sensor_error": false
}
```

### Get Sensor History (Placeholder)
```
GET /api/sensor/history
```

**Note**: This endpoint is a placeholder for future expansion.

## Configuration Options

### WiFi Settings
- **SSID**: Your WiFi network name
- **Password**: Your WiFi password

### Sensor Calibration
- **Temperature Offset**: Adjust temperature readings (±5.0°C)
- **Humidity Offset**: Adjust humidity readings (±10.0%)

### Sensor Settings
- **Reading Interval**: How often to read the sensor (5-3600 seconds)

### Security
- **Configuration Password**: Password required for LAN-based configuration changes

## Usage Examples

### Python
```python
import requests
import json

response = requests.get('http://DEVICE_IP/api/sensor')
data = response.json()

print(f"Temperature: {data['temperature']}°C")
print(f"Humidity: {data['humidity']}%")
```

### cURL
```bash
curl http://DEVICE_IP/api/sensor
```

### JavaScript
```javascript
fetch('http://DEVICE_IP/api/sensor')
  .then(response => response.json())
  .then(data => {
    console.log(`Temperature: ${data.temperature}°C`);
    console.log(`Humidity: ${data.humidity}%`);
  });
```

## Emergency Access

### Factory Reset
1. Pull GPIO2 HIGH (connect to 3.3V)
2. Power cycle the device
3. Device will start in AP mode
4. Access the web interface to reconfigure

### Manual Reset via Web Interface
1. Access `/reset` page
2. Confirm factory reset
3. Device will restart in AP mode

## Troubleshooting

### No WiFi Connection
- Check if WiFi credentials are correctly configured
- Verify the ESP32 is within range of the access point
- Check if the WiFi network is 2.4GHz (ESP32-C3 doesn't support 5GHz)

### Sensor Reading Errors
- Verify DHT22 wiring (VCC, GND, DATA)
- Check if pull-up resistor is properly connected
- Ensure stable power supply (DHT22 is sensitive to voltage fluctuations)

### Web Interface Not Accessible
- Verify the ESP32 is connected to WiFi
- Check the IP address in serial monitor
- Ensure no firewall is blocking port 80

## File Structure

- `esp32_climate.ino` - Main Arduino sketch
- `config_climate.h` - Configuration constants
- `README_climate.md` - This documentation

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Based on the ESP32 DDNS project by the same author
- Uses the excellent DHT sensor library by Adafruit
- Inspired by various IoT climate monitoring projects
