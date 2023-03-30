#include <LiquidCrystal_I2C.h>

#define BAUD 9600
#define RXPIN 4
#define TXPIN 5                                              
#define DELTA 0.33
#define PACKETSIZE 32
#define TIMEOUT 1.0

float dataCorruption = 0.;
float avgT = 0.;
uint64_t lastT = 0;
byte packet[100];
unsigned char packetPointer = 0;
char lineBuffer[21];
uint8_t icon = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool rcv = false;
uint32_t timeouts = 0, packets = 0;

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
  lcd.setCursor(19,3);
  lcd.write(t);
  icon = (icon + 1) % 2;
}

void packetEval(){
  char bytebuffer[10];
  unsigned char err = 0;
  avgT = avgT * (1 - DELTA) + DELTA * (millis() - lastT) / 1000;
  for (byte i = 1; i < PACKETSIZE; i++) {
    if(i != packet[i-1]) err++;
    sprintf(bytebuffer, "%02x%02x ", i, packet[i-1]);
    Serial.print(bytebuffer);
  }
  Serial.println("");
  Serial.flush();
  dataCorruption = dataCorruption * (1 - DELTA) + DELTA * err * 100 / (PACKETSIZE - 1);
  updateLCD();
  pingLCD();
}

void errWipe(){
  dataCorruption = dataCorruption * (1 - DELTA) + DELTA * 100;
  packetPointer = 0;
  avgT = avgT * (1 - DELTA) + DELTA * TIMEOUT;
  updateLCD();
  lcd.setCursor(19,3);
  lcd.write('!');
}

void updateLCD(){
  sprintf(lineBuffer, "Loss:    %06.2f %%", dataCorruption);
  lcd.setCursor(0,0);
  lcd.printstr(lineBuffer);
  sprintf(lineBuffer, "Delay:   %06.3f s", avgT);
  lcd.setCursor(0,1);
  lcd.printstr(lineBuffer);
  float loss;
  if (packets > 0) loss = 100. * timeouts / packets;
  else loss = 0.;
  sprintf(lineBuffer, "Timeout: %6.2f %%", loss);
  lcd.setCursor(0,2);
  lcd.printstr(lineBuffer);
}

void onReceiveFunction(void) {
  while (Serial1.available() > 0) {
    packet[packetPointer] = (byte) Serial1.read();
    packetPointer++;
    if (packet[packetPointer-1] == '\0'){
      rcv = true;
      packetPointer = 0;
      packetEval();
    }
    if (packetPointer > 100){
      packetPointer = 0;
      errWipe();
    }
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial.begin(9600);
  Serial1.begin(BAUD, SERIAL_8N1, RXPIN, TXPIN);
  Serial1.onReceive(onReceiveFunction, true);
  lastT = millis();
  updateLCD();
}

void loop() {
  lastT = millis();
  for (byte i = 1; i < PACKETSIZE; i++) Serial1.write(i);
  Serial1.write('\0');
  Serial1.flush();
  packets++;
  delay((int) 1000 * TIMEOUT);
  if (!rcv) {
    errWipe();
    timeouts++;
  }
  rcv = false;
}
