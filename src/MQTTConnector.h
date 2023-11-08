extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

//LCD setting
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C for 128x32 & 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//LCD end


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
  state1["stateType"] = "PASSIVE";
  state1["dataType"] = "ANY";
  state1["stateName"] = "LCD text";


  JsonObject state2 = statesArray.createNestedObject();
  state2["stateId"] = 2;
  state2["stateType"] = "ACTIVE";
  state2["dataType"] = "ANY_FLOAT";
  state2["stateName"] = "temperature";
  state2["valueUnit"] = "°C";

  JsonObject state3 = statesArray.createNestedObject();
  state3["stateId"] = 3;
  state3["stateType"] = "ACTIVE";
  state3["dataType"] = "ANY_FLOAT";
  state3["stateName"] = "humidity";
  state3["valueUnit"] = "%";

  JsonArray functionsArray = jsonDoc.createNestedArray("functions");

  JsonObject function0 = functionsArray.createNestedObject();
  function0["functionId"] = 0;
  function0["functionName"] = "light switch";
  function0["dataType"] = "OPTIONS";
  JsonArray function0Options = function0.createNestedArray("functionOptions");
  function0Options.add("on");
  function0Options.add("off");

  JsonObject function1 = functionsArray.createNestedObject();
  function1["functionId"] = 1;
  function1["functionName"] = "LCD show text";
  function1["parameterName"] = "any string";
  function1["dataType"] = "ANY";

  serializeJson(jsonDoc, info);
  publishMessage(TOPIC_INFO, info);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("MQTT connected.");
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
  DynamicJsonDocument jsonDoc2(200);
  deserializeJson(jsonDoc, messageTemp);  
  int functionId = jsonDoc["functionId"];
  if(functionId==0){
    int parameter = jsonDoc["parameter"];
    if(parameter == 0)
      digitalWrite(26, 1);
    else if(parameter == 1)
      digitalWrite(26, 0);
    else 
      return;
    jsonDoc2["stateId"] = 0;
  }else if(functionId==1){
    String parameter = jsonDoc["parameter"].as<String>();
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE); 
    display.setCursor(0,0);
    display.print(parameter);
    display.display();
    jsonDoc2["stateId"] = 1;
  }

  
  String state;
  jsonDoc2["deviceId"] = jsonDoc["deviceId"];
  jsonDoc2["stateValue"] = jsonDoc["parameter"];
  jsonDoc2["executor"] = jsonDoc["executor"];
  serializeJson(jsonDoc2, state);
  publishMessage(TOPIC_STATE, state);
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


