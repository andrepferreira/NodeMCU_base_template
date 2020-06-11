/*****************************************************************************
 * Template to be used as base of further programming. You need to:
 * 
 *  - Copy this file to your own folder and rename to match
 *  - Set the correct Wifi, MQTT etc and enable them
 *  - Populate the read_sensor() function
 * 
 * Includes an (optional) wifi connection and the ability to OTA new firmware
 * 
 * Optional MQTT server via #define
 * 
 * Author: Andre Ferreira
 * Date: 11 June 2020
 * Built for the ESP8266 based devices
 * 
 *****************************************************************************/


/************************* Include Libraries *********************************/

#include <ESP8266WiFi.h>              // Include libs for Wifi, OTA and mqtt
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>




/************************ Define device and mqtt topic and things ********/

String NodeID = "device_name_goes_here";    // Define the device name (used in mqtt pub)
String DeviceType = "device_type_goes_here";  // Define the device type for future reference

//#define WIFI              // Define this if you want to use Wi-Fi
//#define MQTT              // Define this if you want to use MQTT (Needs WIFI above)

/************************* Define objects ********************************/

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


/************************* WiFi and MQTT variables ***************************/

const char* ssid = "kenmar";
const char* password = "C0urtney1";
const char* mqtt_server = "172.16.20.10";

String pubTopic="home/"+NodeID+"/values";       // Default data topic for sensor values
String statusTopic="home/deviceStatus";         // Default device status messages topic

/************************* Declare global vars *********************************/

int period = 2000;                   // Time to sleep for between readings (ms)
unsigned long time_now = 0;
char payload [512];                  // json output for the mqtt pub

float rssi;                           // Store Wi-Fi RSSI
float rssi_percent;                   // Store RSSI as a percentage
String bssid;                         // Store BSSID
String ip_addr;                       // Store IP

/************************* Function to get Wi-Fi stats *************************/

void get_wifi_stats() {

  //Get RSSI
  rssi = WiFi.RSSI();

  // Convert RSSI to percent
  rssi_percent = isnan(rssi) ? -100.0 : rssi;
  rssi_percent = min(max(2 * (rssi + 100.0), 0.0), 100.0);

  // Get Base Station MAC
  bssid = WiFi.BSSIDstr();

  // Get IP addr
  ip_addr = WiFi.localIP().toString();

  
  Serial.print("IP address: ");
  Serial.println(ip_addr);

  Serial.print("RSSI: ");
  Serial.println(rssi);
  Serial.print("RSSI %: ");
  Serial.println(rssi_percent);
  Serial.print("BSSID: ");
  Serial.println(bssid);

}

/************************* Function to connect to MQTT *************************/

void mqtt_reconnect() {
  char statPayload[512];
  
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = NodeID;
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
         snprintf(statPayload, sizeof(statPayload), "{\"%s\":{\"DeviceType\":\"%s\", \"IPAddr\":\"%s\", \"BSSID\":\"%s\", \"RSSI\":\"%.1f\", \"RSSIP\":\"%.1f\" \"status\":{\"%s\":\"%s\"}}}", NodeID.c_str(), DeviceType.c_str(), ip_addr.c_str(), bssid.c_str(), rssi, rssi_percent, "MQTT", "Connected" ); 
         mqttClient.publish(statusTopic.c_str(), statPayload);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/************************* Function for MQTT callback *************************/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}


/************************* Function to read the CT *************************/
void read_sensor()
{
  float value1;         // Placeholders for the mqtt code below..
  float value2;

  // Do some clever stuff here

#ifdef MQTT
  snprintf(payload, sizeof(payload), "{\"%s\": {\"DeviceType\":\"%s\", \"IPAddr\":\"%s\", \"BSSID\":\"%s\", \"RSSI\":\"%.1f\", \"RSSIP\":\"%.1f\", \"status\":{\"%s\":\"%.2f\"}}}", NodeID.c_str(), DeviceType.c_str(), ip_addr.c_str(), bssid.c_str(), rssi, rssi_percent, "temp", celcius);
  Serial.println (payload);
  Serial.println (pubTopic);
  mqttClient.publish(pubTopic.c_str(), payload);
#endif
}

/************************* Setup  *************************/

void setup()
{
  Serial.begin(115200);      //Initializing the Serial Monitor
  delay(100);                // Wait 100 msec for things to settle

#ifdef WIFI
  // Setup the Wifi connection
  WiFi.mode(WIFI_STA);    // Ensure we are in Station mode
  WiFi.hostname(NodeID);  // Set the device network name
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  get_wifi_stats();              // Get Wi-Fi stats and store values
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(ip_addr);
#endif

#ifdef MQTT
  // Setup MQTT server
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
#endif

  // OTA code ---------------------------------

  ArduinoOTA.setHostname(NodeID.c_str());
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // End OTA code ----------------------------------


  // Initialise any sensors here
  
}



void loop() {
  // Setup for OTA updates
  ArduinoOTA.handle();

#ifdef WIFI
 // Check for wifi disconnect and reconnect if broken
 if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);    // Ensure we are in Station mode
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
#endif


#ifdef MQTT
  // Check if MQTT connected and fix if broken
  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();
#endif

  // Loop for rest of time
  if(millis() - time_now > period){   // Workaround for overflow of timer
        time_now += period;
        get_wifi_stats();              // Get Wi-Fi stats and store values
        read_sensor();                 // Read the sensor values
    }
}
