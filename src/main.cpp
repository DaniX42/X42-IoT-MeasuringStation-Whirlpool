#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DHT.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_system.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#if __has_include("device_config.h")
#include "device_config.h"
#else
#include "device_config.example.h"
#endif

#ifndef X42_CFG_TEMP_OFFSET_C
#define X42_CFG_TEMP_OFFSET_C 0.0F
#endif

#ifndef X42_CFG_HUMIDITY_OFFSET_PCT
#define X42_CFG_HUMIDITY_OFFSET_PCT 0.0F
#endif

namespace {
constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kSamplingWindowMs = 400;
constexpr uint32_t kZeroCalibrationWindowMs = 2000;
constexpr uint32_t kClimateReadIntervalMs = 5000;
constexpr uint32_t kClimateStaleAfterMs = 30000;
constexpr uint32_t kMqttReconnectIntervalMs = 5000;
constexpr uint32_t kSessionLifetimeMs = 15UL * 60UL * 1000UL;
constexpr uint32_t kWifiConnectTimeoutMs = 20000;
constexpr uint32_t kNtpSyncTimeoutMs = 8000;
constexpr uint16_t kHttpPort = 80;
constexpr uint8_t kDhtPin = 4;
constexpr uint8_t kDhtType = DHT11;
constexpr float kAdcReferenceVoltage = 3.3F;
constexpr uint16_t kAdcMax = 4095;
constexpr time_t kValidEpochThreshold = 1700000000;

constexpr char kZeroNs[] = "sct013";
constexpr char kConfigNs[] = "appcfg";
constexpr char kConfigRevision[] = __DATE__ " " __TIME__;
constexpr char kPendingOtaKey[] = "pending_ota";

enum class ConfigBootAction : uint8_t {
  None,
  PersistLoaded,
  PersistDefaults,
};

struct CurrentChannel {
  const char* phase;
  uint8_t pin;
  float ampsPerVolt;
};

CurrentChannel gChannels[] = {
    {"L1", 34, 0.778F},
    {"L2", 35, 0.377F},
    {"L3", 32, 0.863F},
};

constexpr float kDefaultZeroOffsetVrms[] = {
    0.005F / 0.778F,
    0.002F / 0.377F,
    0.004F / 0.863F,
};

struct DeviceConfig {
  char adminUser[32];
  char adminPassword[32];
  char wifiSsid[33];
  char wifiPassword[65];
  char hostname[33];
  char mqttServer[65];
  uint16_t mqttPort;
  char mqttUser[33];
  char mqttPassword[65];
  char mqttBaseTopic[65];
  char ntpServer[65];
  int16_t timezoneOffsetMinutes;
  float tempOffsetC;
  float humidityOffsetPct;
  uint16_t publishIntervalSec;
  char sensorL1[24];
  char sensorL2[24];
  char sensorL3[24];
  char sensorTemp[24];
  char sensorHumidity[24];
};

struct ChannelMeasurement {
  float irms;
  float meanVoltage;
  float vrmsVoltage;
};

struct PhaseRuntime {
  float currentA;
  float rawCurrentA;
  float zeroOffsetA;
  float meanVoltage;
};

struct ClimateState {
  bool hasValidSample;
  float temperatureC;
  float humidityPercent;
  uint32_t lastReadMs;
  uint32_t lastSuccessMs;
};

struct RuntimeState {
  PhaseRuntime phases[sizeof(gChannels) / sizeof(gChannels[0])];
  float totalCurrentA;
  uint32_t lastReportMs;
};

float gZeroOffsetVrms[sizeof(gChannels) / sizeof(gChannels[0])] = {};
DeviceConfig gConfig = {};
RuntimeState gRuntime = {};
ClimateState gClimate = {false, 0.0F, 0.0F, 0, 0};

DHT gDht(kDhtPin, kDhtType);
Preferences gPreferences;
WebServer gServer(kHttpPort);
WiFiClient gWifiClient;
PubSubClient gMqtt(gWifiClient);

String gSessionToken;
uint32_t gSessionExpiresMs = 0;
uint32_t gLastMqttReconnectMs = 0;
uint32_t gLastMqttPublishMs = 0;
bool gIsApMode = false;
String gLastTimeSyncSource;
time_t gLastTimeSyncEpoch = 0;
}

void copyString(char* dst, size_t dstSize, const String& value) {
  if (dstSize == 0) {
    return;
  }

  const size_t maxCopy = dstSize - 1;
  const size_t length = value.length() < maxCopy ? value.length() : maxCopy;
  memcpy(dst, value.c_str(), length);
  dst[length] = '\0';
}

void setDefaults(DeviceConfig& cfg) {
  copyString(cfg.adminUser, sizeof(cfg.adminUser), X42_CFG_ADMIN_USER);
  copyString(cfg.adminPassword, sizeof(cfg.adminPassword), X42_CFG_ADMIN_PASSWORD);
  copyString(cfg.wifiSsid, sizeof(cfg.wifiSsid), X42_CFG_WIFI_SSID);
  copyString(cfg.wifiPassword, sizeof(cfg.wifiPassword), X42_CFG_WIFI_PASSWORD);
  copyString(cfg.hostname, sizeof(cfg.hostname), X42_CFG_HOSTNAME);
  copyString(cfg.mqttServer, sizeof(cfg.mqttServer), X42_CFG_MQTT_SERVER);
  cfg.mqttPort = X42_CFG_MQTT_PORT;
  copyString(cfg.mqttUser, sizeof(cfg.mqttUser), X42_CFG_MQTT_USER);
  copyString(cfg.mqttPassword, sizeof(cfg.mqttPassword), X42_CFG_MQTT_PASSWORD);
  copyString(cfg.mqttBaseTopic, sizeof(cfg.mqttBaseTopic), X42_CFG_MQTT_BASE_TOPIC);
  copyString(cfg.ntpServer, sizeof(cfg.ntpServer), X42_CFG_NTP_SERVER);
  cfg.timezoneOffsetMinutes = X42_CFG_TZ_OFFSET_MIN;
  cfg.tempOffsetC = X42_CFG_TEMP_OFFSET_C;
  cfg.humidityOffsetPct = X42_CFG_HUMIDITY_OFFSET_PCT;
  cfg.publishIntervalSec = X42_CFG_PUBLISH_INTERVAL_SEC;
  copyString(cfg.sensorL1, sizeof(cfg.sensorL1), X42_CFG_SENSOR_L1);
  copyString(cfg.sensorL2, sizeof(cfg.sensorL2), X42_CFG_SENSOR_L2);
  copyString(cfg.sensorL3, sizeof(cfg.sensorL3), X42_CFG_SENSOR_L3);
  copyString(cfg.sensorTemp, sizeof(cfg.sensorTemp), X42_CFG_SENSOR_TEMP);
  copyString(cfg.sensorHumidity, sizeof(cfg.sensorHumidity), X42_CFG_SENSOR_HUMIDITY);
}

