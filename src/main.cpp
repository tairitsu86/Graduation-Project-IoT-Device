#include "BluetoothConnector.h"
#include "WiFiConnector.h"
#include "MQTTConnector.h"
#include "DataStorage.h"
#include "RestRequestSender.h"
#include "DHT.h"

using namespace std;
//DHT 22
#define DHTPIN 27
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);


//EEPROM data store
#define DATA_SIZE 5
String data[DATA_SIZE];

//Device info
String& DEVICE_ID = data[0];

//WIFI connect
String& WIFI_SSID = data[1];
String& WIFI_PASSWORD = data[2];


//MQTT connect
String& MQTT_HOST = data[3];
String& MQTT_PORT = data[4];

//Init
bool isInit = false;

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
        saveData(data, DATA_SIZE);
      }else if(prefix == "WIFI_PASSWORD"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data, DATA_SIZE);
      }else if(prefix == "CONNECT_WIFI_"){
        setWifiConfig(&WIFI_SSID, &WIFI_PASSWORD);
        wifiStart();
        result = "Try to connect to wifi.";
      }else if(prefix == "WIFI_STATE___"){
        if(WiFi.status() == WL_CONNECTED){
          result = "Connected";
        }else{
          result = "Unconnected";
        }
      }else if(prefix == "RESET_ALL____"){
        formatData();
        result = "Format finish, please restart the device.";
      }else if(prefix == "SET_INIT_URL_"){
        String initData = getInitData(subfix);
        if(initData=="error"){
          result = "getInitData() error";
        }else{
          isInit = false;
          DynamicJsonDocument jsonDoc(200);
          deserializeJson(jsonDoc, initData);  
          DEVICE_ID = jsonDoc["id"].as<String>();
          MQTT_HOST = jsonDoc["mqttHostIp"].as<String>();
          MQTT_PORT = jsonDoc["mqttHostPort"].as<String>();
          saveData(data, DATA_SIZE);
          result = "Success!";
        }

      }
      Serial.println(result);
      SerialBT.println(result);
    }
  }
}



void setup() {
  Serial.begin(115200);
  pinMode(26, OUTPUT);
  loadData(data, DATA_SIZE);
  setBluetoothConfig(&DEVICE_ID);
  bluetoothOn();
  if(DEVICE_ID == "UNKNOWN"|| DEVICE_ID == "null"){
    isInit = true;
    while(isInit)
      bluetoothListener();
    Serial.println("Init finish");
  }else{
    setWifiConfig(&WIFI_SSID, &WIFI_PASSWORD);
    wifiStart();
  }
  Serial.print("Wait for wifi");
  SerialBT.print("Wait for wifi");
  while(WiFi.status() != WL_CONNECTED){
    bluetoothListener();
    Serial.print(".");
    SerialBT.print(".");
    delay(1000);
  }
  Serial.println("");
  SerialBT.println("");
  setMqttConfig(&MQTT_HOST, &MQTT_PORT, &DEVICE_ID);
  mqttStart();
  dht.begin();
}

int rate;
int dhtRefreshRate = 100;
DynamicJsonDocument temperatureJsonDoc(200);
DynamicJsonDocument humidityJsonDoc(200);

void loop() {
  rate++;
  rate = rate%100000;
  bluetoothListener();
  if(rate%dhtRefreshRate==0){
    String temp;
    temperatureJsonDoc["deviceId"] = DEVICE_ID;
    temperatureJsonDoc["stateId"] = 2;
    temperatureJsonDoc["stateValue"] = dht.readTemperature();
    serializeJson(temperatureJsonDoc, temp);
    publishMessage(TOPIC_STATE, temp);
  }
  if(rate%dhtRefreshRate==dhtRefreshRate/2){
    String temp;
    humidityJsonDoc["deviceId"] = DEVICE_ID;
    humidityJsonDoc["stateId"] = 3;
    humidityJsonDoc["stateValue"] = dht.readHumidity(); 
    serializeJson(humidityJsonDoc, temp);
    publishMessage(TOPIC_STATE, temp);
  }
  delay(100);
}