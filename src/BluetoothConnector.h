#include <BluetoothSerial.h> 
BluetoothSerial SerialBT;

String* bluetoothNamePtr;

void setBluetoothConfig(String* bluetoothNamePtr){
    ::bluetoothNamePtr = bluetoothNamePtr;
}

void bluetoothOn(){
    Serial.print(F("Turn on bluetooth with name: "));
    Serial.println(*bluetoothNamePtr);
    SerialBT.begin(*bluetoothNamePtr);
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