bool consumePendingOtaUpdateFlag() {
  if (!gPreferences.begin(kConfigNs, false)) {
    return false;
  }
  const bool pendingOta = gPreferences.getBool(kPendingOtaKey, false);
  if (pendingOta) {
    gPreferences.putBool(kPendingOtaKey, false);
  }
  gPreferences.end();
  return pendingOta;
}

void markPendingOtaUpdate() {
  if (!gPreferences.begin(kConfigNs, false)) {
    return;
  }
  gPreferences.putBool(kPendingOtaKey, true);
  gPreferences.end();
}

ConfigBootAction loadConfig(bool pendingOtaUpdate) {
  setDefaults(gConfig);
  if (!gPreferences.begin(kConfigNs, true)) {
    return ConfigBootAction::None;
  }

  const String persistedRevision = gPreferences.getString("cfg_rev", "");
  const bool revisionMatches = persistedRevision == String(kConfigRevision);
  const bool applyDefaultsOnThisFlash = pendingOtaUpdate ? (X42_CFG_APPLY_ON_OTA_FLASH != 0) : (X42_CFG_APPLY_ON_USB_FLASH != 0);

  if (!revisionMatches && applyDefaultsOnThisFlash) {
    gPreferences.end();
    return ConfigBootAction::PersistDefaults;
  }

  gPreferences.getString("admin_usr", gConfig.adminUser, sizeof(gConfig.adminUser));
  gPreferences.getString("admin_pw", gConfig.adminPassword, sizeof(gConfig.adminPassword));
  gPreferences.getString("wifi_ssid", gConfig.wifiSsid, sizeof(gConfig.wifiSsid));
  gPreferences.getString("wifi_pw", gConfig.wifiPassword, sizeof(gConfig.wifiPassword));
  gPreferences.getString("host", gConfig.hostname, sizeof(gConfig.hostname));
  gPreferences.getString("mqtt_srv", gConfig.mqttServer, sizeof(gConfig.mqttServer));
  gConfig.mqttPort = gPreferences.getUShort("mqtt_port", gConfig.mqttPort);
  gPreferences.getString("mqtt_usr", gConfig.mqttUser, sizeof(gConfig.mqttUser));
  gPreferences.getString("mqtt_pw", gConfig.mqttPassword, sizeof(gConfig.mqttPassword));
  gPreferences.getString("mqtt_base", gConfig.mqttBaseTopic, sizeof(gConfig.mqttBaseTopic));
  gPreferences.getString("ntp_srv", gConfig.ntpServer, sizeof(gConfig.ntpServer));
  gConfig.timezoneOffsetMinutes = gPreferences.getShort("tz_min", gConfig.timezoneOffsetMinutes);
  gConfig.tempOffsetC = gPreferences.getFloat("tmp_off", gConfig.tempOffsetC);
  gConfig.humidityOffsetPct = gPreferences.getFloat("hum_off", gConfig.humidityOffsetPct);
  gConfig.publishIntervalSec = gPreferences.getUShort("pub_int", gConfig.publishIntervalSec);
  gPreferences.getString("name_l1", gConfig.sensorL1, sizeof(gConfig.sensorL1));
  gPreferences.getString("name_l2", gConfig.sensorL2, sizeof(gConfig.sensorL2));
  gPreferences.getString("name_l3", gConfig.sensorL3, sizeof(gConfig.sensorL3));
  gPreferences.getString("name_t", gConfig.sensorTemp, sizeof(gConfig.sensorTemp));
  gPreferences.getString("name_h", gConfig.sensorHumidity, sizeof(gConfig.sensorHumidity));

  gPreferences.end();

  if (gConfig.publishIntervalSec < 1) {
    gConfig.publishIntervalSec = 1;
  }
  if (gConfig.publishIntervalSec > 3600) {
    gConfig.publishIntervalSec = 3600;
  }
  if (gConfig.timezoneOffsetMinutes < -720) {
    gConfig.timezoneOffsetMinutes = -720;
  }
  if (gConfig.timezoneOffsetMinutes > 840) {
    gConfig.timezoneOffsetMinutes = 840;
  }
  if (gConfig.tempOffsetC < -20.0F) {
    gConfig.tempOffsetC = -20.0F;
  }
  if (gConfig.tempOffsetC > 20.0F) {
    gConfig.tempOffsetC = 20.0F;
  }
  if (gConfig.humidityOffsetPct < -50.0F) {
    gConfig.humidityOffsetPct = -50.0F;
  }
  if (gConfig.humidityOffsetPct > 50.0F) {
    gConfig.humidityOffsetPct = 50.0F;
  }

  if (!revisionMatches) {
    return ConfigBootAction::PersistLoaded;
  }

  return ConfigBootAction::None;
}

