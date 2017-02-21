
#define PIN_CLK D4
#define PIN_DATA D3

uint8_t bitIndex = 0;
uint8_t byteIndex = 0;
uint8_t clkValue = LOW;
uint8_t lastClkValue = LOW;

uint8_t tmp = 0;
unsigned long currentMillis = 0;
unsigned long lastMillis = 0;

uint16_t co2Measurement = 0;

byte bits[8];
byte readBytes[5] = {0};

void setup() {
  Serial.begin(115200);

  pinMode(PIN_CLK, INPUT);
  pinMode(PIN_DATA, INPUT);

  attachInterrupt(PIN_CLK, onClock, RISING);
}

void onClock() {
  
  lastMillis = millis();
  bits[bitIndex++] = (digitalRead(PIN_DATA) == HIGH) ? 1 : 0;

  // Transform bits to byte
  if (bitIndex >= 8) {
    tmp = 0;
    for (uint8_t i = 0; i < 8; i++) {
      tmp |= (bits[i] << (7 - i));
    }

    readBytes[byteIndex++] = tmp;
    bitIndex = 0;
  }

  if (byteIndex >= 5) {
    byteIndex = 0;
    decodeDataPackage(readBytes);
  }
}


void loop() {
  currentMillis = millis();

  // Over 50ms no bits? Reset!
  if (currentMillis - lastMillis > 50) {
    bitIndex = 0;
    byteIndex = 0;
  }
}

bool decodeDataPackage(byte data[5]) {

  if (data[4] != 0x0D) {
    return false;
  }

  uint8_t checksum = data[0] + data[1] + data[2];
  if (data[3] != checksum) {
    return false;
  }

  switch (data[0]) {
    case 0x50:
      co2Measurement = (data[1] << 8) | data[2];
      Serial.print("CO2: ");
      Serial.println(co2Measurement);
      break;
  }

}

