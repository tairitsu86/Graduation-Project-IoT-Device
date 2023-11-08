#include "BluetoothConnector.h"
#include "WiFiConnector.h"
#include "MQTTConnector.h"
#include "DataStorage.h"


using namespace std;

//EEPROM data store
#define DATA_SIZE 5
String data[DATA_SIZE];

/*
//Device info
String& DEVICE_ID = data[0];

//WIFI connect
String& WIFI_SSID = data[1];
String& WIFI_PASSWORD = data[2];


//MQTT connect
String& MQTT_HOST = data[3];
String& MQTT_PORT = data[4];
*/

//Device info
String DEVICE_ID = "123456";

//WIFI connect
String WIFI_SSID = "CHIU_Home";
String WIFI_PASSWORD = "wan1964**";

//MQTT connect
String MQTT_HOST = "140.136.151.74";
String MQTT_PORT = "1883";


//Bluetooth connect
void bluetoothListener(){
  if(isBluetoothOn()){
    String message = serialBTLinstener();
    if(message!=""){
      Serial.println(message);
      String prefix = message.substring(0, 13);
      String subfix = message.substring(14);
      String result = "UNKNOWN";
      if(prefix == "WIFI_SSID____"){
        WIFI_SSID = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "WIFI_PASSWORD"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "CONNECT_WIFI_"){
        wifiStart();
        result = "Try to connect to wifi.";
      }else if(prefix == "RESET_ALL____"){
        formatData();
        result = "Format finish, please restart the device.";
      }
      Serial.println(result);
      serialBTSender(result);
    }
  }
}



void setup() {
  Serial.begin(115200);
  pinMode(26, OUTPUT);
  loadData(data,DATA_SIZE);
  setBluetoothConfig(&DEVICE_ID);
  setWifiConfig(&WIFI_SSID, &WIFI_PASSWORD);
  setMqttConfig(&MQTT_HOST, &MQTT_PORT, &DEVICE_ID);
  bluetoothOn();
  if(DEVICE_ID != "UNKNOWN"){
    wifiStart();
    mqttStart();
  }
}

void loop() {
  bluetoothListener();
}