void saveConfig() {
  if (!gPreferences.begin(kConfigNs, false)) {
    return;
  }

  gPreferences.putString("admin_usr", gConfig.adminUser);
  gPreferences.putString("admin_pw", gConfig.adminPassword);
  gPreferences.putString("cfg_rev", kConfigRevision);
  gPreferences.putString("wifi_ssid", gConfig.wifiSsid);
  gPreferences.putString("wifi_pw", gConfig.wifiPassword);
  gPreferences.putString("host", gConfig.hostname);
  gPreferences.putString("mqtt_srv", gConfig.mqttServer);
  gPreferences.putUShort("mqtt_port", gConfig.mqttPort);
  gPreferences.putString("mqtt_usr", gConfig.mqttUser);
  gPreferences.putString("mqtt_pw", gConfig.mqttPassword);
  gPreferences.putString("mqtt_base", gConfig.mqttBaseTopic);
  gPreferences.putString("ntp_srv", gConfig.ntpServer);
  gPreferences.putShort("tz_min", gConfig.timezoneOffsetMinutes);
  gPreferences.putFloat("tmp_off", gConfig.tempOffsetC);
  gPreferences.putFloat("hum_off", gConfig.humidityOffsetPct);
  gPreferences.putUShort("pub_int", gConfig.publishIntervalSec);
  gPreferences.putString("name_l1", gConfig.sensorL1);
  gPreferences.putString("name_l2", gConfig.sensorL2);
  gPreferences.putString("name_l3", gConfig.sensorL3);
  gPreferences.putString("name_t", gConfig.sensorTemp);
  gPreferences.putString("name_h", gConfig.sensorHumidity);

  gPreferences.end();
}

bool hasValidSystemTime() {
  return time(nullptr) >= kValidEpochThreshold;
}

String epochToUtcIso(time_t epoch) {
  if (epoch <= 0) {
    return String();
  }
  struct tm tmUtc;
  gmtime_r(&epoch, &tmUtc);
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tmUtc);
  return String(buffer);
}

void markTimeSync(const char* source) {
  gLastTimeSyncEpoch = time(nullptr);
  gLastTimeSyncSource = source;
}

bool applyEpochFromMqttPayload(const String& payloadRaw) {
  String payload = payloadRaw;
  payload.trim();

  String lower = payload;
  lower.toLowerCase();

  if (lower.startsWith("time=") || lower.startsWith("epoch=") || lower.startsWith("ts=") || lower.startsWith("unix=")) {
    payload = payload.substring(payload.indexOf('=') + 1);
  } else if (lower.startsWith("time:") || lower.startsWith("epoch:") || lower.startsWith("ts:") || lower.startsWith("unix:")) {
    payload = payload.substring(payload.indexOf(':') + 1);
  } else if (lower.startsWith("time ") || lower.startsWith("epoch ") || lower.startsWith("ts ") || lower.startsWith("unix ")) {
    payload = payload.substring(payload.indexOf(' ') + 1);
  }

  payload.trim();
  const time_t epoch = static_cast<time_t>(strtoll(payload.c_str(), nullptr, 10));
  if (epoch < 946684800) {
    return false;
  }

  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);
  markTimeSync("mqtt");
  return true;
}

void syncTimeFromNtp() {
  if (WiFi.status() != WL_CONNECTED || strlen(gConfig.ntpServer) == 0) {
    return;
  }

  configTime(static_cast<long>(gConfig.timezoneOffsetMinutes) * 60L, 0L, gConfig.ntpServer);
  const uint32_t start = millis();
  while (!hasValidSystemTime() && (millis() - start < kNtpSyncTimeoutMs)) {
    delay(100);
  }

  if (hasValidSystemTime()) {
    markTimeSync("ntp");
    Serial.printf("Time synced via NTP server %s: %s\n", gConfig.ntpServer, epochToUtcIso(gLastTimeSyncEpoch).c_str());
  } else {
    Serial.printf("NTP sync timeout for server %s\n", gConfig.ntpServer);
  }
}

void printChipInfo() {
  esp_chip_info_t chipInfo;
  esp_chip_info(&chipInfo);
  Serial.println("=== ESP32 Startup ===");
  Serial.printf("Model: ESP32 family\n");
  Serial.printf("Cores: %u\n", chipInfo.cores);
  Serial.printf("Revision: %u\n", chipInfo.revision);
  Serial.printf("Flash size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("CPU frequency: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.println("=====================");
}

void loadDefaultZeroCalibration() {
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    gZeroOffsetVrms[index] = kDefaultZeroOffsetVrms[index];
  }
}

bool loadPersistedZeroCalibration() {
  if (!gPreferences.begin(kZeroNs, true)) {
    return false;
  }

  const bool valid = gPreferences.getBool("valid", false);
  if (valid) {
    for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
      char key[8];
      snprintf(key, sizeof(key), "ch%u", static_cast<unsigned>(index + 1));
      gZeroOffsetVrms[index] = gPreferences.getFloat(key, kDefaultZeroOffsetVrms[index]);
    }
  }
  gPreferences.end();
  return valid;
}

void savePersistedZeroCalibration() {
  if (!gPreferences.begin(kZeroNs, false)) {
    return;
  }
  gPreferences.putBool("valid", true);
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    char key[8];
    snprintf(key, sizeof(key), "ch%u", static_cast<unsigned>(index + 1));
    gPreferences.putFloat(key, gZeroOffsetVrms[index]);
  }
  gPreferences.end();
}

ChannelMeasurement readChannelMeasurement(const CurrentChannel& channel, uint32_t windowMs) {
  const uint32_t startMs = millis();
  double sum = 0.0;
  double sumSquared = 0.0;
  uint32_t sampleCount = 0;

  while (millis() - startMs < windowMs) {
    const uint16_t raw = analogRead(channel.pin);
    const float voltage = (static_cast<float>(raw) * kAdcReferenceVoltage) / static_cast<float>(kAdcMax);
    sum += static_cast<double>(voltage);
    sumSquared += static_cast<double>(voltage) * static_cast<double>(voltage);
    ++sampleCount;
  }

  if (sampleCount == 0) {
    return {0.0F, 0.0F, 0.0F};
  }

  const double mean = sum / static_cast<double>(sampleCount);
  const double meanSquare = sumSquared / static_cast<double>(sampleCount);
  const double variance = meanSquare - (mean * mean);
  const float vrms = variance > 0.0 ? sqrtf(static_cast<float>(variance)) : 0.0F;

  return {vrms * channel.ampsPerVolt, static_cast<float>(mean), vrms};
}

