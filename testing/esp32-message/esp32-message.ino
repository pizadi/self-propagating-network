#include <LiquidCrystal_I2C.h>
#include <mbedtls/aes.h>
#include <esp_random.h>
#include <esp32/rom/crc.h>


#define TIMEOUT 50

char lcdBuffer[81];
uint8_t pointer = 0;

byte messageBuffer[16]  {0x02, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
byte networkId[4] = {0x01, 0x02, 0x03, 0x04};
byte deviceId[4] = {0x00, 0x00, 0x00, 0x00};
byte aeskey[16] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04};
byte dataBuffer[256];
byte encBuffer[256];
byte ivBuffer[16];
size_t ivOffset;
uint16_t ptr = 0;
uint64_t serial0timer = 0;
uint32_t timebuffer = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);
mbedtls_aes_context aesctx;

void print_fix(char * str);
void sendpacket(byte * buf, uint32_t len);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  int a = 0;
  // AES context initialization
  mbedtls_aes_init(&aesctx);
  a += mbedtls_aes_setkey_enc(&aesctx, (const unsigned char *) aeskey, 128);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  if (a == 0) lcd.printstr("AES INIT");
  else lcd.printstr("AES FAILED");
  for (int i = 0; i < 9; i++) {
    while (Serial2.availableForWrite() < 1);
    Serial2.write(messageBuffer[i]);
  }
}

void loop() {
  // TODO: Rewrite as an interrupt with proper timeout
  while(Serial.available()) {
    if (ptr >= 231) {
      Serial.println("Serial input overflow occured.");
      break;
    }
    char tmp = Serial.read();
    dataBuffer[ptr] = tmp;
    ptr++;
    serial0timer = millis();
  }
//  Serial.println(millis() - serial0timer);
  if ((millis() - serial0timer > TIMEOUT) && (ptr > 0)){
    uint16_t len;
    if (ptr % 16 < 11) {
      len = ptr / 16 * 16 + 16;
    }
    else {
      len = ptr / 16 * 16 + 32;
    }
    for (int i = 0; i < ptr; i++) {
      Serial.print((char) dataBuffer[i]);
    }
    Serial.println("");
    sprintf(lcdBuffer, "LEN: %u PTR: %u", len, ptr);
    Serial.println(lcdBuffer);
    uint32_t romCRC = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)dataBuffer, ptr))^0xffffffff;
    for (int i = ptr; i < len-9; i++) dataBuffer[i] = 0;
    dataBuffer[len-9] = ptr % 256;
    dataBuffer[len-8] = (timebuffer >> 24) & 0xff;
    dataBuffer[len-7] = (timebuffer >> 16) & 0xff;
    dataBuffer[len-6] = (timebuffer >> 8) & 0xff;
    dataBuffer[len-5] = timebuffer & 0xff;
    dataBuffer[len-4] = (romCRC >> 24) & 0xff;
    dataBuffer[len-3] = (romCRC >> 16) & 0xff;
    dataBuffer[len-2] = (romCRC >> 8) & 0xff;
    dataBuffer[len-1] = romCRC & 0xff;

    
    messageBuffer[0] = 1;
    messageBuffer[1] = networkId[0];
    messageBuffer[2] = networkId[1];
    messageBuffer[3] = networkId[2];
    messageBuffer[4] = networkId[3];
    messageBuffer[5] = deviceId[0];
    messageBuffer[6] = deviceId[1];
    messageBuffer[7] = deviceId[2];
    messageBuffer[8] = deviceId[3];    
    messageBuffer[9] = len % 256;
    esp_fill_random(&messageBuffer[10], 6);
    sendpacket(messageBuffer, 16);
    for (int i = 0; i < 16; i++) ivBuffer[i] = messageBuffer[i];
    mbedtls_aes_crypt_cfb128(&aesctx, MBEDTLS_AES_ENCRYPT, len, &ivOffset, ivBuffer, dataBuffer, encBuffer);
    sendpacket(encBuffer, len);
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
