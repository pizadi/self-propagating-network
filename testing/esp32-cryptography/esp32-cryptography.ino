#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
                        
const uint32_t h_init[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
typedef struct hashobj {
  uint32_t hstate[8];
  uint64_t bitlen;
  char dcache[64];
} hashobj;

void hashinit(hashobj * hash){
  int i;
  for (i=0; i<8; i++) hash->hstate[i] = h_init[i];
  hash->bitlen = 0;
}

void updatehash(hashobj * hash){
  uint32_t w[64];
  int i;
  
  for (i=0;i<16;i++){
    uint8_t ri = i*4;
    w[i] = hash->dcache[i+3] + ((uint32_t) hash->dcache[i+2] << 8) + ((uint32_t) hash->dcache[i+1] << 16) + ((uint32_t) hash->dcache[i] << 24);
  }
  for (i=16;i<64;i++){
    uint32_t ws[6], s0, s1;
    ws[0] = (w[i-15] >> 7) + (w[i-15] << 25);
    ws[1] = (w[i-15] >> 18) + (w[i-15] << 14);
    ws[2] = w[i-15] >> 3;
    ws[3] = (w[i-2] >> 17) + (w[i-2] >> 15);
    ws[4] = (w[i-2] >> 19) + (w[i-2] >> 13);
    ws[5] = w[i-2] >> 10;
    s0 = ws[0] ^ ws[1] ^ ws[2];
    s1 = ws[3] ^ ws[4] ^ ws[5];
    w[i] = w[i-16] + s0 + w[i-7] + s1;
  }
  
  uint32_t wv[8];
  for (i=0; i<8; i++) wv[i] = hash->hstate[i];

  for (i=0; i<64; i++){
    uint32_t wv0s[3], wv4s[3], S0, S1, ch, maj, temp1, temp2;
    wv0s[0] = (wv[0] >> 2) + (wv[0] << 30);
    wv0s[1] = (wv[0] >> 13) + (wv[0] << 19);
    wv0s[2] = (wv[0] >> 22) + (wv[0] << 10);
    wv4s[0] = (wv[4] >> 6) + (wv[4] << 26);
    wv4s[1] = (wv[4] >> 11) + (wv[4] << 21);
    wv4s[2] = (wv[4] >> 25) + (wv[4] << 7);

    S1 = wv4s[0] ^ wv4s[1] ^ wv4s[2];
    ch = (wv[4] & wv[5]) ^ ((~wv[4]) & wv[6]);
    temp1 = wv[7] + S1 + ch + k[i] + w[i];
    S0 = wv0s[0] ^ wv0s[1] ^ wv0s[2];
    maj = (wv[0] & wv[1]) ^ (wv[0] & wv[2]) ^ (wv[1] & wv[2]);
    temp2 = S0 + maj;

    wv[7] = wv[6];
    wv[6] = wv[5];
    wv[5] = wv[4];
    wv[4] = wv[3] + temp1;
    wv[3] = wv[2];
    wv[2] = wv[1];
    wv[1] = wv[0];
    wv[0] = temp1 + temp2;
  }
  
  for (i=0; i<8; i++) hash->hstate[i] += wv[i];
  
}

void addbyte(hashobj * hash, char bytedata){
  uint8_t ptr = (hash->bitlen % 512) / 8;
  hash->dcache[ptr] = bytedata;
  hash->bitlen += 8;
  if (ptr == 63) updatehash(hash);
}

void calchash(hashobj * hash, char * out){
  uint8_t ptr = (hash->bitlen % 512) / 8;
  hash->dcache[ptr] = 0x80;
  ptr++;
  if (ptr > 56) {
    
    while (ptr < 64) {
      hash->dcache[ptr] = 0;
      ptr++;
    }
    updatehash(hash);
    ptr = 0;
  }
  while (ptr < 56) {
    hash->dcache[ptr] = 0;
    ptr++;
  }
  while (ptr < 63){
    hash->dcache[ptr] = (hash->bitlen) >> (8*(63-ptr)) % 256;
    ptr++;
  }
  updatehash(hash);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  String input = "LegioAeterna";
  hashobj hash;
  hashinit(&hash);
  int i;
//  for (i=0; i < input.length(); i++){
//    addbyte(&hash, input.charAt(i));
//  }
  char * hashout = (char *) malloc(sizeof(char) * 64);
  calchash(&hash, hashout);
  for (i=0; i< 8; i++) Serial.print(hash.hstate[i], HEX);
  Serial.println("");
//  lcd.clear();
//  for (i=0; i < 64; i++){
//    lcd.setCursor(i % 16, i / 16);
//    lcd.write(hashout[i]);
//  }
}

void loop() {
  if(Serial.available()){
    String input = Serial.readString();
    hashobj hash;
    hashinit(&hash);
    int i;
    for (i=0; i < input.length(); i++){
      addbyte(&hash, input.charAt(i));
    }
    char * hashout = (char *) malloc(sizeof(char) * 64);
    calchash(&hash, hashout);
    lcd.clear();
    for (i=0; i < 64; i++){
      lcd.setCursor(i % 16, i / 16);
      lcd.write(hashout[i]);
    }
  }
}