void runZeroCalibration(bool persist) {
  Serial.println("=== Zero Calibration ===");
  Serial.println("Ensure all CT clamps have no conductor inside.");
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    const ChannelMeasurement measurement = readChannelMeasurement(gChannels[index], kZeroCalibrationWindowMs);
    gZeroOffsetVrms[index] = measurement.vrmsVoltage;
    Serial.printf(
        "%s GPIO%u zero_offset=%.3f A\n",
        gChannels[index].phase,
        gChannels[index].pin,
        gZeroOffsetVrms[index] * gChannels[index].ampsPerVolt);
  }
  if (persist) {
    savePersistedZeroCalibration();
    Serial.println("Zero calibration saved.");
  }
  Serial.println("========================");
}

void updateClimateMeasurement() {
  const uint32_t now = millis();
  if (now - gClimate.lastReadMs < kClimateReadIntervalMs) {
    return;
  }

  gClimate.lastReadMs = now;
  const float humidity = gDht.readHumidity();
  const float temperatureC = gDht.readTemperature();

  if (isnan(humidity) || isnan(temperatureC)) {
    return;
  }

  gClimate.hasValidSample = true;
  float correctedHumidity = humidity + gConfig.humidityOffsetPct;
  if (correctedHumidity < 0.0F) {
    correctedHumidity = 0.0F;
  }
  if (correctedHumidity > 100.0F) {
    correctedHumidity = 100.0F;
  }

  gClimate.humidityPercent = correctedHumidity;
  gClimate.temperatureC = temperatureC + gConfig.tempOffsetC;
  gClimate.lastSuccessMs = now;
}

bool climateSampleIsFresh() {
  return gClimate.hasValidSample && (millis() - gClimate.lastSuccessMs <= kClimateStaleAfterMs);
}

void updateMeasurements() {
  gRuntime.totalCurrentA = 0.0F;
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    const ChannelMeasurement measurement = readChannelMeasurement(gChannels[index], kSamplingWindowMs);
    const float zeroOffsetA = gZeroOffsetVrms[index] * gChannels[index].ampsPerVolt;
    const float correctedA = measurement.irms > zeroOffsetA ? measurement.irms - zeroOffsetA : 0.0F;
    gRuntime.phases[index] = {correctedA, measurement.irms, zeroOffsetA, measurement.meanVoltage};
    gRuntime.totalCurrentA += correctedA;
  }
  updateClimateMeasurement();
}

String readArg(const char* key) {
  return gServer.hasArg(key) ? gServer.arg(key) : String();
}

String topicFor(const char* sensorName) {
  String topic = gConfig.mqttBaseTopic;
  if (!topic.endsWith("/")) {
    topic += "/";
  }
  topic += sensorName;
  return topic;
}

void publishFloat(const String& topic, float value, uint8_t precision) {
  char payload[24];
  dtostrf(value, 0, precision, payload);
  gMqtt.publish(topic.c_str(), payload, true);
}

void publishMqttMeasurements() {
  if (!gMqtt.connected()) {
    return;
  }

  publishFloat(topicFor(gConfig.sensorL1), gRuntime.phases[0].currentA, 3);
  publishFloat(topicFor(gConfig.sensorL2), gRuntime.phases[1].currentA, 3);
  publishFloat(topicFor(gConfig.sensorL3), gRuntime.phases[2].currentA, 3);
  publishFloat(topicFor("total_current"), gRuntime.totalCurrentA, 3);

  if (climateSampleIsFresh()) {
    publishFloat(topicFor(gConfig.sensorTemp), gClimate.temperatureC, 1);
    publishFloat(topicFor(gConfig.sensorHumidity), gClimate.humidityPercent, 1);
  }

  if (hasValidSystemTime()) {
    publishFloat(topicFor("time/epoch"), static_cast<float>(time(nullptr)), 0);
  }
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  String topicText(topic);
  String body;
  for (unsigned int i = 0; i < length; ++i) {
    body += static_cast<char>(payload[i]);
  }

  const String commandTopic = topicFor("cmd");
  const String zeroTopic = topicFor("cmd/zero");
  const String timeTopic = topicFor("cmd/time");

  if (topicText == timeTopic) {
    if (!applyEpochFromMqttPayload(body)) {
      Serial.printf("Invalid MQTT time payload: %s\n", body.c_str());
    }
    return;
  }

  if (topicText == commandTopic || topicText == zeroTopic) {
    body.toLowerCase();
    if (body == "zero" || body == "zero_calibrate" || body == "calibrate_zero") {
      runZeroCalibration(true);
    } else if (body == "ntp" || body == "sync_time" || body == "sync_time_ntp") {
      syncTimeFromNtp();
    } else if (body.startsWith("time") || body.startsWith("epoch") || body.startsWith("unix") || body.startsWith("ts")) {
      if (!applyEpochFromMqttPayload(body)) {
        Serial.printf("Invalid MQTT time command payload: %s\n", body.c_str());
      }
    }
  }
}

void configureMqttClient() {
  gMqtt.setServer(gConfig.mqttServer, gConfig.mqttPort);
  gMqtt.setCallback(mqttCallback);
}

String mqttClientId() {
  String id = gConfig.hostname;
  id += "-";
  id += String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
  return id;
}

void maintainMqttConnection() {
  if (strlen(gConfig.mqttServer) == 0) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (gMqtt.connected()) {
    gMqtt.loop();
    return;
  }

  const uint32_t now = millis();
  if (now - gLastMqttReconnectMs < kMqttReconnectIntervalMs) {
    return;
  }
  gLastMqttReconnectMs = now;

  bool connected = false;
  if (strlen(gConfig.mqttUser) > 0) {
    connected = gMqtt.connect(mqttClientId().c_str(), gConfig.mqttUser, gConfig.mqttPassword);
  } else {
    connected = gMqtt.connect(mqttClientId().c_str());
  }

  if (connected) {
    gMqtt.subscribe(topicFor("cmd").c_str());
    gMqtt.subscribe(topicFor("cmd/zero").c_str());
    gMqtt.subscribe(topicFor("cmd/time").c_str());
  }
}

