#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "dep/pubsubclient-2.7/src/PubSubClient.cpp"
#include "dep/WiFiManager-0.15.0/WiFiManager.cpp"


#include "settings.h"

#define IDX_CMD 0
#define IDX_MSB 1
#define IDX_LSB 2
#define IDX_CHECKSUM 3
#define IDX_END 4

#define CMD_TEMPERATURE 0x42
#define CMD_CO2_MEASUREMENT 0x50

#ifdef USE_HA_AUTODISCOVERY
  #define FIRMWARE_PREFIX "esp8266-co2monitor"
  char MQTT_TOPIC_LAST_WILL[128];
  char MQTT_TOPIC_CO2_MEASUREMENT[128];
  char MQTT_TOPIC_TEMPERATURE_MEASUREMENT[128];
#endif

WiFiClient wifiClient;
PubSubClient mqttClient;

uint8_t bitIndex = 0;
uint8_t byteIndex = 0;
uint8_t clkValue = LOW;
uint8_t lastClkValue = LOW;

uint8_t tmp = 0;
unsigned long currentMillis = 0;
unsigned long lastMillis = 0;
unsigned long lastUpdateMs = 0;
uint8_t mqttRetryCounter = 0;

uint16_t co2Measurement = 0;
float smoothCo2Measurement = 0.0;

float temperature = 0;

byte bits[8];
byte bytes[5] = {0};

char sprintfHelper[16] = {0};

char hostname[16];

void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  Serial.println("Hello from esp8266-co2monitor");

  // Power up wait
  delay(2000);

  WiFiManager wifiManager;
  int32_t chipid = ESP.getChipId();

  Serial.print("MQTT_MAX_PACKET_SIZE: ");
  Serial.println(MQTT_MAX_PACKET_SIZE);


#ifdef HOSTNAME
  hostname = HOSTNAME;
#else
  snprintf(hostname, 24, "CO2MONITOR-%X", chipid);
#endif

#ifdef USE_HA_AUTODISCOVERY
  snprintf(MQTT_TOPIC_LAST_WILL, 127, "%s/%s/presence", FIRMWARE_PREFIX, hostname);
  snprintf(MQTT_TOPIC_CO2_MEASUREMENT, 127, "%s/%s/%s_%s/state", FIRMWARE_PREFIX, hostname, hostname, "co2");
  snprintf(MQTT_TOPIC_TEMPERATURE_MEASUREMENT, 127, "%s/%s/%s_%s/state", FIRMWARE_PREFIX, hostname, hostname, "temp");
#endif

#ifdef CONF_WIFI_PASSWORD
  wifiManager.autoConnect(hostname, CONF_WIFI_PASSWORD);
#else
  wifiManager.autoConnect(hostname);
#endif

  WiFi.hostname(hostname);
  mqttClient.setClient(wifiClient);
  mqttClient.setServer(MQTT_HOST, 1883);

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  pinMode(PIN_CLK, INPUT);
  pinMode(PIN_DATA, INPUT);

  attachInterrupt(PIN_CLK, onClock, RISING);

  Serial.print("Hostname: ");
  Serial.println(hostname);

  Serial.println("-- Current GPIO Configuration --");
  Serial.print("PIN_CLK: ");
  Serial.println(PIN_CLK);
  Serial.print("PIN_DATA: ");
  Serial.println(PIN_DATA);

  mqttConnect();
}

ICACHE_RAM_ATTR void onClock() {

  lastMillis = millis();
  bits[bitIndex++] = (digitalRead(PIN_DATA) == HIGH) ? 1 : 0;

  // Transform bits to byte
  if (bitIndex >= 8) {
    tmp = 0;
    for (uint8_t i = 0; i < 8; i++) {
      tmp |= (bits[i] << (7 - i));
    }

    bytes[byteIndex++] = tmp;
    bitIndex = 0;
  }

  if (byteIndex >= 5) {
    byteIndex = 0;
    decodeDataPackage(bytes);
  }
}

void mqttConnect() {
  while (!mqttClient.connected()) {

    bool mqttConnected = false;
    if (MQTT_USERNAME && MQTT_PASSWORD) {
      mqttConnected = mqttClient.connect(hostname, MQTT_USERNAME, MQTT_PASSWORD, MQTT_TOPIC_LAST_WILL, 1, true, MQTT_LAST_WILL_PAYLOAD_DISCONNECTED);
    } else {
      mqttConnected = mqttClient.connect(hostname, MQTT_TOPIC_LAST_WILL, 1, true, MQTT_LAST_WILL_PAYLOAD_DISCONNECTED);
    }

    if (mqttConnected) {
      Serial.println("Connected to MQTT Broker");
      mqttClient.publish(MQTT_TOPIC_LAST_WILL, MQTT_LAST_WILL_PAYLOAD_CONNECTED, true);
      mqttRetryCounter = 0;

      #ifdef USE_HA_AUTODISCOVERY
        setupHAAutodiscovery();
      #endif

    } else {
      Serial.println("Failed to connect to MQTT Broker");

      if (mqttRetryCounter++ > MQTT_MAX_CONNECT_RETRY) {
        Serial.println("Restarting uC");
        ESP.restart();
      }

      delay(2000);
    }
  }
}


