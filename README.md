# NodeMCU_base_template
A firmware template for the NodeMCU (ESP8266) microcontroller

**Includes code for:**
- Wi-Fi connection
- MQTT connection
- OTA Updates
- Generic sensor read and publish (in JSON format)

**Output:**
- Sensor values are output as JSON, populated in the read_sensor() function
- The base JSON includes the device host name, device type, IP address, BSSID and RSSI (in dB and %)