void printSerialReport() {
  Serial.println("=== Current Report (A RMS) ===");
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    Serial.printf(
        "%s GPIO%u: %.3f A raw=%.3f A zero=%.3f A (mean=%.3f V)\n",
        gChannels[index].phase,
        gChannels[index].pin,
        gRuntime.phases[index].currentA,
        gRuntime.phases[index].rawCurrentA,
        gRuntime.phases[index].zeroOffsetA,
        gRuntime.phases[index].meanVoltage);
  }
  Serial.printf("total_current=%.3f A\n", gRuntime.totalCurrentA);
  Serial.printf("free_heap=%u bytes\n", ESP.getFreeHeap());
  Serial.println("==============================");

  Serial.println("=== Maintenance Shaft Climate ===");
  if (climateSampleIsFresh()) {
    Serial.printf("DHT11 GPIO%u: temperature=%.1f C, humidity=%.1f %%\n", kDhtPin, gClimate.temperatureC, gClimate.humidityPercent);
  } else if (gClimate.hasValidSample) {
    Serial.printf("DHT11 GPIO%u: stale sample age=%lu ms\n", kDhtPin, millis() - gClimate.lastSuccessMs);
  } else {
    Serial.printf("DHT11 GPIO%u: waiting for first valid sample\n", kDhtPin);
  }
  Serial.println("=================================");
}

String htmlEscape(const String& value) {
  String out = value;
  out.replace("&", "&amp;");
  out.replace("<", "&lt;");
  out.replace(">", "&gt;");
  out.replace("\"", "&quot;");
  return out;
}

String getCookieValue(const String& cookieHeader, const String& key) {
  const String token = key + "=";
  const int start = cookieHeader.indexOf(token);
  if (start < 0) {
    return String();
  }
  const int valueStart = start + token.length();
  int valueEnd = cookieHeader.indexOf(';', valueStart);
  if (valueEnd < 0) {
    valueEnd = cookieHeader.length();
  }
  return cookieHeader.substring(valueStart, valueEnd);
}

bool isAuthenticated() {
  if (gSessionToken.isEmpty()) {
    return false;
  }
  if (millis() > gSessionExpiresMs) {
    return false;
  }
  if (!gServer.hasHeader("Cookie")) {
    return false;
  }
  const String sid = getCookieValue(gServer.header("Cookie"), "sid");
  return sid == gSessionToken;
}

void createSession() {
  gSessionToken = String(static_cast<uint32_t>(esp_random()), HEX) + String(static_cast<uint32_t>(esp_random()), HEX);
  gSessionExpiresMs = millis() + kSessionLifetimeMs;
}

void requireAuthOrReject() {
  if (!isAuthenticated()) {
    gServer.send(403, "text/plain", "Forbidden");
  }
}

