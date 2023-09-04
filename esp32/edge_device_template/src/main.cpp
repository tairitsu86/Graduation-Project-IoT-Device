#include "WiFiConnector.h"
#include "BluetoothConnector.h"
#include "DataStorage.h"

using namespace std;

//EEPROM data store
#define DATA_SIZE 9
String data[DATA_SIZE];


//Device info
String& DEVICE_ID = data[0];
String& DEVICE_NAME = data[1];
String& OWNER = data[2];


//WIFI connect
String& WIFI_SSID = data[3];
String& WIFI_PASSWORD = data[4];


//Bluetooth connect
void bluetoothListener(){
  if(isBluetoothOn()){
    String message = serialBTLinstener();
    if(message!=""){
      Serial.println(message);
      String prefix = message.substring(0, 13);
      String subfix = message.substring(14);
      String result = "UNKNOW";
      if(      prefix == "DEVICE_NAME__"){
        DEVICE_NAME = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "WIFI_SSID____"){
        WIFI_SSID= subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "WIFI_PASSWORD"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "MQTT_HOST____"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "MQTT_PORT____"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "CONNECT_START"){
        wifiConnect(WIFI_SSID,WIFI_PASSWORD)
        result = "Try to connect WiFi.";
      }
      Serial.println(result);
      serialBTSender(result);
    }
  }
}
//MQTT connect
String& MQTT_HOST = data[5];
String& MQTT_PORT = data[6];
String& MQTT_USERNAME = data[0];
String& MQTT_PASSWORD = data[8];
String& MQTT_TOPIC = data[0];







void setup() {
  Serial.begin(115200);
  loadData(data,DATA_SIZE);
}

void loop() {
  bluetoothListener();
}