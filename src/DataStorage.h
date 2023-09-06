#include <EEPROM.h>
#define EEPROM_SIZE 4096
using namespace std;

void loadData(String data[], int size) {
  EEPROM.begin(EEPROM_SIZE);
  // Read each string from EEPROM
  int address = 0;
  for (int i = 0; i < size; i++) {
    char currentChar = EEPROM.read(address++);
    int j = 0;
    while (currentChar != '\0' && j < EEPROM_SIZE) {
      data[i] += currentChar;
      currentChar = EEPROM.read(address++);
      j++;
    }
    if(j==EEPROM_SIZE||data[0] == ""){
      data[0] = "UNKNOWN";
      data[1] = "NEW_ESP32_DEVICE";
      break;
    }
  }
  
  EEPROM.end();
}
void saveData(const String data[], int size) {
  EEPROM.begin(EEPROM_SIZE);
  // Write each string to EEPROM
  int address = 0;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < data[i].length(); j++) {
      EEPROM.write(address++, data[i][j]);
    }
    // Null-terminate each string
    EEPROM.write(address++, '\0');
  }

  EEPROM.commit();
  EEPROM.end();
}
void formatData(){
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}