String dashboardHtml(bool authenticated) {
  String html;
  html.reserve(14000);
  html += "<html><head><meta charset='utf-8'/>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'/>";
  html += "<title>X42MeasuringStation :: " + htmlEscape(gConfig.hostname) + "</title>";
  html += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css'/>";
  html += "<style>";
  html += ":root{--pico-font-size:95%;}";
  html += "body{padding-top:1rem;padding-bottom:2rem;}";
  html += "main.container{max-width:1100px;}";
  html += ".x42-header{display:flex;justify-content:space-between;align-items:flex-start;gap:1rem;flex-wrap:wrap;}";
  html += ".x42-muted{opacity:.75;font-size:.9rem;}";
  html += ".x42-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:1rem;}";
  html += ".x42-card h3{margin-bottom:.35rem;}";
  html += ".x42-value{font-size:1.45rem;font-weight:700;line-height:1.2;}";
  html += "</style></head><body><main class='container'>";

  String connectionLabel;
  if (!gIsApMode) {
    connectionLabel = "Connected to ";
    connectionLabel += htmlEscape(gConfig.wifiSsid);
  } else if (strlen(gConfig.wifiSsid) == 0) {
    connectionLabel = "Hotspot enabled because no SSID is configured.";
  } else {
    connectionLabel = "Hotspot enabled because configured SSID ";
    connectionLabel += htmlEscape(gConfig.wifiSsid);
    connectionLabel += " is not reachable or password is wrong.";
  }

  html += "<header class='x42-header'>";
  html += "<div><h1>X42MeasuringStation</h1><p class='x42-muted'>Realtime Monitoring and Device Control from X42.icu powerd by Daniel Hofer.</p></div>";
  html += "<article><strong>WLAN-Info</strong><br/>" + connectionLabel + "</article>";
  html += "</header>";

  html += "<section><h2>Live Values</h2><div class='x42-grid'>";
  html += "<article class='x42-card'><h3>L1 Current</h3><div class='x42-value'>" + String(gRuntime.phases[0].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>L2 Current</h3><div class='x42-value'>" + String(gRuntime.phases[1].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>L3 Current</h3><div class='x42-value'>" + String(gRuntime.phases[2].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>Total Current</h3><div class='x42-value'>" + String(gRuntime.totalCurrentA, 3) + " A</div></article>";
  if (climateSampleIsFresh()) {
    html += "<article class='x42-card'><h3>Temperature</h3><div class='x42-value'>" + String(gClimate.temperatureC, 1) + " C</div></article>";
    html += "<article class='x42-card'><h3>Humidity</h3><div class='x42-value'>" + String(gClimate.humidityPercent, 1) + " %</div></article>";
  } else {
    html += "<article class='x42-card'><h3>Climate</h3><div class='x42-value'>waiting/stale</div></article>";
  }
  html += "</div></section>";

  html += "<section><h2>Access</h2>";
  if (!authenticated) {
    html += "<article><form method='post' action='/login'>";
    html += "<div class='grid'><label>User<input name='username' autocomplete='username'/></label>";
    html += "<label>Password<input type='password' name='password' autocomplete='current-password'/></label></div>";
    html += "<button type='submit'>Login</button></form>";
    html += "<p class='x42-muted'>Login enables configuration, calibration, reboot and OTA actions.</p>";
    html += "<a href='/status.json' role='button' class='secondary'>Open Status JSON</a></article></section>";
    html += "</main></body></html>";
    return html;
  }
  html += "<article><p>Admin session is active. Advanced settings and actions are available below.</p>";
  html += "</article></section>";

  html += "<section><h2>Configuration</h2><article><form method='post' action='/save-config'>";
  html += "<h3>WLAN</h3>";
  html += "<div class='grid'><label>WiFi SSID<input name='wifi_ssid' value='" + htmlEscape(gConfig.wifiSsid) + "'/></label>";
  html += "<label>WiFi Password<input type='password' name='wifi_pw' placeholder='(unchanged if empty)' autocomplete='new-password'/></label></div>";
  html += "<label>Hostname<input name='hostname' value='" + htmlEscape(gConfig.hostname) + "'/></label>";

  html += "<h3>MQTT</h3>";
  html += "<div class='grid'><label>MQTT Server<input name='mqtt_srv' value='" + htmlEscape(gConfig.mqttServer) + "'/></label>";
  html += "<label>MQTT Port<input name='mqtt_port' type='number' value='" + String(gConfig.mqttPort) + "'/></label></div>";
  html += "<div class='grid'><label>MQTT User<input name='mqtt_usr' value='" + htmlEscape(gConfig.mqttUser) + "'/></label>";
  html += "<label>MQTT Password<input type='password' name='mqtt_pw' placeholder='(unchanged if empty)' autocomplete='new-password'/></label></div>";
  html += "<div class='grid'><label>MQTT Base Topic<input name='mqtt_base' value='" + htmlEscape(gConfig.mqttBaseTopic) + "'/></label>";
  html += "<label>Publish Interval (s)<input name='pub_int' type='number' min='1' max='3600' value='" + String(gConfig.publishIntervalSec) + "'/></label></div>";
  html += "<h3>MQTT Topics</h3>";
  html += "<div class='grid'><label>L1<input name='name_l1' value='" + htmlEscape(gConfig.sensorL1) + "'/></label>";
  html += "<label>L2<input name='name_l2' value='" + htmlEscape(gConfig.sensorL2) + "'/></label></div>";
  html += "<div class='grid'><label>L3<input name='name_l3' value='" + htmlEscape(gConfig.sensorL3) + "'/></label>";
  html += "<label>Temperature<input name='name_t' value='" + htmlEscape(gConfig.sensorTemp) + "'/></label></div>";
  html += "<label>Humidity<input name='name_h' value='" + htmlEscape(gConfig.sensorHumidity) + "'/></label>";

  html += "<h3>Timezone</h3>";
  html += "<div class='grid'><label>NTP Server<input name='ntp_srv' value='" + htmlEscape(gConfig.ntpServer) + "'/></label>";
  html += "<label>Timezone Offset (min)<input name='tz_min' type='number' min='-720' max='840' value='" + String(gConfig.timezoneOffsetMinutes) + "'/></label></div>";

  html += "<h3>Offsets</h3>";
  html += "<div class='grid'><label>Temperature Offset (C)<input name='temp_off' type='number' step='0.1' min='-20' max='20' value='" + String(gConfig.tempOffsetC, 1) + "'/></label>";
  html += "<label>Humidity Offset (%)<input name='hum_off' type='number' step='0.1' min='-50' max='50' value='" + String(gConfig.humidityOffsetPct, 1) + "'/></label></div>";

  html += "<button type='submit'>Save Configuration</button></form></article></section>";

  html += "<section><h2>Security</h2><article><form method='post' action='/change-admin-user'>";
  html += "<div class='grid'><label>Current Password<input type='password' name='current_pw_user'/></label>";
  html += "<label>New Admin User<input name='new_user' value='" + htmlEscape(gConfig.adminUser) + "'/></label>";
  html += "<label>Confirm Admin User<input name='confirm_user' value='" + htmlEscape(gConfig.adminUser) + "'/></label></div>";
  html += "<button type='submit'>Change Admin User</button></form>";
  html += "<form method='post' action='/change-password'>";
  html += "<div class='grid'><label>Current Password<input type='password' name='current_pw'/></label>";
  html += "<label>New Password<input type='password' name='new_pw'/></label>";
  html += "<label>Confirm New Password<input type='password' name='confirm_pw'/></label></div>";
  html += "<button type='submit'>Change Password</button></form></article></section>";

  html += "<section><h2>Actions</h2><article>";
  html += "<div class='grid'>";
  html += "<form method='post' action='/run-zero'><button type='submit'>Run Zero Calibration</button></form>";
  html += "<form method='post' action='/reboot'><button type='submit' class='contrast'>Save and Reboot</button></form>";
  html += "</div>";
  html += "<div class='grid'><a href='/ota' role='button'>OTA Update</a><a href='/status.json' role='button' class='secondary'>Status JSON</a></div>";
  html += "</article></section>";

  html += "<section><article><form method='post' action='/logout'><button type='submit' class='secondary'>Logout</button></form></article></section>";

  html += "</main></body></html>";
  return html;
}

String loginHtml(const String& message) {
  String html;
  html.reserve(2200);
  html += "<html><head><meta charset='utf-8'/><meta name='viewport' content='width=device-width,initial-scale=1'/><title>X42MeasuringStation :: " + htmlEscape(gConfig.hostname) + "</title>";
  html += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css'/>";
  html += "</head><body><main class='container'><article>";
  html += "<h1>X42MeasuringStation Login</h1>";
  if (message.length() > 0) {
    html += "<p><mark>" + htmlEscape(message) + "</mark></p>";
  }
  html += "<form method='post' action='/login'>";
  html += "<label>User<input name='username' autocomplete='username'/></label>";
  html += "<label>Password<input type='password' name='password' autocomplete='current-password'/></label>";
  html += "<button type='submit'>Login</button></form>";
  html += "</article></main></body></html>";
  return html;
}

void handleRoot() {
  gServer.send(200, "text/html", dashboardHtml(isAuthenticated()));
}

void handleLogin() {
  const String username = readArg("username");
  const String password = readArg("password");
  if (username == String(gConfig.adminUser) && password == String(gConfig.adminPassword)) {
    createSession();
    gServer.sendHeader("Set-Cookie", "sid=" + gSessionToken + "; Path=/; HttpOnly");
    gServer.sendHeader("Location", "/");
    gServer.send(302, "text/plain", "Logged in");
    return;
  }
  gServer.send(200, "text/html", loginHtml("Invalid user or password"));
}

void handleChangeAdminUser() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }

  const String currentPw = readArg("current_pw_user");
  const String newUser = readArg("new_user");
  const String confirmUser = readArg("confirm_user");

  if (currentPw != String(gConfig.adminPassword)) {
    gServer.send(400, "text/plain", "Current password is invalid.");
    return;
  }
  if (newUser.length() < 3) {
    gServer.send(400, "text/plain", "New admin user must be at least 3 characters.");
    return;
  }
  if (newUser != confirmUser) {
    gServer.send(400, "text/plain", "Admin user confirmation does not match.");
    return;
  }

  copyString(gConfig.adminUser, sizeof(gConfig.adminUser), newUser);
  saveConfig();
  gServer.send(200, "text/plain", "Admin user changed successfully.");
}

