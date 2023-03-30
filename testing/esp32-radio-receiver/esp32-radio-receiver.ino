#include <LiquidCrystal_I2C.h>

#define BAUD 9600
#define RXPIN 4
#define TXPIN 5                                              
#define DELTA 0.25

float dataCorruption = 0.;
float avgT = 1.;
uint64_t lastT = 0;
byte packet[100];
unsigned char packetPointer = 0;
char lineBuffer[21];
uint8_t icon = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);

void pingLCD(){
  char t;
  switch (icon){
    case 0:
    t = '+';
    break;
    case 1:
    t = '-';
    break;
    default:
    t = '.';
    break;
  }
  lcd.setCursor(19,0);
  lcd.write(t);
  icon = (icon + 1) % 2;
}

void packetEval(){
  unsigned char err = 0;
  for (byte i = 1; i < 100; i++) if(i != packet[i-1]) err++;
  dataCorruption = dataCorruption * (1 - DELTA) + DELTA * err;
  avgT = avgT * (1 - DELTA) + DELTA * (millis() - lastT) / 1000;
  lastT = millis();
}

void errWipe(){
  dataCorruption = dataCorruption * (1 - DELTA) + DELTA * 100;
}

void updateLCD(){
  sprintf(lineBuffer, "Loss: %3.3f", dataCorruption);
  lcd.setCursor(0,0);
  lcd.printstr(lineBuffer);
  sprintf(lineBuffer, "Delay: %3.3f", avgT);
  lcd.setCursor(0,1);
  lcd.printstr(lineBuffer);
}

void onReceiveFunction(void) {
  // TODO: widget update
  pingLCD();
  while (Serial1.available() > 0) {
    packet[packetPointer] = (byte) Serial1.read();
    if (packet[packetPointer] == '\0'){
      packetPointer = 0;
      packetEval();
    }
    packetPointer++;
    if (packetPointer > 100){
      packetPointer = 0;
      errWipe();
    }
  }
  updateLCD();
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);
  Serial1.onReceive(onReceiveFunction);
  lastT = millis();
  updateLCD();
}

void loop() {
  
}
