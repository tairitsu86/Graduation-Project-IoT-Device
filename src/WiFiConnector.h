extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include "WiFi.h"

TimerHandle_t wifiReconnectTimer;
String* wifiSsidPtr;
String* wifiPasswordPtr;

void setWifiConfig(String* wifiSsidPtr,String* wifiPasswordPtr){
  ::wifiSsidPtr = wifiSsidPtr;
  ::wifiPasswordPtr = wifiPasswordPtr;
}

void connectToWifi(){
  WiFi.begin(*wifiSsidPtr, *wifiPasswordPtr);
  Serial.print(F("Try to connect to "));
  Serial.print(*wifiSsidPtr);
}

void wifiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %dn", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}


void wifiStart(){
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(wifiEvent);
  connectToWifi();
}


