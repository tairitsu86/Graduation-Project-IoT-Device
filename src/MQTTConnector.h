extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include "JsonDecoder.h"
#include <AsyncMqttClient.h>

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

String* mqttHostPtr;
String* mqttPortPtr;
String* deviceId;

String TOPIC_INFO = "devices/info";
String TOPIC_STATE = "devices/state";
String TOPIC_CONTROL;

void publishMessage(String topic,String message);





void setMqttConfig(String* mqttHostPtr, String* mqttPortPtr, String* deviceId){
  ::mqttHostPtr = mqttHostPtr;
  ::mqttPortPtr = mqttPortPtr;
  ::deviceId = deviceId;
}

void publishInfo(){
  String info;
  DynamicJsonDocument jsonDoc(1024);

  jsonDoc["deviceId"] = *deviceId;
  jsonDoc["deviceName"] = "test device";
  jsonDoc["description"] = "This is a test device, made by ESP32!";
  jsonDoc["owner"] = "OAO";

  JsonArray statesArray = jsonDoc.createNestedArray("states");
  JsonObject state0 = statesArray.createNestedObject();
  state0["stateId"] = 0;
  state0["stateType"] = "PASSIVE";
  state0["dataType"] = "OPTIONS";
  state0["stateName"] = "light power";
  JsonArray state0Options = state0.createNestedArray("stateOptions");
  state0Options.add("on");
  state0Options.add("off");

  JsonObject state1 = statesArray.createNestedObject();
  state1["stateId"] = 1;
  state1["stateType"] = "ACTIVE";
  state1["dataType"] = "ANY_FLOAT";
  state1["stateName"] = "temperature";
  state1["valueUnit"] = "°C";

  JsonArray functionsArray = jsonDoc.createNestedArray("functions");

  JsonObject function0 = functionsArray.createNestedObject();
  function0["functionId"] = 0;
  function0["functionName"] = "light switch";
  function0["dataType"] = "OPTIONS";
  JsonArray function0Options = function0.createNestedArray("functionOptions");
  function0Options.add("on");
  function0Options.add("off");

  serializeJson(jsonDoc, info);
  publishMessage(TOPIC_INFO, info);
}



void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT connected.");
  serialBTSender("MQTT connected.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  xTimerStop(mqttReconnectTimer, 0);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(mqttClient.subscribe(TOPIC_CONTROL.c_str(), 2));
  publishInfo();
}



void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  xTimerStart(mqttReconnectTimer, 0);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void publishMessage(String topic,String message){
  Serial.println("Publishing data");
  uint16_t packetId = mqttClient.publish(topic.c_str(), 1, true, message.c_str());
  Serial.print("Publish done, packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("\n Publish received.");
  Serial.print("topic: ");
  Serial.println(topic);
  String messageTemp;
  for (int i = 0; i < len; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.print("Message: ");
  Serial.println(messageTemp);
  if(String(topic) != TOPIC_CONTROL) return; 

  DynamicJsonDocument jsonDoc(200);
  //TODO
  // deserializeJson(jsonDoc, messageTemp);  
  // int switchValue = jsonDoc["switch"];
  // if(switchValue!=0&&switchValue!=1) return;
  // digitalWrite(26, switchValue);
  // DynamicJsonDocument jsonDoc2(200);
  // String state;
  // jsonDoc2["lightState"] = switchValue;
  // serializeJson(jsonDoc2, state);
  // publishMessage(TOPIC_STATE,state);
}

void mqttStart(){
  TOPIC_CONTROL = "devices/"+*deviceId+"/control";
  mqttClient.setServer(mqttHostPtr->c_str(), mqttPortPtr->toInt());
  Serial.println("MQTT Host: "+*mqttHostPtr+":"+*mqttPortPtr);
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  xTimerStart(mqttReconnectTimer, 0);
}


