#include "MQTTConnector.h"
#include "DataStorage.h"


using namespace std;

//EEPROM data store
#define DATA_SIZE 8
String data[DATA_SIZE];


//Device info
String& DEVICE_ID = data[0];
String& DEVICE_NAME = data[1];
String& OWNER = data[2];


//WIFI connect
String& WIFI_SSID = data[3];
String& WIFI_PASSWORD = data[4];


//MQTT connect
String& MQTT_HOST = data[5];
String& MQTT_PORT = data[6];
String& MQTT_USERNAME = data[0];
String& MQTT_PASSWORD = data[7];
String& MQTT_TOPIC = data[0];

void uploadInfo(){
  String info;
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["deviceId"] = DEVICE_ID;
  jsonDoc["deviceName"] = DEVICE_NAME;
  jsonDoc["owner"] = OWNER;
    // Create an array for the "functions" key
  JsonArray functionsArray = jsonDoc.createNestedArray("functions");

  // Create and add the first object in the "functions" array
  JsonObject function1 = functionsArray.createNestedObject();
  function1["functionId"] = 0;
  function1["functionName"] = "light state";
  function1["functionType"] = "STATE";

  // Create and add the "parameters" array for the first function
  JsonArray parametersArray1 = function1.createNestedArray("parameters");
  JsonObject parameter1_1 = parametersArray1.createNestedObject();
  parameter1_1["parameterName"] = "lightState";

  // Create and add the second object in the "functions" array
  JsonObject function2 = functionsArray.createNestedObject();
  function2["functionId"] = 1;
  function2["functionName"] = "light switch";
  function2["functionType"] = "CONTROL";

  // Create and add the "parameters" array for the second function
  JsonArray parametersArray2 = function2.createNestedArray("parameters");
  JsonObject parameter2_1 = parametersArray2.createNestedObject();
  parameter2_1["parameterName"] = "switch";
  parameter2_1["parameterRange"] = "[0,1]";
  serializeJson(jsonDoc, info);
  publishInfo(info);
}


//Bluetooth connect
void bluetoothListener(){
  if(isBluetoothOn()){
    String message = serialBTLinstener();
    if(message!=""){
      Serial.println(message);
      String prefix = message.substring(0, 13);
      String subfix = message.substring(14);
      String result = "UNKNOWN";
      if(      prefix == "DEVICE_NAME__"){
        DEVICE_NAME = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "OWNER________"){
        OWNER = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "WIFI_SSID____"){
        WIFI_SSID = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "WIFI_PASSWORD"){
        WIFI_PASSWORD = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "MQTT_HOST____"){
        MQTT_HOST = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "MQTT_PORT____"){
        MQTT_PORT = subfix;
        result = subfix;
        saveData(data,DATA_SIZE);
      }else if(prefix == "CONNECT_WIFI_"){
        wifiStart();
        result = "Try to connect to wifi.";
      }else if(prefix == "CONNECT_MQTT_"){
        mqttStart();
        result = "Try to connect to mqtt.";
      }else if(prefix == "MQTT_INIT____"){
        mqttInitialization();
        result = "Try to init mqtt setting.";
      }else if(prefix == "COMMIT_INIT__"){
        saveData(data,DATA_SIZE);
        result = "Save data.";
      }else if(prefix == "UPLOAD_INFO__"){
        uploadInfo();
        result = "Try to upload info.";
      }else if(prefix == "BLUETOOTH_OFF"){
        bluetoothOff();
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
  setBluetoothConfig(&DEVICE_NAME);
  setWifiConfig(&WIFI_SSID, &WIFI_PASSWORD);
  setMqttConfig(&MQTT_HOST, &MQTT_PORT, &MQTT_USERNAME, &MQTT_PASSWORD, &MQTT_TOPIC);
  bluetoothOn();
  for(int i=0;i<DATA_SIZE;i++){
    Serial.println(String(i)+" "+data[i]);
  }
  if(DEVICE_ID != "UNKNOWN"){
    wifiStart();
    mqttStart();
  }
}

void loop() {
  bluetoothListener();
}