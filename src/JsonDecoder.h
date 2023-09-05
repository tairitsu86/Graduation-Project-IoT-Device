#include <ArduinoJson.h>

void jsonDecode(String json,String data[],int dataSize){
    DynamicJsonDocument jsonDoc(200);
    DeserializationError error = deserializeJson(jsonDoc, json);  
    // Check for parsing errors
    if (error) {
        Serial.println("Error parsing JSON");
        return;
    }

    for(int i=0;i<dataSize;i++){
        const char* jsonValue = jsonDoc[data[i].c_str()];
        data[i] = String(jsonValue);  
    }
}
