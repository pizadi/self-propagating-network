void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  for (unsigned char i = 1; i < 100; i++) Serial.print((byte) i);
  Serial.print('\0');
  Serial.flush();
  delay(1000);
}
