#include "WiFiConnector.h"
#include "JsonDecoder.h"
#include <AsyncMqttClient.h>
#define INIT_ACCOUNT_TOPIC "newAccount"

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;

String* mqttHostPtr;
String* mqttPortPtr;
String* mqttUsernamePtr;
String* mqttPasswordPtr;
String* mqttTopic;
bool isInit = false;
bool mqttReady = false;

void setMqttConfig(String* mqttHostPtr,String* mqttPortPtr,String* mqttUsernamePtr,String* mqttPasswordPtr,String* mqttTopic){
  ::mqttHostPtr = mqttHostPtr;
  ::mqttPortPtr = mqttPortPtr;
  ::mqttUsernamePtr = mqttUsernamePtr;
  ::mqttPasswordPtr = mqttPasswordPtr;
  ::mqttTopic = mqttTopic;
}

String TOPIC_INFO;
String TOPIC_STATE;
String TOPIC_CONTROL;
String TOPIC_HEARTBEAT;

void setMqttTopic(){
  TOPIC_INFO = "devices/"+*mqttTopic+"/info";
  TOPIC_STATE = "devices/"+*mqttTopic+"/state";
  TOPIC_CONTROL = "devices/"+*mqttTopic+"/control";
  TOPIC_HEARTBEAT = "devices/"+*mqttTopic+"/heartbeat";
}

void subscribeTopic(){
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(mqttClient.subscribe(TOPIC_CONTROL.c_str(), 2));
}

void mqttInitializationDataSetter(String json){
  String data[] = {"username","password"};
  jsonDecode(json,data,2);
  *mqttUsernamePtr = data[0];
  *mqttPasswordPtr = data[1];
  Serial.println(data[0]);
  Serial.println(data[1]);
  setMqttTopic();
  isInit = false;
  mqttClient.setCredentials(mqttUsernamePtr->c_str(), mqttPasswordPtr->c_str());
  mqttClient.unsubscribe(INIT_ACCOUNT_TOPIC);
  mqttClient.disconnect();
}

void connectToMqtt() {
  if(!WiFi.isConnected()){
    Serial.println("WiFi connect fail, skip connect mqtt step.");
    return;
  }
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
  
}


void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT connected.");
  serialBTSender("MQTT connected.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  xTimerStop(mqttReconnectTimer, 0);
  if(isInit){
    Serial.println(mqttClient.subscribe(INIT_ACCOUNT_TOPIC, 2));
    return;
  }
  subscribeTopic();
  mqttReady = true;

}



void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  mqttReady = false;
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
  if(String(topic) == INIT_ACCOUNT_TOPIC){
    Serial.print("Start set data.");
    mqttInitializationDataSetter(messageTemp);
  }else if(String(topic) == TOPIC_CONTROL){
    DynamicJsonDocument jsonDoc(200);
    deserializeJson(jsonDoc, messageTemp);  
    int switchValue = jsonDoc["switch"];
    if(switchValue!=0&&switchValue!=1) return;
    digitalWrite(26, switchValue);
    DynamicJsonDocument jsonDoc2(200);
    String state;
    jsonDoc2["lightState"] = switchValue;
    serializeJson(jsonDoc2, state);
    publishMessage(TOPIC_STATE,state);
  }
}



void publishInfo(String message){
  publishMessage(TOPIC_INFO,message);
}

void mqttStart(){
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

void mqttInitialization(){
  isInit = true;
  Serial.println("Init MQTT...");
  mqttClient.setCredentials("", "");
  mqttStart();
}