void handleLogout() {
  gSessionToken = "";
  gServer.sendHeader("Set-Cookie", "sid=; Max-Age=0; Path=/");
  gServer.sendHeader("Location", "/");
  gServer.send(302, "text/plain", "Logged out");
}

void handleSaveConfig() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }

  copyString(gConfig.wifiSsid, sizeof(gConfig.wifiSsid), readArg("wifi_ssid"));
  const String wifiPwArg = readArg("wifi_pw");
  if (wifiPwArg.length() > 0) {
    copyString(gConfig.wifiPassword, sizeof(gConfig.wifiPassword), wifiPwArg);
  }
  copyString(gConfig.hostname, sizeof(gConfig.hostname), readArg("hostname"));
  copyString(gConfig.mqttServer, sizeof(gConfig.mqttServer), readArg("mqtt_srv"));
  copyString(gConfig.mqttUser, sizeof(gConfig.mqttUser), readArg("mqtt_usr"));
  const String mqttPwArg = readArg("mqtt_pw");
  if (mqttPwArg.length() > 0) {
    copyString(gConfig.mqttPassword, sizeof(gConfig.mqttPassword), mqttPwArg);
  }
  copyString(gConfig.mqttBaseTopic, sizeof(gConfig.mqttBaseTopic), readArg("mqtt_base"));
  copyString(gConfig.ntpServer, sizeof(gConfig.ntpServer), readArg("ntp_srv"));
  copyString(gConfig.sensorL1, sizeof(gConfig.sensorL1), readArg("name_l1"));
  copyString(gConfig.sensorL2, sizeof(gConfig.sensorL2), readArg("name_l2"));
  copyString(gConfig.sensorL3, sizeof(gConfig.sensorL3), readArg("name_l3"));
  copyString(gConfig.sensorTemp, sizeof(gConfig.sensorTemp), readArg("name_t"));
  copyString(gConfig.sensorHumidity, sizeof(gConfig.sensorHumidity), readArg("name_h"));

  const int mqttPort = readArg("mqtt_port").toInt();
  gConfig.mqttPort = mqttPort > 0 ? static_cast<uint16_t>(mqttPort) : 1883;

  int timezoneOffsetMin = readArg("tz_min").toInt();
  if (timezoneOffsetMin < -720) {
    timezoneOffsetMin = -720;
  }
  if (timezoneOffsetMin > 840) {
    timezoneOffsetMin = 840;
  }
  gConfig.timezoneOffsetMinutes = static_cast<int16_t>(timezoneOffsetMin);

  float tempOffset = readArg("temp_off").toFloat();
  if (tempOffset < -20.0F) {
    tempOffset = -20.0F;
  }
  if (tempOffset > 20.0F) {
    tempOffset = 20.0F;
  }
  gConfig.tempOffsetC = tempOffset;

  float humidityOffset = readArg("hum_off").toFloat();
  if (humidityOffset < -50.0F) {
    humidityOffset = -50.0F;
  }
  if (humidityOffset > 50.0F) {
    humidityOffset = 50.0F;
  }
  gConfig.humidityOffsetPct = humidityOffset;

  int publishInterval = readArg("pub_int").toInt();
  if (publishInterval < 1) {
    publishInterval = 1;
  }
  if (publishInterval > 3600) {
    publishInterval = 3600;
  }
  gConfig.publishIntervalSec = static_cast<uint16_t>(publishInterval);

  saveConfig();
  configureMqttClient();
  gMqtt.disconnect();
  syncTimeFromNtp();

  gServer.send(200, "text/plain", "Configuration saved. Use reboot action to apply WiFi/hostname changes.");
}

void handleChangePassword() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }

  const String currentPw = readArg("current_pw");
  const String newPw = readArg("new_pw");
  const String confirmPw = readArg("confirm_pw");

  if (currentPw != String(gConfig.adminPassword)) {
    gServer.send(400, "text/plain", "Current password is invalid.");
    return;
  }
  if (newPw.length() < 6) {
    gServer.send(400, "text/plain", "New password must be at least 6 characters.");
    return;
  }
  if (newPw != confirmPw) {
    gServer.send(400, "text/plain", "New password confirmation does not match.");
    return;
  }

  copyString(gConfig.adminPassword, sizeof(gConfig.adminPassword), newPw);
  saveConfig();
  gServer.send(200, "text/plain", "Password changed successfully.");
}

void handleStatusJson() {
  updateMeasurements();
  String json = "{";
  json += "\"hostname\":\"" + String(gConfig.hostname) + "\",";
  json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"mqtt_connected\":" + String(gMqtt.connected() ? "true" : "false") + ",";
  json += "\"time\":{";
  json += "\"valid\":" + String(hasValidSystemTime() ? "true" : "false") + ",";
  json += "\"epoch\":" + String(static_cast<unsigned long>(time(nullptr))) + ",";
  json += "\"utc\":\"" + epochToUtcIso(time(nullptr)) + "\",";
  json += "\"last_sync_source\":\"" + gLastTimeSyncSource + "\",";
  json += "\"last_sync_epoch\":" + String(static_cast<unsigned long>(gLastTimeSyncEpoch)) + ",";
  json += "\"last_sync_utc\":\"" + epochToUtcIso(gLastTimeSyncEpoch) + "\"";
  json += "},";
  json += "\"currents\":{";
  json += "\"L1\":" + String(gRuntime.phases[0].currentA, 3) + ",";
  json += "\"L2\":" + String(gRuntime.phases[1].currentA, 3) + ",";
  json += "\"L3\":" + String(gRuntime.phases[2].currentA, 3) + ",";
  json += "\"total\":" + String(gRuntime.totalCurrentA, 3);
  json += "},";
  json += "\"climate\":{";
  if (climateSampleIsFresh()) {
    json += "\"temperature_c\":" + String(gClimate.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(gClimate.humidityPercent, 1) + ",";
    json += "\"temperature_offset_c\":" + String(gConfig.tempOffsetC, 1) + ",";
    json += "\"humidity_offset_percent\":" + String(gConfig.humidityOffsetPct, 1) + ",";
    json += "\"fresh\":true";
  } else {
    json += "\"fresh\":false";
  }
  json += "}";
  json += "}";
  gServer.send(200, "application/json", json);
}

void handleRunZero() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }
  runZeroCalibration(true);
  gServer.send(200, "text/plain", "Zero calibration completed and saved.");
}

