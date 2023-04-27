#include <LiquidCrystal_I2C.h>
#include <mbedtls/aes.h>
#include <esp_random.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
char lcdBuffer[81];
uint8_t pointer = 0;

byte messageBuffer[16]  {0x02, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
byte networkId[4] = {0x01, 0x02, 0x03, 0x04};
byte deviceId[4] = {0x01, 0x02, 0x03, 0x04};
byte aeskey[16] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04};
byte dataBuffer[256];
uint16_t ptr = 0;

void print_fix(char * str);
void sendpacket(byte * buf, uint32_t len);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  int a, b;
  uint64_t t = esp_timer_get_time();
  for (int i = 0; i < 9; i++) {
    while (Serial2.availableForWrite() < 1);
    Serial2.write(messageBuffer[i]);
  }
  delay(1);
  t = esp_timer_get_time() - t;
  double r = 9 * 1000000 / t;
  sprintf(lcdBuffer, "%.3lf Bps", r);
  lcd.printstr(lcdBuffer); 
}

void loop() {
  // TODO: Rewrite as an interrupt with proper timeout
  while(Serial.available()) {
    char tmp = Serial.read();
    dataBuffer[ptr] = tmp;
    ptr++;
  }
  if (Serial.available() == 0 && ptr > 0){
    uint16_t len = ptr / 16 * 16 + 16 * (ptr % 16 > 0);
    messageBuffer[0] = 1;
    messageBuffer[1] = networkId[0];
    messageBuffer[2] = networkId[1];
    messageBuffer[3] = networkId[2];
    messageBuffer[4] = networkId[3];
    messageBuffer[5] = deviceId[0];
    messageBuffer[6] = deviceId[1];
    messageBuffer[7] = deviceId[2];
    messageBuffer[8] = deviceId[3];    
    messageBuffer[9] = len;
    esp_fill_random(&messageBuffer[10], 6);
    sendpacket(messageBuffer, 16);
    uint32_t wptr = 0;
    while (wptr < ptr) {
      for (int i = 0; i < 16; i++) messageBuffer[i] = dataBuffer[wptr+i];
      sendpacket(messageBuffer, 16);
      wptr += 16;
    }
    ptr = 0;
  }

}



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

void sendpacket(byte * buf, uint32_t len){
  for (int i = 0; i < len; i++){
    while (Serial2.availableForWrite() < 1);
    Serial2.write(buf[i]);    
  }
}
