#include <BluetoothSerial.h> 
BluetoothSerial SerialBT;

void bluetoothOn(String name){
    Serial.print(F("Turn on bluetooth with name: "));
    Serial.println(name);
    SerialBT.begin(name);
}

void bluetoothOff(){
    Serial.println(F("Turn off bluetooth."));
    SerialBT.end();
}

bool isBluetoothOn(){
    return SerialBT.available();
}

String serialBTLinstener(){
    if(SerialBT.available()){
        return SerialBT.readString();
    }
    return "";
}
void serialBTSender(String message){
    SerialBT.println(message);
}