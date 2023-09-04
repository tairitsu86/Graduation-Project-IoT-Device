extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#define MQTT_HOST IPAddress(140, 136, 149, 165)
#define MQTT_PORT 1883

//MQTT Topics
#define MQTT_Control "esp32/Control"
#define MQTT_Data "esp32/Data"


