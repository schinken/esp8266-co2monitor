#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SimpleTimer.h>
#include "settings.h"

SimpleTimer timer;
WiFiClient wifiClient;
PubSubClient mqttClient;

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
char sprintfHelper[16] = {0};

void setup() {
  Serial.begin(115200);

  pinMode(PIN_CLK, INPUT);
  pinMode(PIN_DATA, INPUT);

  attachInterrupt(PIN_CLK, onClock, RISING);

  WiFi.hostname(WIFI_HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(MQTT_HOST, 1883);

  mqttConnect();

  timer.setInterval(PUBLISH_INTERVAL_MS, []() {
    if (co2Measurement > 0) {
      sprintf(sprintfHelper, "%d", co2Measurement);
      mqttClient.publish(MQTT_TOPIC_CO2_MEASUREMENT, sprintfHelper, true);
    }
  });
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

void mqttConnect() {
  while (!mqttClient.connected()) {
    mqttClient.connect(WIFI_HOSTNAME, MQTT_TOPIC_SENSOR_STATE, 1, true, "disconnected");
    mqttClient.publish(MQTT_TOPIC_SENSOR_STATE, "connected", true);

    delay(1000);
  }
}

void loop() {
  currentMillis = millis();

  // Over 50ms no bits? Reset!
  if (currentMillis - lastMillis > 50) {
    bitIndex = 0;
    byteIndex = 0;
  }

  mqttConnect();
  mqttClient.loop();

  timer.run();
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
      break;
  }

}

