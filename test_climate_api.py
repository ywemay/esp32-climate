#!/usr/bin/env python3
"""
Test script for ESP32-C3 Climate Sensor API
Usage: python3 test_climate_api.py <device_ip>
"""

import sys
import requests
import json
from datetime import datetime

def test_api(device_ip):
    """Test the climate sensor API endpoints"""
    
    base_url = f"http://{device_ip}"
    
    print(f"Testing ESP32-C3 Climate Sensor at {base_url}")
    print("=" * 50)
    
    # Test 1: Get sensor data
    print("\n1. Testing /api/sensor endpoint...")
    try:
        response = requests.get(f"{base_url}/api/sensor", timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"   Status: {data.get('status', 'unknown')}")
            if data.get('status') == 'ok':
                print(f"   Temperature: {data.get('temperature', 'N/A')}°C")
                print(f"   Humidity: {data.get('humidity', 'N/A')}%")
                print(f"   Timestamp: {datetime.fromtimestamp(data.get('timestamp', 0))}")
            else:
                print(f"   Error: {data.get('message', 'Unknown error')}")
        else:
            print(f"   HTTP Error: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"   Request failed: {e}")
    
    # Test 2: Get sensor history (placeholder)
    print("\n2. Testing /api/sensor/history endpoint...")
    try:
        response = requests.get(f"{base_url}/api/sensor/history", timeout=5)
        if response.status_code == 200:
            data = response.json()
            print(f"   Status: {data.get('status', 'unknown')}")
            print(f"   Message: {data.get('message', 'No message')}")
        else:
            print(f"   HTTP Error: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"   Request failed: {e}")
    
    # Test 3: Check web interface
    print("\n3. Testing web interface...")
    try:
        response = requests.get(f"{base_url}/status", timeout=5)
        if response.status_code == 200:
            print("   Web interface is accessible")
            # Check if it's the status page by looking for keywords
            if "ESP32 Climate Sensor" in response.text:
                print("   Status page loaded successfully")
        else:
            print(f"   HTTP Error: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"   Request failed: {e}")
    
    print("\n" + "=" * 50)
    print("Test complete!")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 test_climate_api.py <device_ip>")
        print("Example: python3 test_climate_api.py 192.168.1.100")
        sys.exit(1)
    
    device_ip = sys.argv[1]
    test_api(device_ip)

if __name__ == "__main__":
    main()