void handleReboot() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }
  gServer.send(200, "text/plain", "Rebooting...");
  delay(200);
  ESP.restart();
}

void handleOtaPage() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }

  String html;
  html += "<html><body><h1>OTA Update</h1>";
  html += "<form method='post' action='/ota' enctype='multipart/form-data'>";
  html += "<input type='file' name='firmware'/>";
  html += "<button type='submit'>Upload</button></form>";
  html += "<a href='/'>Back</a></body></html>";
  gServer.send(200, "text/html", html);
}

void configureWebServer() {
  const char* headerKeys[] = {"Cookie"};
  gServer.collectHeaders(headerKeys, 1);

  gServer.on("/", HTTP_GET, handleRoot);
  gServer.on("/login", HTTP_POST, handleLogin);
  gServer.on("/logout", HTTP_POST, handleLogout);
  gServer.on("/save-config", HTTP_POST, handleSaveConfig);
  gServer.on("/change-admin-user", HTTP_POST, handleChangeAdminUser);
  gServer.on("/change-password", HTTP_POST, handleChangePassword);
  gServer.on("/status.json", HTTP_GET, handleStatusJson);
  gServer.on("/run-zero", HTTP_POST, handleRunZero);
  gServer.on("/reboot", HTTP_POST, handleReboot);
  gServer.on("/ota", HTTP_GET, handleOtaPage);

  gServer.on(
      "/ota",
      HTTP_POST,
      []() {
        if (!isAuthenticated()) {
          requireAuthOrReject();
          return;
        }
        const bool ok = !Update.hasError();
        if (ok) {
          markPendingOtaUpdate();
        }
        gServer.send(200, "text/plain", ok ? "OTA success. Rebooting..." : "OTA failed.");
        delay(300);
        if (ok) {
          ESP.restart();
        }
      },
      []() {
        HTTPUpload& upload = gServer.upload();
        if (upload.status == UPLOAD_FILE_START) {
          Update.begin(UPDATE_SIZE_UNKNOWN);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          Update.write(upload.buf, upload.currentSize);
        } else if (upload.status == UPLOAD_FILE_END) {
          Update.end(true);
        }
      });

  gServer.begin();
}

void configureArduinoOta() {
  ArduinoOTA.setHostname(gConfig.hostname);
  ArduinoOTA.begin();
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(gConfig.hostname);

  if (strlen(gConfig.wifiSsid) == 0) {
    gIsApMode = true;
  } else {
    Serial.printf("Connecting to WiFi SSID: %s\n", gConfig.wifiSsid);
    WiFi.begin(gConfig.wifiSsid, gConfig.wifiPassword);
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < kWifiConnectTimeoutMs) {
      delay(300);
      Serial.print('.');
    }
    Serial.println();
    gIsApMode = WiFi.status() != WL_CONNECTED;
  }

  if (gIsApMode) {
    WiFi.mode(WIFI_AP);
    String apSsid = String(gConfig.hostname) + "-setup";
    WiFi.softAP(apSsid.c_str());
    Serial.printf("AP mode active, SSID: %s, IP: %s\n", apSsid.c_str(), WiFi.softAPIP().toString().c_str());
  } else {
    Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    syncTimeFromNtp();
  }
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(Serial.read());
    if (command == 'z' || command == 'Z') {
      runZeroCalibration(true);
    } else if (command == 'h' || command == 'H' || command == '?') {
      Serial.println("Commands: z = refresh zero calibration, h = help");
    }
  }
}

void setup() {
  Serial.begin(kBaudRate);
  delay(200);

  const bool pendingOtaUpdate = consumePendingOtaUpdateFlag();
  const ConfigBootAction configBootAction = loadConfig(pendingOtaUpdate);
  if (configBootAction == ConfigBootAction::PersistDefaults) {
    saveConfig();
    Serial.println("Applied build defaults from device_config.h after firmware update based on flash policy.");
  } else if (configBootAction == ConfigBootAction::PersistLoaded) {
    saveConfig();
    Serial.println("Kept existing persisted config and updated firmware config revision.");
  }
  printChipInfo();

  analogReadResolution(12);
  for (const CurrentChannel& channel : gChannels) {
    analogSetPinAttenuation(channel.pin, ADC_11db);
  }

  loadDefaultZeroCalibration();
  if (loadPersistedZeroCalibration()) {
    Serial.println("Loaded saved zero calibration.");
  } else {
    Serial.println("Using built-in zero calibration defaults.");
  }

  gDht.begin();
  connectWifi();
  configureMqttClient();
  configureWebServer();
  configureArduinoOta();

  Serial.println("System ready: web server, MQTT, OTA, current sensing, DHT11");
  Serial.println("Serial command 'z' triggers zero calibration.");

  gRuntime.lastReportMs = millis();
  gLastMqttPublishMs = millis();
}

void loop() {
  handleSerialCommands();
  gServer.handleClient();
  ArduinoOTA.handle();

  maintainMqttConnection();

  const uint32_t now = millis();
  const uint32_t intervalMs = static_cast<uint32_t>(gConfig.publishIntervalSec) * 1000UL;
  if (now - gRuntime.lastReportMs >= intervalMs) {
    gRuntime.lastReportMs = now;
    updateMeasurements();
    printSerialReport();
  }

  if (now - gLastMqttPublishMs >= intervalMs) {
    gLastMqttPublishMs = now;
    publishMqttMeasurements();
  }
}