#ifdef USE_HA_AUTODISCOVERY
#define AUTOCONFIG_PAYLOAD_TPL_TEMP  "{\
\"stat_t\":\"%s/%s/%s_temp/state\",\
\"unique_id\":\"%s_temp\",\
\"name\":\"%s Temp\",\
\"unit_of_meas\":\"°C\",\
\"dev_cla\":\"temperature\",\
\"dev\": {\
\"identifiers\":\"%s\",\
\"name\":\"%s\",\
\"manufacturer\":\"RADIANT INNOVATION INC\",\
\"model\":\"ZyAura ZGm05\"\
}\
}"
#define AUTOCONFIG_PAYLOAD_TPL_CO2  "{\
\"stat_t\":\"%s/%s/%s_co2/state\",\
\"unique_id\":\"%s_co2\",\
\"name\":\"%s CO2\",\
\"unit_of_meas\":\"ppm\",\
\"dev\": {\
\"identifiers\":\"%s\",\
\"name\":\"%s\",\
\"manufacturer\":\"RADIANT INNOVATION INC\",\
\"model\":\"ZyAura ZGm05\"\
}\
}"

void setupHAAutodiscovery() {
  char autoconfig_topic_co2[128];
  char autoconfig_payload_co2[1024];
  char autoconfig_topic_temp[128];
  char autoconfig_payload_temp[1024];

  snprintf(
    autoconfig_topic_co2,
    127,
    "%s/sensor/%s/%s_co2/config",
    HA_DISCOVERY_PREFIX,
    hostname,
    hostname
  );

  snprintf(
    autoconfig_topic_temp,
    127,
    "%s/sensor/%s/%s_temp/config",
    HA_DISCOVERY_PREFIX,
    hostname,
    hostname
  );

  snprintf(
    autoconfig_payload_co2,
    1023,

    AUTOCONFIG_PAYLOAD_TPL_CO2,

    FIRMWARE_PREFIX,
    hostname,
    hostname,
    hostname,
    hostname,
    hostname,
    hostname
  );

  snprintf(
    autoconfig_payload_temp,
    1023,

    AUTOCONFIG_PAYLOAD_TPL_TEMP,

    FIRMWARE_PREFIX,
    hostname,
    hostname,
    hostname,
    hostname,
    hostname,
    hostname
  );

  Serial.println(autoconfig_topic_co2);
  Serial.println(autoconfig_payload_co2);
  if(
    mqttClient.publish(autoconfig_topic_co2, autoconfig_payload_co2, true) &&
    mqttClient.publish(autoconfig_topic_temp, autoconfig_payload_temp, true)
  ){
    Serial.println("Autoconf publish successful");
  } else {
    Serial.println("Autoconf publish failed. Is MQTT_MAX_PACKET_SIZE large enough?");
  }
}
#endif

void loop() {
  currentMillis = millis();

  // Over 50ms no bits? Reset!
  if (currentMillis - lastMillis > 50) {
    bitIndex = 0;
    byteIndex = 0;
  }

  long updateInterval = PUBLISH_INTERVAL_SLOW_MS;

  // If the change is above a specific threshold, we update faster!
  float percentChange = abs(((float) co2Measurement / smoothCo2Measurement) - 1.0);
  if (percentChange > 0.05) {
    updateInterval = PUBLISH_INTERVAL_FAST_MS;
  }

  if (currentMillis - lastUpdateMs > updateInterval) {
    lastUpdateMs = millis();

    if (co2Measurement > 0) {
      sprintf(sprintfHelper, "%d", co2Measurement);
      mqttClient.publish(MQTT_TOPIC_CO2_MEASUREMENT, sprintfHelper, true);
    }

    if (temperature > 0) {
      dtostrf(temperature, 4, 2, sprintfHelper);
      mqttClient.publish(MQTT_TOPIC_TEMPERATURE_MEASUREMENT, sprintfHelper, true);
    }

  }

  mqttConnect();
  mqttClient.loop();

  ArduinoOTA.handle();
  delay(40);
}

bool decodeDataPackage(byte data[5]) {

  if (data[IDX_END] != 0x0D) {
    return false;
  }

  uint8_t checksum = data[IDX_CMD] + data[IDX_MSB] + data[IDX_LSB];
  if (data[IDX_CHECKSUM] != checksum) {
    return false;
  }

  switch (data[IDX_CMD]) {
    case CMD_CO2_MEASUREMENT:
      co2Measurement = (data[IDX_MSB] << 8) | data[IDX_LSB];

      // Exponential smoothing
      smoothCo2Measurement = EXP_SMOOTH_ALPHA * (float) co2Measurement + (1.0 - EXP_SMOOTH_ALPHA) * smoothCo2Measurement;

      Serial.print("CO2: ");
      Serial.println(co2Measurement);
      break;
    case CMD_TEMPERATURE:
      temperature = ((data[IDX_MSB] << 8) | data[IDX_LSB]) / 16.0 - 273.15;
      Serial.print("Temp: ");
      Serial.println(temperature);
      break;
  }
}
