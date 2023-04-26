#include <LiquidCrystal_I2C.h>
//#include <mbedtls/aes.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
char lcdBuffer[81];
uint8_t pointer = 0;
byte messageBuffer[16] = {0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void print_fix(char * str) {
  char cbuffer[21];
  cbuffer[20] = '\0';
  int i = 0, t = 0, l = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  while (t < 80) {
    if (str[t] == '\0') {
      cbuffer[i] = '\0';
      lcd.printstr(cbuffer);
      break;
    }
    else if (i < 20) {
      cbuffer[i] = str[t];
      i++;
      t++;
    }
    else {
      lcd.printstr(cbuffer);
      l++;
      lcd.setCursor(0, l);
      cbuffer[0] = str[t];
      t++;
      i = 1;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("1");
  Serial2.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial.println("2");
  int a, b;
  uint64_t t = millis();
  Serial.println("3");
  for (int i = 0; i < 16; i++) {
    while (Serial2.availableForWrite() < 1);
    Serial2.write(messageBuffer[i]);
  }
  Serial.println("4");
  delay(1);
  t = millis() - t;
  double r = 16000 * 1000 / 100;
  sprintf(lcdBuffer, "%.3lf Bps", r);
  Serial.println("5");
  lcd.printstr(lcdBuffer); 
}

void loop() {
  while(Serial2.available()) {
    char a = Serial2.read();
    lcdBuffer[pointer] = a;
    if (a == '\r') {
      lcdBuffer[pointer] = '\0';
      lcd.clear();
      print_fix(lcdBuffer);
      pointer = 0;
    }
    else {
      pointer++;
      if (pointer > 80) {
        pointer = 0;
      }
    }
  }

}
