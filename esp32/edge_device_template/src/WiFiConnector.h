#include "WiFi.h"

void wifiConnect(String WIFI_SSID,String WIFI_PASSWORD){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Try to connect to "));
  Serial.print(WIFI_SSID);
}


