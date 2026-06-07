#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
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

#ifndef X42_CFG_ADMIN_USER
#define X42_CFG_ADMIN_USER "admin"
#endif

#ifndef X42_CFG_ADMIN_PASSWORD
#define X42_CFG_ADMIN_PASSWORD "admin1234"
#endif

#ifndef X42_CFG_WIFI_SSID
#define X42_CFG_WIFI_SSID ""
#endif

#ifndef X42_CFG_WIFI_PASSWORD
#define X42_CFG_WIFI_PASSWORD ""
#endif

#ifndef X42_CFG_HOSTNAME
#define X42_CFG_HOSTNAME "x42-whirlpool-station"
#endif

#ifndef X42_CFG_MQTT_SERVER
#define X42_CFG_MQTT_SERVER ""
#endif

#ifndef X42_CFG_MQTT_PORT
#define X42_CFG_MQTT_PORT 1883
#endif

#ifndef X42_CFG_MQTT_USER
#define X42_CFG_MQTT_USER ""
#endif

#ifndef X42_CFG_MQTT_PASSWORD
#define X42_CFG_MQTT_PASSWORD ""
#endif

#ifndef X42_CFG_MQTT_BASE_TOPIC
#define X42_CFG_MQTT_BASE_TOPIC "whirlpool/station"
#endif

#ifndef X42_CFG_NTP_SERVER
#define X42_CFG_NTP_SERVER "pool.ntp.org"
#endif

#ifndef X42_CFG_TZ_OFFSET_MIN
#define X42_CFG_TZ_OFFSET_MIN 60
#endif

#ifndef X42_CFG_APPLY_ON_USB_FLASH
#define X42_CFG_APPLY_ON_USB_FLASH 1
#endif

#ifndef X42_CFG_APPLY_ON_OTA_FLASH
#define X42_CFG_APPLY_ON_OTA_FLASH 1
#endif

#ifndef X42_CFG_PUBLISH_INTERVAL_SEC
#define X42_CFG_PUBLISH_INTERVAL_SEC 5
#endif

#ifndef X42_CFG_SENSOR_L1
#define X42_CFG_SENSOR_L1 "current_l1"
#endif

#ifndef X42_CFG_SENSOR_L2
#define X42_CFG_SENSOR_L2 "current_l2"
#endif

#ifndef X42_CFG_SENSOR_L3
#define X42_CFG_SENSOR_L3 "current_l3"
#endif

#ifndef X42_CFG_SENSOR_TEMP
#define X42_CFG_SENSOR_TEMP "temp_outdoor"
#endif

#ifndef X42_CFG_SENSOR_HUMIDITY
#define X42_CFG_SENSOR_HUMIDITY "humidity_outdoor"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TOTAL
#define X42_CFG_SENSOR_ENERGY_TOTAL "energy_total_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TODAY
#define X42_CFG_SENSOR_ENERGY_TODAY "energy_today_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_SEASON
#define X42_CFG_SENSOR_ENERGY_SEASON "energy_season_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_LAST_SEASON
#define X42_CFG_SENSOR_ENERGY_LAST_SEASON "energy_last_season_kwh"
#endif

#ifndef X42_CFG_PH_NEUTRAL_VOLTAGE
#define X42_CFG_PH_NEUTRAL_VOLTAGE 2.50F
#endif

#ifndef X42_CFG_PH_SLOPE_PER_VOLT
#define X42_CFG_PH_SLOPE_PER_VOLT 3.00F
#endif

#ifndef X42_GITHUB_RELEASE
#define X42_GITHUB_RELEASE "dev"
#endif

#ifndef X42_GIT_BRANCH
#define X42_GIT_BRANCH "main"
#endif

#ifndef X42_CFG_HA_ENABLED
#define X42_CFG_HA_ENABLED 1
#endif

#ifndef X42_CFG_HA_NAME_L1
#define X42_CFG_HA_NAME_L1 "L1 Current"
#endif

#ifndef X42_CFG_HA_NAME_L2
#define X42_CFG_HA_NAME_L2 "L2 Current"
#endif

#ifndef X42_CFG_HA_NAME_L3
#define X42_CFG_HA_NAME_L3 "L3 Current"
#endif

#ifndef X42_CFG_HA_NAME_TOTAL
#define X42_CFG_HA_NAME_TOTAL "Total Current"
#endif

#ifndef X42_CFG_HA_NAME_TEMP
#define X42_CFG_HA_NAME_TEMP "Temperature"
#endif

#ifndef X42_CFG_HA_NAME_HUMIDITY
#define X42_CFG_HA_NAME_HUMIDITY "Humidity"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TOTAL
#define X42_CFG_SENSOR_ENERGY_TOTAL "energy_total_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TODAY
#define X42_CFG_SENSOR_ENERGY_TODAY "energy_today_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_SEASON
#define X42_CFG_SENSOR_ENERGY_SEASON "energy_season_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_LAST_SEASON
#define X42_CFG_SENSOR_ENERGY_LAST_SEASON "energy_last_season_kwh"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_TOTAL
#define X42_CFG_HA_NAME_ENERGY_TOTAL "Total Energy"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_TODAY
#define X42_CFG_HA_NAME_ENERGY_TODAY "Current Today"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_SEASON
#define X42_CFG_HA_NAME_ENERGY_SEASON "Energy This Season"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_LAST_SEASON
#define X42_CFG_HA_NAME_ENERGY_LAST_SEASON "Energy Last Season"
#endif

#ifndef X42_CFG_GAIN_L1
#define X42_CFG_GAIN_L1 0.778F
#endif

#ifndef X42_CFG_GAIN_L2
#define X42_CFG_GAIN_L2 0.778F
#endif

#ifndef X42_CFG_GAIN_L3
#define X42_CFG_GAIN_L3 0.863F
#endif

#ifndef X42_CFG_HA_NAME_L3
#define X42_CFG_HA_NAME_L3 "L3 Current"
#endif

#ifndef X42_CFG_HA_NAME_TOTAL
#define X42_CFG_HA_NAME_TOTAL "Total Current"
#endif

#ifndef X42_CFG_HA_NAME_TEMP
#define X42_CFG_HA_NAME_TEMP "Temperature"
#endif

#ifndef X42_CFG_HA_NAME_HUMIDITY
#define X42_CFG_HA_NAME_HUMIDITY "Humidity"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TOTAL
#define X42_CFG_SENSOR_ENERGY_TOTAL "energy_total_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_TODAY
#define X42_CFG_SENSOR_ENERGY_TODAY "energy_today_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_SEASON
#define X42_CFG_SENSOR_ENERGY_SEASON "energy_season_kwh"
#endif

#ifndef X42_CFG_SENSOR_ENERGY_LAST_SEASON
#define X42_CFG_SENSOR_ENERGY_LAST_SEASON "energy_last_season_kwh"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_TOTAL
#define X42_CFG_HA_NAME_ENERGY_TOTAL "Total Energy"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_TODAY
#define X42_CFG_HA_NAME_ENERGY_TODAY "Current Today"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_SEASON
#define X42_CFG_HA_NAME_ENERGY_SEASON "Energy This Season"
#endif

#ifndef X42_CFG_HA_NAME_ENERGY_LAST_SEASON
#define X42_CFG_HA_NAME_ENERGY_LAST_SEASON "Energy Last Season"
#endif

#ifndef X42_CFG_GAIN_L1
#define X42_CFG_GAIN_L1 0.778F
#endif

#ifndef X42_CFG_GAIN_L2
#define X42_CFG_GAIN_L2 0.778F
#endif

#ifndef X42_CFG_GAIN_L3
#define X42_CFG_GAIN_L3 0.863F
#endif

namespace {
constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kSamplingWindowMs = 400;
constexpr uint32_t kZeroCalibrationWindowMs = 2000;
constexpr uint32_t kClimateReadIntervalMs = 5000;
constexpr uint32_t kClimateStaleAfterMs = 30000;
constexpr uint32_t kMqttReconnectIntervalMs = 5000;
constexpr uint16_t kMqttPayloadBufferSize = 1024;
constexpr uint32_t kSessionLifetimeMs = 15UL * 60UL * 1000UL;
constexpr uint32_t kStartupWifiConnectWindowMs = 30000;
constexpr uint32_t kWifiRetryIntervalMs = 2UL * 60UL * 1000UL;
constexpr uint32_t kWifiRetryAttemptTimeoutMs = 10000;
constexpr uint32_t kRj45ReadIntervalMs = 5000;
constexpr uint32_t kDs18b20DiscoveryIntervalMs = 15000;
constexpr uint32_t kRadarPresenceStaleAfterMs = 10000;
constexpr uint32_t kNtpSyncTimeoutMs = 8000;
constexpr uint32_t kEnergyPersistIntervalMs = 60000;
constexpr uint32_t kMaxEnergyDeltaMs = 10UL * 60UL * 1000UL;
constexpr uint16_t kHttpPort = 80;
constexpr uint8_t kShaftDhtPin = 4;
constexpr uint8_t kShaftDhtType = DHT11;
constexpr uint8_t kOutdoorDhtPin = 25;
constexpr uint8_t kOutdoorDhtType = DHT11;
constexpr uint8_t kRj45PhAnalogPin = 33;
constexpr uint8_t kRj45RadarTxPin = 17;
constexpr uint8_t kRj45RadarRxPin = 16;
constexpr uint8_t kRj45SclPin = 22;
constexpr uint8_t kRj45SdaPin = 21;
constexpr uint8_t kLcdI2cAddress = 0x27;
constexpr uint8_t kLcdAltI2cAddress = 0x3F;
constexpr uint8_t kLcdColumns = 16;
constexpr uint8_t kLcdRows = 2;
constexpr uint32_t kLcdInitRetryIntervalMs = 10000;
constexpr uint8_t kRj45Ds18b20Pin = 27;
constexpr uint32_t kRadarSerialBaudRate = 256000;
constexpr float kAdcReferenceVoltage = 3.3F;
constexpr float kNominalVoltageV = 230.0F;
constexpr uint16_t kAdcMax = 4095;
constexpr time_t kValidEpochThreshold = 1700000000;
constexpr char kHomeAssistantDiscoveryPrefix[] = "homeassistant";

constexpr char kZeroNs[] = "sct013";
constexpr char kConfigNs[] = "appcfg";
constexpr char kEnergyNs[] = "energy";
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
  {"L1", 34, X42_CFG_GAIN_L1},
  {"L2", 35, X42_CFG_GAIN_L2},
  {"L3", 32, X42_CFG_GAIN_L3},
};

constexpr float kDefaultZeroOffsetVrms[] = {
  0.005F / X42_CFG_GAIN_L1,
  0.002F / X42_CFG_GAIN_L2,
  0.004F / X42_CFG_GAIN_L3,
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
  char sensorEnergyTotal[24];
  char sensorEnergyToday[24];
  char sensorEnergySeason[24];
  char sensorEnergyLastSeason[24];
  float gainL1;
  float gainL2;
  float gainL3;
  bool haEnabled;
  char haNameL1[48];
  char haNameL2[48];
  char haNameL3[48];
  char haNameTotal[48];
  char haNameTemp[48];
  char haNameHumidity[48];
  char haNameEnergyTotal[48];
  char haNameEnergyToday[48];
  char haNameEnergySeason[48];
  char haNameEnergyLastSeason[48];
};

struct ChannelMeasurement {
  float irms;
  float meanVoltage;
  float vrmsVoltage;
};

struct PhaseRuntime {
  float currentA;
  float powerW;
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

struct Rj45State {
  uint16_t phRaw;
  float phVoltage;
  bool i2cReady;
  bool radarSerialReady;
  bool radarFrameSeen;
  bool radarPresenceFresh;
  bool radarPersonDetected;
  uint8_t radarTargetState;
  bool ds18b20Detected;
  bool ds18b20Fresh;
  float ds18b20TempC;
  uint32_t lastReadMs;
  uint32_t lastRadarFrameMs;
  uint32_t lastRadarMotionDetectedMs;
  uint32_t lastDs18b20DiscoveryMs;
};

struct RuntimeState {
  PhaseRuntime phases[sizeof(gChannels) / sizeof(gChannels[0])];
  float totalCurrentA;
  float totalPowerW;
  uint32_t lastReportMs;
};

struct EnergyState {
  float totalKWh;
  float todayKWh;
  float seasonKWh;
  float lastSeasonKWh;
  int32_t localDayKey;
  uint32_t lastSampleMs;
  uint32_t lastPersistMs;
  bool initialized;
};

float gZeroOffsetVrms[sizeof(gChannels) / sizeof(gChannels[0])] = {};
DeviceConfig gConfig = {};
RuntimeState gRuntime = {};
ClimateState gShaftClimate = {false, 0.0F, 0.0F, 0, 0};
ClimateState gOutdoorClimate = {false, 0.0F, 0.0F, 0, 0};
Rj45State gRj45 = {0, 0.0F, false, false, false, false, false, 0, false, false, NAN, 0, 0, 0, 0};
uint8_t gRadarFrameBuffer[96] = {0};
size_t gRadarFrameBufferLen = 0;
EnergyState gEnergy = {0.0F, 0.0F, 0.0F, 0.0F, -1, 0, 0, false};

DHT gShaftDht(kShaftDhtPin, kShaftDhtType);
DHT gOutdoorDht(kOutdoorDhtPin, kOutdoorDhtType);
OneWire gOneWire(kRj45Ds18b20Pin);
DallasTemperature gDs18b20(&gOneWire);
LiquidCrystal_I2C* gLcd = nullptr;
Preferences gPreferences;
WebServer gServer(kHttpPort);
WiFiClient gWifiClient;
PubSubClient gMqtt(gWifiClient);

String gSessionToken;
uint32_t gSessionExpiresMs = 0;
uint32_t gLastMqttReconnectMs = 0;
uint32_t gLastMqttPublishMs = 0;
bool gIsApMode = false;
uint32_t gLastWifiRetryAttemptMs = 0;
bool gWifiRetryInProgress = false;
uint32_t gWifiRetryStartedMs = 0;
String gLastTimeSyncSource;
time_t gLastTimeSyncEpoch = 0;
bool gLcdReady = false;
uint8_t gLcdDetectedAddress = 0;
uint32_t gLastLcdInitAttemptMs = 0;
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

float sanitizeGain(float value, float fallback) {
  if (!isfinite(value) || value < 0.05F || value > 5.0F) {
    return fallback;
  }
  return value;
}

void applyConfiguredCurrentGains() {
  gConfig.gainL1 = sanitizeGain(gConfig.gainL1, X42_CFG_GAIN_L1);
  gConfig.gainL2 = sanitizeGain(gConfig.gainL2, X42_CFG_GAIN_L2);
  gConfig.gainL3 = sanitizeGain(gConfig.gainL3, X42_CFG_GAIN_L3);

  gChannels[0].ampsPerVolt = gConfig.gainL1;
  gChannels[1].ampsPerVolt = gConfig.gainL2;
  gChannels[2].ampsPerVolt = gConfig.gainL3;
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
  copyString(cfg.sensorEnergyTotal, sizeof(cfg.sensorEnergyTotal), X42_CFG_SENSOR_ENERGY_TOTAL);
  copyString(cfg.sensorEnergyToday, sizeof(cfg.sensorEnergyToday), X42_CFG_SENSOR_ENERGY_TODAY);
  copyString(cfg.sensorEnergySeason, sizeof(cfg.sensorEnergySeason), X42_CFG_SENSOR_ENERGY_SEASON);
  copyString(cfg.sensorEnergyLastSeason, sizeof(cfg.sensorEnergyLastSeason), X42_CFG_SENSOR_ENERGY_LAST_SEASON);
  cfg.gainL1 = X42_CFG_GAIN_L1;
  cfg.gainL2 = X42_CFG_GAIN_L2;
  cfg.gainL3 = X42_CFG_GAIN_L3;
  cfg.haEnabled = X42_CFG_HA_ENABLED != 0;
  copyString(cfg.haNameL1, sizeof(cfg.haNameL1), X42_CFG_HA_NAME_L1);
  copyString(cfg.haNameL2, sizeof(cfg.haNameL2), X42_CFG_HA_NAME_L2);
  copyString(cfg.haNameL3, sizeof(cfg.haNameL3), X42_CFG_HA_NAME_L3);
  copyString(cfg.haNameTotal, sizeof(cfg.haNameTotal), X42_CFG_HA_NAME_TOTAL);
  copyString(cfg.haNameTemp, sizeof(cfg.haNameTemp), X42_CFG_HA_NAME_TEMP);
  copyString(cfg.haNameHumidity, sizeof(cfg.haNameHumidity), X42_CFG_HA_NAME_HUMIDITY);
  copyString(cfg.haNameEnergyTotal, sizeof(cfg.haNameEnergyTotal), X42_CFG_HA_NAME_ENERGY_TOTAL);
  copyString(cfg.haNameEnergyToday, sizeof(cfg.haNameEnergyToday), X42_CFG_HA_NAME_ENERGY_TODAY);
  copyString(cfg.haNameEnergySeason, sizeof(cfg.haNameEnergySeason), X42_CFG_HA_NAME_ENERGY_SEASON);
  copyString(cfg.haNameEnergyLastSeason, sizeof(cfg.haNameEnergyLastSeason), X42_CFG_HA_NAME_ENERGY_LAST_SEASON);
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
  bool persistLoadedDefaults = false;
  if (strlen(gConfig.wifiSsid) == 0) {
    copyString(gConfig.wifiSsid, sizeof(gConfig.wifiSsid), X42_CFG_WIFI_SSID);
    persistLoadedDefaults = true;
  }
  gPreferences.getString("wifi_pw", gConfig.wifiPassword, sizeof(gConfig.wifiPassword));
  if (strlen(gConfig.wifiPassword) == 0) {
    copyString(gConfig.wifiPassword, sizeof(gConfig.wifiPassword), X42_CFG_WIFI_PASSWORD);
    persistLoadedDefaults = true;
  }
  gPreferences.getString("host", gConfig.hostname, sizeof(gConfig.hostname));
  if (strlen(gConfig.hostname) == 0) {
    copyString(gConfig.hostname, sizeof(gConfig.hostname), X42_CFG_HOSTNAME);
  }
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
  gPreferences.getString("name_e_tot", gConfig.sensorEnergyTotal, sizeof(gConfig.sensorEnergyTotal));
  gPreferences.getString("name_e_day", gConfig.sensorEnergyToday, sizeof(gConfig.sensorEnergyToday));
  gPreferences.getString("name_e_sea", gConfig.sensorEnergySeason, sizeof(gConfig.sensorEnergySeason));
  gPreferences.getString("name_e_lst", gConfig.sensorEnergyLastSeason, sizeof(gConfig.sensorEnergyLastSeason));
  gConfig.gainL1 = gPreferences.getFloat("gain_l1", gConfig.gainL1);
  gConfig.gainL2 = gPreferences.getFloat("gain_l2", gConfig.gainL2);
  gConfig.gainL3 = gPreferences.getFloat("gain_l3", gConfig.gainL3);
  gConfig.haEnabled = gPreferences.getBool("ha_en", gConfig.haEnabled);
  gPreferences.getString("ha_l1", gConfig.haNameL1, sizeof(gConfig.haNameL1));
  gPreferences.getString("ha_l2", gConfig.haNameL2, sizeof(gConfig.haNameL2));
  gPreferences.getString("ha_l3", gConfig.haNameL3, sizeof(gConfig.haNameL3));
  gPreferences.getString("ha_tot", gConfig.haNameTotal, sizeof(gConfig.haNameTotal));
  gPreferences.getString("ha_tmp", gConfig.haNameTemp, sizeof(gConfig.haNameTemp));
  gPreferences.getString("ha_hum", gConfig.haNameHumidity, sizeof(gConfig.haNameHumidity));
  gPreferences.getString("ha_e_tot", gConfig.haNameEnergyTotal, sizeof(gConfig.haNameEnergyTotal));
  gPreferences.getString("ha_e_day", gConfig.haNameEnergyToday, sizeof(gConfig.haNameEnergyToday));
  gPreferences.getString("ha_e_sea", gConfig.haNameEnergySeason, sizeof(gConfig.haNameEnergySeason));
  gPreferences.getString("ha_e_lst", gConfig.haNameEnergyLastSeason, sizeof(gConfig.haNameEnergyLastSeason));

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

  applyConfiguredCurrentGains();

  if (persistLoadedDefaults) {
    gPreferences.end();
    return ConfigBootAction::PersistDefaults;
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
  gPreferences.putString("name_e_tot", gConfig.sensorEnergyTotal);
  gPreferences.putString("name_e_day", gConfig.sensorEnergyToday);
  gPreferences.putString("name_e_sea", gConfig.sensorEnergySeason);
  gPreferences.putString("name_e_lst", gConfig.sensorEnergyLastSeason);
  gPreferences.putFloat("gain_l1", gConfig.gainL1);
  gPreferences.putFloat("gain_l2", gConfig.gainL2);
  gPreferences.putFloat("gain_l3", gConfig.gainL3);
  gPreferences.putBool("ha_en", gConfig.haEnabled);
  gPreferences.putString("ha_l1", gConfig.haNameL1);
  gPreferences.putString("ha_l2", gConfig.haNameL2);
  gPreferences.putString("ha_l3", gConfig.haNameL3);
  gPreferences.putString("ha_tot", gConfig.haNameTotal);
  gPreferences.putString("ha_tmp", gConfig.haNameTemp);
  gPreferences.putString("ha_hum", gConfig.haNameHumidity);
  gPreferences.putString("ha_e_tot", gConfig.haNameEnergyTotal);
  gPreferences.putString("ha_e_day", gConfig.haNameEnergyToday);
  gPreferences.putString("ha_e_sea", gConfig.haNameEnergySeason);
  gPreferences.putString("ha_e_lst", gConfig.haNameEnergyLastSeason);

  gPreferences.end();
}

bool hasValidSystemTime() {
  return time(nullptr) >= kValidEpochThreshold;
}

int32_t localDayKeyFromEpoch(time_t epoch) {
  if (epoch <= 0) {
    return -1;
  }

  struct tm tmLocal;
  localtime_r(&epoch, &tmLocal);
  return (tmLocal.tm_year + 1900) * 10000 + (tmLocal.tm_mon + 1) * 100 + tmLocal.tm_mday;
}

void saveEnergyState() {
  if (!gPreferences.begin(kEnergyNs, false)) {
    return;
  }

  gPreferences.putFloat("total", gEnergy.totalKWh);
  gPreferences.putFloat("today", gEnergy.todayKWh);
  gPreferences.putFloat("season", gEnergy.seasonKWh);
  gPreferences.putFloat("last_season", gEnergy.lastSeasonKWh);
  gPreferences.putInt("day_key", gEnergy.localDayKey);
  gPreferences.end();
  gEnergy.lastPersistMs = millis();
}

void loadEnergyState() {
  gEnergy = {0.0F, 0.0F, 0.0F, 0.0F, -1, 0, 0, false};
  if (!gPreferences.begin(kEnergyNs, true)) {
    return;
  }

  gEnergy.totalKWh = gPreferences.getFloat("total", 0.0F);
  gEnergy.todayKWh = gPreferences.getFloat("today", 0.0F);
  gEnergy.seasonKWh = gPreferences.getFloat("season", 0.0F);
  gEnergy.lastSeasonKWh = gPreferences.getFloat("last_season", 0.0F);
  gEnergy.localDayKey = gPreferences.getInt("day_key", -1);
  gPreferences.end();
}

void updateEnergyDayBoundary() {
  if (!hasValidSystemTime()) {
    return;
  }

  const int32_t dayKey = localDayKeyFromEpoch(time(nullptr));
  if (dayKey < 0) {
    return;
  }

  if (gEnergy.localDayKey < 0) {
    gEnergy.localDayKey = dayKey;
    return;
  }

  if (dayKey != gEnergy.localDayKey) {
    gEnergy.todayKWh = 0.0F;
    gEnergy.localDayKey = dayKey;
    saveEnergyState();
  }
}

void integrateEnergy(float totalPowerW) {
  const uint32_t now = millis();
  if (!gEnergy.initialized) {
    gEnergy.initialized = true;
    gEnergy.lastSampleMs = now;
    updateEnergyDayBoundary();
    return;
  }

  uint32_t deltaMs = now - gEnergy.lastSampleMs;
  gEnergy.lastSampleMs = now;
  updateEnergyDayBoundary();

  if (deltaMs == 0) {
    return;
  }
  if (deltaMs > kMaxEnergyDeltaMs) {
    deltaMs = kMaxEnergyDeltaMs;
  }

  const float powerW = totalPowerW > 0.0F ? totalPowerW : 0.0F;
  if (powerW <= 0.0F) {
    if (now - gEnergy.lastPersistMs >= kEnergyPersistIntervalMs) {
      saveEnergyState();
    }
    return;
  }

  const float deltaKWh = (powerW * (static_cast<float>(deltaMs) / 3600000.0F)) / 1000.0F;
  if (deltaKWh <= 0.0F) {
    return;
  }

  gEnergy.totalKWh += deltaKWh;
  gEnergy.todayKWh += deltaKWh;
  gEnergy.seasonKWh += deltaKWh;

  if (now - gEnergy.lastPersistMs >= kEnergyPersistIntervalMs) {
    saveEnergyState();
  }
}

void resetSeasonEnergy() {
  gEnergy.lastSeasonKWh = gEnergy.seasonKWh;
  gEnergy.seasonKWh = 0.0F;
  saveEnergyState();
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
  Serial.println("RJ45 breakout pin mapping prepared:");
  Serial.printf("  pH analog -> GPIO%u\n", kRj45PhAnalogPin);
  Serial.printf("  outdoor DHT11 data -> GPIO%u\n", kOutdoorDhtPin);
  Serial.printf("  radar TX -> GPIO%u\n", kRj45RadarTxPin);
  Serial.printf("  radar RX -> GPIO%u\n", kRj45RadarRxPin);
  Serial.printf("  I2C SCL -> GPIO%u\n", kRj45SclPin);
  Serial.printf("  I2C SDA -> GPIO%u\n", kRj45SdaPin);
  Serial.printf("  DS18B20 OneWire -> GPIO%u\n", kRj45Ds18b20Pin);
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

void updateClimateMeasurement(ClimateState& climate, DHT& sensor) {
  const uint32_t now = millis();
  if (now - climate.lastReadMs < kClimateReadIntervalMs) {
    return;
  }

  climate.lastReadMs = now;
  const float humidity = sensor.readHumidity();
  const float temperatureC = sensor.readTemperature();

  if (isnan(humidity) || isnan(temperatureC)) {
    return;
  }

  climate.hasValidSample = true;
  float correctedHumidity = humidity + gConfig.humidityOffsetPct;
  if (correctedHumidity < 0.0F) {
    correctedHumidity = 0.0F;
  }
  if (correctedHumidity > 100.0F) {
    correctedHumidity = 100.0F;
  }

  climate.humidityPercent = correctedHumidity;
  climate.temperatureC = temperatureC + gConfig.tempOffsetC;
  climate.lastSuccessMs = now;
}

const char* radarPresenceText(bool personDetected) {
  return personDetected ? "True" : "False";
}

String radarDetectedAgoText(uint32_t ageSeconds) {
  if (ageSeconds <= 59U) {
    return String(ageSeconds) + " sec. ago";
  }
  const uint32_t ageMinutes = ageSeconds / 60U;
  return String(ageMinutes) + " min. ago";
}

void consumeRadarPayload(const uint8_t* payload, size_t payloadLen) {
  if (payloadLen < 3) {
    return;
  }

  if (payload[0] != 0x02 || payload[1] != 0xAA) {
    return;
  }

  const uint8_t targetState = payload[2];
  const bool personDetected = targetState != 0;
  const bool wasDetected = gRj45.radarPersonDetected;
  const bool hadFrameBefore = gRj45.radarFrameSeen;
  const uint32_t now = millis();
  gRj45.radarFrameSeen = true;
  gRj45.radarPresenceFresh = true;
  gRj45.radarPersonDetected = personDetected;
  gRj45.radarTargetState = targetState;
  gRj45.lastRadarFrameMs = now;

  // Track last motion event on rising edge to avoid resetting timer on each radar frame.
  if (personDetected && (!hadFrameBefore || !wasDetected)) {
    gRj45.lastRadarMotionDetectedMs = now;
  }
}

void processRadarUartByte(uint8_t byteValue) {
  if (gRadarFrameBufferLen >= sizeof(gRadarFrameBuffer)) {
    gRadarFrameBufferLen = 0;
  }
  gRadarFrameBuffer[gRadarFrameBufferLen++] = byteValue;

  const uint8_t kHeader[4] = {0xF4, 0xF3, 0xF2, 0xF1};
  const uint8_t kTail[4] = {0xF8, 0xF7, 0xF6, 0xF5};

  while (gRadarFrameBufferLen >= 10) {
    size_t frameStart = 0;
    while (frameStart + 3 < gRadarFrameBufferLen) {
      if (gRadarFrameBuffer[frameStart] == kHeader[0] &&
          gRadarFrameBuffer[frameStart + 1] == kHeader[1] &&
          gRadarFrameBuffer[frameStart + 2] == kHeader[2] &&
          gRadarFrameBuffer[frameStart + 3] == kHeader[3]) {
        break;
      }
      ++frameStart;
    }

    if (frameStart > 0) {
      memmove(gRadarFrameBuffer, gRadarFrameBuffer + frameStart, gRadarFrameBufferLen - frameStart);
      gRadarFrameBufferLen -= frameStart;
    }

    if (gRadarFrameBufferLen < 10) {
      return;
    }

    const size_t payloadLen = static_cast<size_t>(gRadarFrameBuffer[4]) | (static_cast<size_t>(gRadarFrameBuffer[5]) << 8U);
    if (payloadLen > 64) {
      memmove(gRadarFrameBuffer, gRadarFrameBuffer + 1, gRadarFrameBufferLen - 1);
      --gRadarFrameBufferLen;
      continue;
    }

    const size_t frameLen = 4 + 2 + payloadLen + 4;
    if (gRadarFrameBufferLen < frameLen) {
      return;
    }

    const size_t tailIndex = frameLen - 4;
    if (!(gRadarFrameBuffer[tailIndex] == kTail[0] &&
          gRadarFrameBuffer[tailIndex + 1] == kTail[1] &&
          gRadarFrameBuffer[tailIndex + 2] == kTail[2] &&
          gRadarFrameBuffer[tailIndex + 3] == kTail[3])) {
      memmove(gRadarFrameBuffer, gRadarFrameBuffer + 1, gRadarFrameBufferLen - 1);
      --gRadarFrameBufferLen;
      continue;
    }

    consumeRadarPayload(gRadarFrameBuffer + 6, payloadLen);
    memmove(gRadarFrameBuffer, gRadarFrameBuffer + frameLen, gRadarFrameBufferLen - frameLen);
    gRadarFrameBufferLen -= frameLen;
  }
}

void pollRadarPresence() {
  while (Serial2.available() > 0) {
    processRadarUartByte(static_cast<uint8_t>(Serial2.read()));
  }

  if (gRj45.radarPresenceFresh && millis() - gRj45.lastRadarFrameMs > kRadarPresenceStaleAfterMs) {
    gRj45.radarPresenceFresh = false;
  }
}

void printRadarStartupPresence() {
  if (!gRj45.radarSerialReady) {
    Serial.println("HLK-LD2410C startup: UART not ready");
    return;
  }

  const uint32_t startMs = millis();
  while (millis() - startMs < 2000) {
    pollRadarPresence();
    delay(20);
  }

  if (gRj45.radarFrameSeen) {
    Serial.printf("HLK-LD2410C startup: %s\n", radarPresenceText(gRj45.radarPersonDetected));
  } else {
    Serial.println("HLK-LD2410C startup: no person detected (no valid frame yet)");
  }
}

void refreshDs18b20Detection(bool force) {
  const uint32_t now = millis();
  if (!force && (now - gRj45.lastDs18b20DiscoveryMs < kDs18b20DiscoveryIntervalMs)) {
    return;
  }
  gRj45.lastDs18b20DiscoveryMs = now;

  const bool wasDetected = gRj45.ds18b20Detected;
  gDs18b20.begin();
  gRj45.ds18b20Detected = gDs18b20.getDeviceCount() > 0;

  if (gRj45.ds18b20Detected != wasDetected) {
    Serial.printf(
        "DS18B20 on GPIO%u: %s\n",
        kRj45Ds18b20Pin,
        gRj45.ds18b20Detected ? "detected" : "not detected");
  }
}

void initializeRj45Sensors() {
  Wire.begin(kRj45SdaPin, kRj45SclPin);
  gRj45.i2cReady = true;

  Serial2.begin(kRadarSerialBaudRate, SERIAL_8N1, kRj45RadarRxPin, kRj45RadarTxPin);
  gRj45.radarSerialReady = true;

  pinMode(kRj45Ds18b20Pin, INPUT_PULLUP);
  refreshDs18b20Detection(true);
}

void printCenteredLcdLine(uint8_t row, const char* text) {
  if (!gLcdReady || gLcd == nullptr || row >= kLcdRows || text == nullptr) {
    return;
  }

  const size_t length = strlen(text);
  const size_t visibleLength = length > kLcdColumns ? kLcdColumns : length;
  const uint8_t leftPadding = static_cast<uint8_t>((kLcdColumns - visibleLength) / 2);

  gLcd->setCursor(0, row);
  gLcd->print("                ");
  gLcd->setCursor(leftPadding, row);
  for (size_t i = 0; i < visibleLength; ++i) {
    gLcd->print(text[i]);
  }
}

bool i2cDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

uint8_t detectLcdI2cAddress() {
  if (i2cDevicePresent(kLcdI2cAddress)) {
    return kLcdI2cAddress;
  }
  if (i2cDevicePresent(kLcdAltI2cAddress)) {
    return kLcdAltI2cAddress;
  }
  return 0;
}

void initializeLcd1602() {
  gLastLcdInitAttemptMs = millis();
  gLcdDetectedAddress = detectLcdI2cAddress();
  if (gLcdDetectedAddress == 0) {
    Serial.printf("LCD1602 not found on I2C (tried 0x%02X and 0x%02X)\n", kLcdI2cAddress, kLcdAltI2cAddress);
    return;
  }

  if (gLcd == nullptr || gLcdDetectedAddress != kLcdI2cAddress) {
    if (gLcd != nullptr) {
      delete gLcd;
      gLcd = nullptr;
    }
    gLcd = new LiquidCrystal_I2C(gLcdDetectedAddress, kLcdColumns, kLcdRows);
  }

  gLcd->init();
  gLcd->backlight();
  gLcdReady = true;

  gLcd->clear();
  printCenteredLcdLine(0, "X42Measuring");
  printCenteredLcdLine(1, "Station");
  Serial.printf("LCD1602 initialized at 0x%02X and startup text rendered.\n", gLcdDetectedAddress);
}

void maintainLcdInitialization() {
  if (gLcdReady) {
    return;
  }

  const uint32_t now = millis();
  if (now - gLastLcdInitAttemptMs < kLcdInitRetryIntervalMs) {
    return;
  }
  initializeLcd1602();
}

void updateRj45Measurements() {
  const uint32_t now = millis();
  if (now - gRj45.lastReadMs < kRj45ReadIntervalMs) {
    return;
  }
  gRj45.lastReadMs = now;

  gRj45.phRaw = analogRead(kRj45PhAnalogPin);
  gRj45.phVoltage = (static_cast<float>(gRj45.phRaw) * kAdcReferenceVoltage) / static_cast<float>(kAdcMax);

  pollRadarPresence();

  refreshDs18b20Detection(false);
  gRj45.ds18b20Fresh = false;
  if (gRj45.ds18b20Detected) {
    gDs18b20.requestTemperatures();
    const float dsTempC = gDs18b20.getTempCByIndex(0);
    if (dsTempC != DEVICE_DISCONNECTED_C) {
      gRj45.ds18b20TempC = dsTempC;
      gRj45.ds18b20Fresh = true;
    }
  }
}

bool climateSampleIsFresh(const ClimateState& climate) {
  return climate.hasValidSample && (millis() - climate.lastSuccessMs <= kClimateStaleAfterMs);
}

void updateMeasurements() {
  gRuntime.totalCurrentA = 0.0F;
  gRuntime.totalPowerW = 0.0F;
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    const ChannelMeasurement measurement = readChannelMeasurement(gChannels[index], kSamplingWindowMs);
    const float zeroOffsetA = gZeroOffsetVrms[index] * gChannels[index].ampsPerVolt;
    const float correctedA = measurement.irms > zeroOffsetA ? measurement.irms - zeroOffsetA : 0.0F;
    const float estimatedPowerW = correctedA * kNominalVoltageV;
    gRuntime.phases[index] = {correctedA, estimatedPowerW, measurement.irms, zeroOffsetA, measurement.meanVoltage};
    gRuntime.totalCurrentA += correctedA;
    gRuntime.totalPowerW += estimatedPowerW;
  }
  integrateEnergy(gRuntime.totalPowerW);
  updateClimateMeasurement(gShaftClimate, gShaftDht);
  updateClimateMeasurement(gOutdoorClimate, gOutdoorDht);
  updateRj45Measurements();
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

String availabilityTopic() {
  return topicFor("availability");
}

String jsonEscape(const String& value) {
  String out;
  out.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value.charAt(i);
    if (c == '\\') {
      out += "\\\\";
    } else if (c == '"') {
      out += "\\\"";
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else {
      out += c;
    }
  }
  return out;
}

String mqttToken(const String& value) {
  String out;
  out.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    char c = static_cast<char>(tolower(value.charAt(i)));
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      out += c;
    } else {
      out += '_';
    }
  }
  while (out.indexOf("__") >= 0) {
    out.replace("__", "_");
  }
  if (out.startsWith("_")) {
    out.remove(0, 1);
  }
  if (out.endsWith("_")) {
    out.remove(out.length() - 1);
  }
  if (out.length() == 0) {
    out = "x42";
  }
  return out;
}

String discoveryDeviceId() {
  String id = "x42ms_";
  id += mqttToken(gConfig.hostname);
  id += "_";
  id += String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
  return id;
}

String discoveryTopicForObject(const String& objectId) {
  return String(kHomeAssistantDiscoveryPrefix) + "/sensor/" + discoveryDeviceId() + "/" + objectId + "/config";
}

String discoveryDeviceJson() {
  const String deviceName = "X42MeasuringStation :: " + String(gConfig.hostname);
  String model = "Measuringstation (v." + String(X42_GITHUB_RELEASE);
  const String branch = String(X42_GIT_BRANCH);
  if (branch.length() > 0 && branch != "main") {
    model += "-" + branch;
  }
  model += ")";

  String json = "\"device\":{";
  json += "\"identifiers\":[\"" + jsonEscape(discoveryDeviceId()) + "\"],";
  json += "\"name\":\"" + jsonEscape(deviceName) + "\",";
  json += "\"manufacturer\":\"X42.icu\",";
  json += "\"model\":\"" + jsonEscape(model) + "\"";
  json += "}";
  return json;
}

void publishHomeAssistantDiscovery(
    const String& objectId,
    const String& name,
    const String& stateTopic,
    const char* unit,
    const char* deviceClass,
    const char* stateClass) {
  String payload = "{";
  payload += "\"name\":\"" + jsonEscape(name) + "\",";
  payload += "\"unique_id\":\"" + jsonEscape(discoveryDeviceId() + "_" + objectId) + "\",";
  payload += "\"state_topic\":\"" + jsonEscape(stateTopic) + "\",";
  payload += "\"availability_topic\":\"" + jsonEscape(availabilityTopic()) + "\",";
  payload += "\"payload_available\":\"online\",";
  payload += "\"payload_not_available\":\"offline\",";
  if (unit != nullptr && strlen(unit) > 0) {
    payload += "\"unit_of_measurement\":\"" + jsonEscape(String(unit)) + "\",";
  }
  if (deviceClass != nullptr && strlen(deviceClass) > 0) {
    payload += "\"device_class\":\"" + jsonEscape(String(deviceClass)) + "\",";
  }
  if (stateClass != nullptr && strlen(stateClass) > 0) {
    payload += "\"state_class\":\"" + jsonEscape(String(stateClass)) + "\",";
  }
  payload += discoveryDeviceJson();
  payload += "}";

  const String discoveryTopic = discoveryTopicForObject(objectId);
  const bool ok = gMqtt.publish(discoveryTopic.c_str(), payload.c_str(), true);
  if (!ok) {
    Serial.printf(
        "Home Assistant discovery publish failed: topic=%s payload_len=%u\n",
        discoveryTopic.c_str(),
        static_cast<unsigned>(payload.length()));
  }
}

void publishHomeAssistantAutoDiscovery() {
  const String objectIds[] = {
      "current_l1",
      "current_l2",
      "current_l3",
      "current_total",
      "power_l1",
      "power_l2",
      "power_l3",
      "power_total",
  "energy_total",
  "energy_today",
  "energy_season",
  "energy_last_season",
      "temperature",
      "humidity"};
  if (!gConfig.haEnabled) {
    for (size_t i = 0; i < (sizeof(objectIds) / sizeof(objectIds[0])); ++i) {
      gMqtt.publish(discoveryTopicForObject(objectIds[i]).c_str(), "", true);
    }
    return;
  }

  publishHomeAssistantDiscovery("current_l1", gConfig.haNameL1, topicFor(gConfig.sensorL1), "A", "current", "measurement");
  publishHomeAssistantDiscovery("current_l2", gConfig.haNameL2, topicFor(gConfig.sensorL2), "A", "current", "measurement");
  publishHomeAssistantDiscovery("current_l3", gConfig.haNameL3, topicFor(gConfig.sensorL3), "A", "current", "measurement");
  publishHomeAssistantDiscovery("current_total", gConfig.haNameTotal, topicFor("total_current"), "A", "current", "measurement");
  publishHomeAssistantDiscovery("power_l1", "L1 Power", topicFor("power_l1"), "W", "power", "measurement");
  publishHomeAssistantDiscovery("power_l2", "L2 Power", topicFor("power_l2"), "W", "power", "measurement");
  publishHomeAssistantDiscovery("power_l3", "L3 Power", topicFor("power_l3"), "W", "power", "measurement");
  publishHomeAssistantDiscovery("power_total", "Total Power", topicFor("total_power"), "W", "power", "measurement");
  publishHomeAssistantDiscovery("energy_total", gConfig.haNameEnergyTotal, topicFor(gConfig.sensorEnergyTotal), "kWh", "energy", "total_increasing");
  publishHomeAssistantDiscovery("energy_today", gConfig.haNameEnergyToday, topicFor(gConfig.sensorEnergyToday), "kWh", "energy", "total");
  publishHomeAssistantDiscovery("energy_season", gConfig.haNameEnergySeason, topicFor(gConfig.sensorEnergySeason), "kWh", "energy", "total");
  publishHomeAssistantDiscovery("energy_last_season", gConfig.haNameEnergyLastSeason, topicFor(gConfig.sensorEnergyLastSeason), "kWh", "energy", "total");
  publishHomeAssistantDiscovery("temperature", gConfig.haNameTemp, topicFor(gConfig.sensorTemp), "C", "temperature", "measurement");
  publishHomeAssistantDiscovery("humidity", gConfig.haNameHumidity, topicFor(gConfig.sensorHumidity), "%", "humidity", "measurement");
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
  publishFloat(topicFor("power_l1"), gRuntime.phases[0].powerW, 1);
  publishFloat(topicFor("power_l2"), gRuntime.phases[1].powerW, 1);
  publishFloat(topicFor("power_l3"), gRuntime.phases[2].powerW, 1);
  publishFloat(topicFor("total_power"), gRuntime.totalPowerW, 1);
  publishFloat(topicFor(gConfig.sensorEnergyTotal), gEnergy.totalKWh, 3);
  publishFloat(topicFor(gConfig.sensorEnergyToday), gEnergy.todayKWh, 3);
  publishFloat(topicFor(gConfig.sensorEnergySeason), gEnergy.seasonKWh, 3);
  publishFloat(topicFor(gConfig.sensorEnergyLastSeason), gEnergy.lastSeasonKWh, 3);

  if (climateSampleIsFresh(gOutdoorClimate)) {
    publishFloat(topicFor(gConfig.sensorTemp), gOutdoorClimate.temperatureC, 1);
    publishFloat(topicFor(gConfig.sensorHumidity), gOutdoorClimate.humidityPercent, 1);
  } else if (climateSampleIsFresh(gShaftClimate)) {
    publishFloat(topicFor(gConfig.sensorTemp), gShaftClimate.temperatureC, 1);
    publishFloat(topicFor(gConfig.sensorHumidity), gShaftClimate.humidityPercent, 1);
  }

  publishFloat(topicFor("rj45/ph_voltage"), gRj45.phVoltage, 3);
  if (climateSampleIsFresh(gOutdoorClimate)) {
    publishFloat(topicFor("rj45/outdoor_temp_c"), gOutdoorClimate.temperatureC, 1);
    publishFloat(topicFor("rj45/outdoor_humidity_percent"), gOutdoorClimate.humidityPercent, 1);
  }
  if (gRj45.ds18b20Fresh) {
    publishFloat(topicFor("rj45/water_temp_c"), gRj45.ds18b20TempC, 2);
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
  if (!gMqtt.setBufferSize(kMqttPayloadBufferSize)) {
    Serial.printf("Failed to set MQTT buffer size to %u bytes\n", static_cast<unsigned>(kMqttPayloadBufferSize));
  }
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
  const String lwtTopic = availabilityTopic();
  if (strlen(gConfig.mqttUser) > 0) {
    connected = gMqtt.connect(
        mqttClientId().c_str(),
        gConfig.mqttUser,
        gConfig.mqttPassword,
        lwtTopic.c_str(),
        0,
        true,
        "offline");
  } else {
    connected = gMqtt.connect(
        mqttClientId().c_str(),
        lwtTopic.c_str(),
        0,
        true,
        "offline");
  }

  if (connected) {
    gMqtt.publish(lwtTopic.c_str(), "online", true);
    gMqtt.subscribe(topicFor("cmd").c_str());
    gMqtt.subscribe(topicFor("cmd/zero").c_str());
    gMqtt.subscribe(topicFor("cmd/time").c_str());
    publishHomeAssistantAutoDiscovery();
    publishMqttMeasurements();
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
  if (climateSampleIsFresh(gShaftClimate)) {
    Serial.printf(
        "DHT11 GPIO%u: temperature=%.1f C, humidity=%.1f %%\n",
        kShaftDhtPin,
        gShaftClimate.temperatureC,
        gShaftClimate.humidityPercent);
  } else if (gShaftClimate.hasValidSample) {
    Serial.printf("DHT11 GPIO%u: stale sample age=%lu ms\n", kShaftDhtPin, millis() - gShaftClimate.lastSuccessMs);
  } else {
    Serial.printf("DHT11 GPIO%u: waiting for first valid sample\n", kShaftDhtPin);
  }
  Serial.println("=================================");

  Serial.println("=== Outdoor Climate (RJ45) ===");
  if (climateSampleIsFresh(gOutdoorClimate)) {
    Serial.printf(
        "DHT11 GPIO%u: temperature=%.1f C, humidity=%.1f %%\n",
        kOutdoorDhtPin,
        gOutdoorClimate.temperatureC,
        gOutdoorClimate.humidityPercent);
  } else if (gOutdoorClimate.hasValidSample) {
    Serial.printf("DHT11 GPIO%u: stale sample age=%lu ms\n", kOutdoorDhtPin, millis() - gOutdoorClimate.lastSuccessMs);
  } else {
    Serial.printf("DHT11 GPIO%u: waiting for first valid sample\n", kOutdoorDhtPin);
  }
  Serial.println("================================");

  Serial.println("=== RJ45 Sensor Breakout ===");
  Serial.printf("pH analog GPIO%u: raw=%u, voltage=%.3f V\n", kRj45PhAnalogPin, gRj45.phRaw, gRj45.phVoltage);
  if (gRj45.ds18b20Fresh) {
    Serial.printf("DS18B20 GPIO%u: %.2f C\n", kRj45Ds18b20Pin, gRj45.ds18b20TempC);
  } else if (gRj45.ds18b20Detected) {
    Serial.printf("DS18B20 GPIO%u: waiting for valid sample\n", kRj45Ds18b20Pin);
  } else {
    Serial.printf("DS18B20 GPIO%u: no sensor detected\n", kRj45Ds18b20Pin);
  }
  Serial.printf("I2C ready=%s (SCL GPIO%u, SDA GPIO%u)\n", gRj45.i2cReady ? "true" : "false", kRj45SclPin, kRj45SdaPin);
  Serial.printf(
      "Radar UART ready=%s (TX GPIO%u, RX GPIO%u, baud=%lu)\n",
      gRj45.radarSerialReady ? "true" : "false",
      kRj45RadarTxPin,
      kRj45RadarRxPin,
      static_cast<unsigned long>(kRadarSerialBaudRate));
  Serial.printf(
      "HLK-LD2410C presence: %s%s\n",
      radarPresenceText(gRj45.radarPersonDetected),
      gRj45.radarPresenceFresh ? "" : " (no fresh frame)");
  Serial.println("============================");
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

String dashboardHtml(bool authenticated, const String& savedScope, bool rebootRequired, bool gainRecalibrationRecommended) {
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
  html += "<article class='x42-card'><h3>L1 Current</h3><div class='x42-value' id='val-l1-current'>" + String(gRuntime.phases[0].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>L2 Current</h3><div class='x42-value' id='val-l2-current'>" + String(gRuntime.phases[1].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>L3 Current</h3><div class='x42-value' id='val-l3-current'>" + String(gRuntime.phases[2].currentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>Total Current</h3><div class='x42-value' id='val-total-current'>" + String(gRuntime.totalCurrentA, 3) + " A</div></article>";
  html += "<article class='x42-card'><h3>L1 Power</h3><div class='x42-value' id='val-l1-power'>" + String(gRuntime.phases[0].powerW, 1) + " W</div></article>";
  html += "<article class='x42-card'><h3>L2 Power</h3><div class='x42-value' id='val-l2-power'>" + String(gRuntime.phases[1].powerW, 1) + " W</div></article>";
  html += "<article class='x42-card'><h3>L3 Power</h3><div class='x42-value' id='val-l3-power'>" + String(gRuntime.phases[2].powerW, 1) + " W</div></article>";
  html += "<article class='x42-card'><h3>Total Power</h3><div class='x42-value' id='val-total-power'>" + String(gRuntime.totalPowerW, 1) + " W</div></article>";
  html += "<article class='x42-card'><h3>Total Energy</h3><div class='x42-value' id='val-energy-total'>" + String(gEnergy.totalKWh, 3) + " kWh</div></article>";
  html += "<article class='x42-card'><h3>Current Today</h3><div class='x42-value' id='val-energy-today'>" + String(gEnergy.todayKWh, 3) + " kWh</div></article>";
  html += "<article class='x42-card'><h3>pH Analog Voltage</h3><div class='x42-value' id='val-rj45-ph-voltage'>" + String(gRj45.phVoltage, 3) + " V</div></article>";
  const uint32_t radarAgeSec = gRj45.lastRadarMotionDetectedMs == 0 ? 0U : (millis() - gRj45.lastRadarMotionDetectedMs) / 1000U;
  String radarLabel = radarPresenceText(gRj45.radarPersonDetected);
  if (gRj45.lastRadarMotionDetectedMs > 0) {
    radarLabel += " (" + radarDetectedAgoText(radarAgeSec) + ")";
  } else {
    radarLabel += " (never)";
  }
  html += "<article class='x42-card'><h3>Radar Presence</h3><div class='x42-value' id='val-rj45-radar-presence'>" + radarLabel + "</div></article>";
  if (climateSampleIsFresh(gOutdoorClimate)) {
    html += "<article class='x42-card'><h3>Outdoor Temperature</h3><div class='x42-value' id='val-outdoor-temp'>" + String(gOutdoorClimate.temperatureC, 1) + " C</div></article>";
    html += "<article class='x42-card'><h3>Outdoor Humidity</h3><div class='x42-value' id='val-outdoor-humidity'>" + String(gOutdoorClimate.humidityPercent, 1) + " %</div></article>";
  } else {
    html += "<article class='x42-card'><h3>Outdoor Temperature</h3><div class='x42-value' id='val-outdoor-temp'>waiting/stale</div></article>";
    html += "<article class='x42-card'><h3>Outdoor Humidity</h3><div class='x42-value' id='val-outdoor-humidity'>waiting/stale</div></article>";
  }
  if (gRj45.ds18b20Fresh) {
    html += "<article class='x42-card'><h3>Water Temperature (DS18B20)</h3><div class='x42-value' id='val-rj45-water-temp'>" + String(gRj45.ds18b20TempC, 2) + " C</div></article>";
  } else {
    html += "<article class='x42-card'><h3>Water Temperature (DS18B20)</h3><div class='x42-value' id='val-rj45-water-temp'>waiting/sensor</div></article>";
  }
  if (climateSampleIsFresh(gShaftClimate)) {
    html += "<article class='x42-card'><h3>Shaft Temperature</h3><div class='x42-value' id='val-shaft-temp'>" + String(gShaftClimate.temperatureC, 1) + " C</div></article>";
    html += "<article class='x42-card'><h3>Shaft Humidity</h3><div class='x42-value' id='val-shaft-humidity'>" + String(gShaftClimate.humidityPercent, 1) + " %</div></article>";
  } else {
    html += "<article class='x42-card'><h3>Shaft Temperature</h3><div class='x42-value' id='val-shaft-temp'>waiting/stale</div></article>";
    html += "<article class='x42-card'><h3>Shaft Humidity</h3><div class='x42-value' id='val-shaft-humidity'>waiting/stale</div></article>";
  }
  html += "</div></section>";

  html += "<section><h2>Season Counter</h2><article class='x42-card'>";
  html += "<div class='x42-grid'>";
  html += "<div><h3>Energy This Season</h3><div class='x42-value' id='val-energy-season'>" + String(gEnergy.seasonKWh, 3) + " kWh</div></div>";
  html += "<div><h3>Energy Last Season</h3><div class='x42-value' id='val-energy-last-season'>" + String(gEnergy.lastSeasonKWh, 3) + " kWh</div></div>";
  html += "</div>";
  if (authenticated) {
    if (savedScope == "energy") {
      html += "<p><small style='color:var(--pico-color-green-500);'>season reset done</small></p>";
    }
    html += "<form method='post' action='/energy-season-reset'>";
    html += "<label>Type YES to confirm moving <strong>Energy This Season</strong> to <strong>Energy Last Season</strong> and resetting <strong>Energy This Season</strong> to 0.";
    html += "<input name='confirm' placeholder='YES'/></label>";
    html += "<button type='submit' class='secondary'>Reset Energy This Season (with rollover)</button></form>";
  } else {
    html += "<p class='x42-muted'>Login is required to reset season counters.</p>";
  }
  html += "</article></section>";

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

  html += "<form method='post' action='/save-config'>";
  html += "<section><h2>Configuration</h2><article>";
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
  html += "<div class='grid'><label>Humidity<input name='name_h' value='" + htmlEscape(gConfig.sensorHumidity) + "'/></label>";
  html += "<label>Energy Total<input name='name_e_tot' value='" + htmlEscape(gConfig.sensorEnergyTotal) + "'/></label></div>";
  html += "<div class='grid'><label>Energy Today<input name='name_e_day' value='" + htmlEscape(gConfig.sensorEnergyToday) + "'/></label>";
  html += "<label>Energy Season<input name='name_e_sea' value='" + htmlEscape(gConfig.sensorEnergySeason) + "'/></label></div>";
  html += "<label>Energy Last Season<input name='name_e_lst' value='" + htmlEscape(gConfig.sensorEnergyLastSeason) + "'/></label>";

  html += "<h3>Timezone</h3>";
  html += "<div class='grid'><label>NTP Server<input name='ntp_srv' value='" + htmlEscape(gConfig.ntpServer) + "'/></label>";
  html += "<label>Timezone Offset (min)<input name='tz_min' type='number' min='-720' max='840' value='" + String(gConfig.timezoneOffsetMinutes) + "'/></label></div>";

  html += "<h3>Offsets</h3>";
  html += "<div class='grid'><label>Temperature Offset (C)<input name='temp_off' type='number' step='0.1' min='-20' max='20' value='" + String(gConfig.tempOffsetC, 1) + "'/></label>";
  html += "<label>Humidity Offset (%)<input name='hum_off' type='number' step='0.1' min='-50' max='50' value='" + String(gConfig.humidityOffsetPct, 1) + "'/></label></div>";

  html += "<h3>Current Calibration";
  if (savedScope == "config") {
    html += " <small style='color:var(--pico-color-green-500);'>saved</small>";
  }
  html += "</h3>";
  if (gainRecalibrationRecommended) {
    html += "<p><small style='color:var(--pico-color-amber-500);'>Gain changed: run Zero Calibration with empty CT clamps for accurate low-current values.</small></p>";
  }
  html += "<div class='grid'><label>L1 Gain (A/Vrms)<input name='gain_l1' type='number' step='0.001' min='0.05' max='5' value='" + String(gConfig.gainL1, 3) + "'/></label>";
  html += "<label>L2 Gain (A/Vrms)<input name='gain_l2' type='number' step='0.001' min='0.05' max='5' value='" + String(gConfig.gainL2, 3) + "'/></label></div>";
  html += "<label>L3 Gain (A/Vrms)<input name='gain_l3' type='number' step='0.001' min='0.05' max='5' value='" + String(gConfig.gainL3, 3) + "'/></label>";

  html += "<button type='submit' name='save_scope' value='config'>Save Configuration</button></article></section>";

  html += "<section><h2>Home Assistant</h2><article>";
  html += "<h3>Discovery";
  if (savedScope == "ha") {
    html += " <small style='color:var(--pico-color-green-500);'>saved</small>";
  }
  html += "</h3>";
  html += "<label><input type='checkbox' name='ha_enabled' role='switch'";
  if (gConfig.haEnabled) {
    html += " checked";
  }
  html += "/>Enable MQTT Auto-Discovery</label>";
  html += "<div class='grid'><label>L1 Name<input name='ha_l1' value='" + htmlEscape(gConfig.haNameL1) + "'/></label>";
  html += "<label>L2 Name<input name='ha_l2' value='" + htmlEscape(gConfig.haNameL2) + "'/></label></div>";
  html += "<div class='grid'><label>L3 Name<input name='ha_l3' value='" + htmlEscape(gConfig.haNameL3) + "'/></label>";
  html += "<label>Total Name<input name='ha_tot' value='" + htmlEscape(gConfig.haNameTotal) + "'/></label></div>";
  html += "<div class='grid'><label>Temperature Name<input name='ha_tmp' value='" + htmlEscape(gConfig.haNameTemp) + "'/></label>";
  html += "<label>Humidity Name<input name='ha_hum' value='" + htmlEscape(gConfig.haNameHumidity) + "'/></label></div>";
  html += "<div class='grid'><label>Energy Total Name<input name='ha_e_tot' value='" + htmlEscape(gConfig.haNameEnergyTotal) + "'/></label>";
  html += "<label>Energy Today Name<input name='ha_e_day' value='" + htmlEscape(gConfig.haNameEnergyToday) + "'/></label></div>";
  html += "<div class='grid'><label>Energy Season Name<input name='ha_e_sea' value='" + htmlEscape(gConfig.haNameEnergySeason) + "'/></label>";
  html += "<label>Energy Last Season Name<input name='ha_e_lst' value='" + htmlEscape(gConfig.haNameEnergyLastSeason) + "'/></label></div>";
  html += "<button type='submit' name='save_scope' value='ha'>Save Home Assistant Configuration</button></article></section>";
  html += "</form>";

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

  html += "<script>";
  html += "const X42_REBOOT_REQUIRED=" + String(rebootRequired ? "true" : "false") + ";";
  html += "const fmt=(n,d,u)=>typeof n==='number'?n.toFixed(d)+' '+u:'--';";
  html += "const radarAgo=(s)=>{if(typeof s!=='number'||s<0)return 'never';const secs=Math.floor(s);if(secs<=59)return secs+' sec. ago';const mins=Math.floor(secs/60);return mins+' min. ago';};";
  html += "const setVal=(id,v)=>{const el=document.getElementById(id);if(el)el.textContent=v;};";
  html += "async function refreshLive(){try{const r=await fetch('/status.json',{cache:'no-store'});if(!r.ok)return;const j=await r.json();";
  html += "setVal('val-l1-current',fmt(j.currents?.L1,3,'A'));setVal('val-l2-current',fmt(j.currents?.L2,3,'A'));setVal('val-l3-current',fmt(j.currents?.L3,3,'A'));setVal('val-total-current',fmt(j.currents?.total,3,'A'));";
  html += "setVal('val-l1-power',fmt(j.power_w?.L1,1,'W'));setVal('val-l2-power',fmt(j.power_w?.L2,1,'W'));setVal('val-l3-power',fmt(j.power_w?.L3,1,'W'));setVal('val-total-power',fmt(j.power_w?.total,1,'W'));";
  html += "setVal('val-energy-total',fmt(j.energy_kwh?.total,3,'kWh'));setVal('val-energy-today',fmt(j.energy_kwh?.today,3,'kWh'));setVal('val-energy-season',fmt(j.energy_kwh?.season,3,'kWh'));setVal('val-energy-last-season',fmt(j.energy_kwh?.last_season,3,'kWh'));";
  html += "setVal('val-rj45-ph-voltage',fmt(j.rj45?.ph_voltage,3,'V'));";
  html += "{const state=j.rj45?.radar_person_detected?'True':'False';const age=radarAgo(j.rj45?.radar_last_detected_ago_s);setVal('val-rj45-radar-presence',state+' ('+age+')');}";
  html += "if(j.climate_outdoor?.fresh){setVal('val-outdoor-temp',fmt(j.climate_outdoor?.temperature_c,1,'C'));setVal('val-outdoor-humidity',fmt(j.climate_outdoor?.humidity_percent,1,'%'));}else{setVal('val-outdoor-temp','waiting/stale');setVal('val-outdoor-humidity','waiting/stale');}";
  html += "if(j.climate_shaft?.fresh){setVal('val-shaft-temp',fmt(j.climate_shaft?.temperature_c,1,'C'));setVal('val-shaft-humidity',fmt(j.climate_shaft?.humidity_percent,1,'%'));}else{setVal('val-shaft-temp','waiting/stale');setVal('val-shaft-humidity','waiting/stale');}";
  html += "if(j.rj45?.ds18b20_fresh){setVal('val-rj45-water-temp',fmt(j.rj45?.ds18b20_temp_c,2,'C'));}else{setVal('val-rj45-water-temp','waiting/sensor');}";
  html += "}catch(e){}}";
  html += "refreshLive();setInterval(refreshLive,5000);";
  html += "if(X42_REBOOT_REQUIRED){alert('WiFi/Hostname changed. Reboot is required to apply network identity changes.');}";
  html += "if(" + String(gainRecalibrationRecommended ? "true" : "false") + "){alert('Gain changed. Please run Zero Calibration with empty CT clamps to restore accuracy.');}";
  html += "</script>";
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
  const String savedScope = gServer.hasArg("saved") ? gServer.arg("saved") : String();
  const bool rebootRequired = gServer.hasArg("reboot") && gServer.arg("reboot") == "1";
  const bool gainRecalibrationRecommended = gServer.hasArg("gain") && gServer.arg("gain") == "1";
  gServer.send(200, "text/html", dashboardHtml(isAuthenticated(), savedScope, rebootRequired, gainRecalibrationRecommended));
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

  const String saveScope = gServer.hasArg("save_scope") ? gServer.arg("save_scope") : String("config");
  const String oldWifiSsid = String(gConfig.wifiSsid);
  const String oldHostname = String(gConfig.hostname);
  const float oldGainL1 = gConfig.gainL1;
  const float oldGainL2 = gConfig.gainL2;
  const float oldGainL3 = gConfig.gainL3;

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
  copyString(gConfig.sensorEnergyTotal, sizeof(gConfig.sensorEnergyTotal), readArg("name_e_tot"));
  copyString(gConfig.sensorEnergyToday, sizeof(gConfig.sensorEnergyToday), readArg("name_e_day"));
  copyString(gConfig.sensorEnergySeason, sizeof(gConfig.sensorEnergySeason), readArg("name_e_sea"));
  copyString(gConfig.sensorEnergyLastSeason, sizeof(gConfig.sensorEnergyLastSeason), readArg("name_e_lst"));
  gConfig.haEnabled = gServer.hasArg("ha_enabled");
  copyString(gConfig.haNameL1, sizeof(gConfig.haNameL1), readArg("ha_l1"));
  copyString(gConfig.haNameL2, sizeof(gConfig.haNameL2), readArg("ha_l2"));
  copyString(gConfig.haNameL3, sizeof(gConfig.haNameL3), readArg("ha_l3"));
  copyString(gConfig.haNameTotal, sizeof(gConfig.haNameTotal), readArg("ha_tot"));
  copyString(gConfig.haNameTemp, sizeof(gConfig.haNameTemp), readArg("ha_tmp"));
  copyString(gConfig.haNameHumidity, sizeof(gConfig.haNameHumidity), readArg("ha_hum"));
  copyString(gConfig.haNameEnergyTotal, sizeof(gConfig.haNameEnergyTotal), readArg("ha_e_tot"));
  copyString(gConfig.haNameEnergyToday, sizeof(gConfig.haNameEnergyToday), readArg("ha_e_day"));
  copyString(gConfig.haNameEnergySeason, sizeof(gConfig.haNameEnergySeason), readArg("ha_e_sea"));
  copyString(gConfig.haNameEnergyLastSeason, sizeof(gConfig.haNameEnergyLastSeason), readArg("ha_e_lst"));

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

  gConfig.gainL1 = sanitizeGain(readArg("gain_l1").toFloat(), gConfig.gainL1);
  gConfig.gainL2 = sanitizeGain(readArg("gain_l2").toFloat(), gConfig.gainL2);
  gConfig.gainL3 = sanitizeGain(readArg("gain_l3").toFloat(), gConfig.gainL3);
  const bool gainChanged = fabsf(gConfig.gainL1 - oldGainL1) > 0.0005F || fabsf(gConfig.gainL2 - oldGainL2) > 0.0005F || fabsf(gConfig.gainL3 - oldGainL3) > 0.0005F;
  applyConfiguredCurrentGains();

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

  const bool rebootRequired = oldWifiSsid != String(gConfig.wifiSsid) || oldHostname != String(gConfig.hostname);
  String location = "/?saved=" + saveScope;
  if (rebootRequired) {
    location += "&reboot=1";
  }
  if (gainChanged) {
    location += "&gain=1";
  }
  gServer.sendHeader("Location", location);
  gServer.send(303, "text/plain", "Saved");
}

void handleEnergySeasonReset() {
  if (!isAuthenticated()) {
    requireAuthOrReject();
    return;
  }

  const String confirm = readArg("confirm");
  if (confirm != "YES") {
    gServer.send(400, "text/plain", "Season reset aborted. Type YES to confirm.");
    return;
  }

  resetSeasonEnergy();
  gServer.sendHeader("Location", "/?saved=energy");
  gServer.send(303, "text/plain", "Season reset done");
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
  json += "\"total\":" + String(gRuntime.totalCurrentA, 3) + ",";
  json += "\"gain_l1\":" + String(gConfig.gainL1, 3) + ",";
  json += "\"gain_l2\":" + String(gConfig.gainL2, 3) + ",";
  json += "\"gain_l3\":" + String(gConfig.gainL3, 3);
  json += "},";
  json += "\"power_w\":{";
  json += "\"L1\":" + String(gRuntime.phases[0].powerW, 1) + ",";
  json += "\"L2\":" + String(gRuntime.phases[1].powerW, 1) + ",";
  json += "\"L3\":" + String(gRuntime.phases[2].powerW, 1) + ",";
  json += "\"total\":" + String(gRuntime.totalPowerW, 1);
  json += "},";
  json += "\"energy_kwh\":{";
  json += "\"total\":" + String(gEnergy.totalKWh, 3) + ",";
  json += "\"today\":" + String(gEnergy.todayKWh, 3) + ",";
  json += "\"season\":" + String(gEnergy.seasonKWh, 3) + ",";
  json += "\"last_season\":" + String(gEnergy.lastSeasonKWh, 3);
  json += "},";
  json += "\"climate\":{";
  if (climateSampleIsFresh(gOutdoorClimate)) {
    json += "\"temperature_c\":" + String(gOutdoorClimate.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(gOutdoorClimate.humidityPercent, 1) + ",";
    json += "\"source\":\"outdoor\",";
    json += "\"temperature_offset_c\":" + String(gConfig.tempOffsetC, 1) + ",";
    json += "\"humidity_offset_percent\":" + String(gConfig.humidityOffsetPct, 1) + ",";
    json += "\"fresh\":true";
  } else if (climateSampleIsFresh(gShaftClimate)) {
    json += "\"temperature_c\":" + String(gShaftClimate.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(gShaftClimate.humidityPercent, 1) + ",";
    json += "\"source\":\"shaft\",";
    json += "\"temperature_offset_c\":" + String(gConfig.tempOffsetC, 1) + ",";
    json += "\"humidity_offset_percent\":" + String(gConfig.humidityOffsetPct, 1) + ",";
    json += "\"fresh\":true";
  } else {
    json += "\"fresh\":false";
  }
  json += "},";
  json += "\"climate_outdoor\":{";
  if (climateSampleIsFresh(gOutdoorClimate)) {
    json += "\"temperature_c\":" + String(gOutdoorClimate.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(gOutdoorClimate.humidityPercent, 1) + ",";
    json += "\"fresh\":true";
  } else {
    json += "\"fresh\":false";
  }
  json += "},";
  json += "\"climate_shaft\":{";
  if (climateSampleIsFresh(gShaftClimate)) {
    json += "\"temperature_c\":" + String(gShaftClimate.temperatureC, 1) + ",";
    json += "\"humidity_percent\":" + String(gShaftClimate.humidityPercent, 1) + ",";
    json += "\"fresh\":true";
  } else {
    json += "\"fresh\":false";
  }
  json += "},";
  json += "\"rj45\":{";
  json += "\"pins\":{";
  json += "\"ph_analog\":" + String(kRj45PhAnalogPin) + ",";
  json += "\"outdoor_dht_data\":" + String(kOutdoorDhtPin) + ",";
  json += "\"radar_tx\":" + String(kRj45RadarTxPin) + ",";
  json += "\"radar_rx\":" + String(kRj45RadarRxPin) + ",";
  json += "\"scl\":" + String(kRj45SclPin) + ",";
  json += "\"sda\":" + String(kRj45SdaPin) + ",";
  json += "\"ds18b20\":" + String(kRj45Ds18b20Pin);
  json += "},";
  json += "\"ph_raw\":" + String(gRj45.phRaw) + ",";
  json += "\"ph_voltage\":" + String(gRj45.phVoltage, 3) + ",";
  json += "\"i2c_ready\":" + String(gRj45.i2cReady ? "true" : "false") + ",";
  json += "\"radar_uart_ready\":" + String(gRj45.radarSerialReady ? "true" : "false") + ",";
  json += "\"radar_presence_fresh\":" + String(gRj45.radarPresenceFresh ? "true" : "false") + ",";
  json += "\"radar_person_detected\":" + String(gRj45.radarPersonDetected ? "true" : "false") + ",";
  json += "\"radar_target_state\":" + String(gRj45.radarTargetState) + ",";
  if (gRj45.lastRadarMotionDetectedMs > 0) {
    json += "\"radar_last_detected_ago_s\":" + String((millis() - gRj45.lastRadarMotionDetectedMs) / 1000U) + ",";
  } else {
    json += "\"radar_last_detected_ago_s\":-1,";
  }
  json += "\"outdoor_dht_fresh\":" + String(climateSampleIsFresh(gOutdoorClimate) ? "true" : "false") + ",";
  json += "\"outdoor_temp_c\":" + String(gOutdoorClimate.temperatureC, 1) + ",";
  json += "\"outdoor_humidity_percent\":" + String(gOutdoorClimate.humidityPercent, 1) + ",";
  json += "\"ds18b20_detected\":" + String(gRj45.ds18b20Detected ? "true" : "false") + ",";
  json += "\"ds18b20_fresh\":" + String(gRj45.ds18b20Fresh ? "true" : "false") + ",";
  json += "\"ds18b20_temp_c\":" + String(gRj45.ds18b20TempC, 2);
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
  gServer.on("/energy-season-reset", HTTP_POST, handleEnergySeasonReset);
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
  Serial.printf("WiFi config: ssid='%s' (len=%u)\n", gConfig.wifiSsid, static_cast<unsigned>(strlen(gConfig.wifiSsid)));

  if (strlen(gConfig.wifiSsid) == 0) {
    Serial.println("No WiFi SSID configured; entering AP fallback immediately.");
    gIsApMode = true;
  } else {
    Serial.printf(
        "Connecting to WiFi SSID: %s (startup retry window %lu s)\n",
        gConfig.wifiSsid,
        static_cast<unsigned long>(kStartupWifiConnectWindowMs / 1000UL));
    const uint32_t startupStartMs = millis();
    uint32_t attemptStartMs = 0;

    while (WiFi.status() != WL_CONNECTED && millis() - startupStartMs < kStartupWifiConnectWindowMs) {
      const uint32_t now = millis();
      if (attemptStartMs == 0 || now - attemptStartMs >= kWifiRetryAttemptTimeoutMs) {
        WiFi.disconnect(false, false);
        WiFi.mode(WIFI_STA);
        WiFi.setHostname(gConfig.hostname);
        WiFi.begin(gConfig.wifiSsid, gConfig.wifiPassword);
        attemptStartMs = now;
        Serial.print("\nStartup WiFi retry...");
      }
      delay(300);
      Serial.print('.');
    }
    Serial.println();
    gIsApMode = WiFi.status() != WL_CONNECTED;
    if (gIsApMode) {
      Serial.printf("WiFi join failed, status=%d\n", static_cast<int>(WiFi.status()));
    }
  }

  if (gIsApMode) {
    WiFi.mode(WIFI_AP);
    String apSsid = String(gConfig.hostname) + "-setup";
    WiFi.softAP(apSsid.c_str());
    Serial.printf("AP mode active, SSID: %s, IP: %s\n", apSsid.c_str(), WiFi.softAPIP().toString().c_str());
    gWifiRetryInProgress = false;
    gLastWifiRetryAttemptMs = millis();
  } else {
    Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    gWifiRetryInProgress = false;
    syncTimeFromNtp();
  }
}

void maintainWifiFallbackRetry() {
  if (!gIsApMode || strlen(gConfig.wifiSsid) == 0) {
    return;
  }

  const uint32_t now = millis();

  if (!gWifiRetryInProgress) {
    if (now - gLastWifiRetryAttemptMs < kWifiRetryIntervalMs) {
      return;
    }

    gLastWifiRetryAttemptMs = now;
    gWifiRetryStartedMs = now;
    gWifiRetryInProgress = true;

    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(gConfig.hostname);
    WiFi.begin(gConfig.wifiSsid, gConfig.wifiPassword);
    Serial.printf("AP fallback: retrying WiFi SSID %s in background\n", gConfig.wifiSsid);
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    gWifiRetryInProgress = false;
    gIsApMode = false;
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.printf("WiFi fallback recovery successful, IP: %s\n", WiFi.localIP().toString().c_str());
    syncTimeFromNtp();
    return;
  }

  if (now - gWifiRetryStartedMs >= kWifiRetryAttemptTimeoutMs) {
    gWifiRetryInProgress = false;
    WiFi.disconnect(false, false);
    WiFi.mode(WIFI_AP);
    String apSsid = String(gConfig.hostname) + "-setup";
    WiFi.softAP(apSsid.c_str());
    Serial.println("AP fallback: retry timed out, hotspot remains active.");
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
  analogSetPinAttenuation(kRj45PhAnalogPin, ADC_11db);

  loadDefaultZeroCalibration();
  if (loadPersistedZeroCalibration()) {
    Serial.println("Loaded saved zero calibration.");
  } else {
    Serial.println("Using built-in zero calibration defaults.");
  }

  loadEnergyState();
  if (gEnergy.localDayKey < 0 && hasValidSystemTime()) {
    gEnergy.localDayKey = localDayKeyFromEpoch(time(nullptr));
    saveEnergyState();
  }

  gShaftDht.begin();
  gOutdoorDht.begin();
  initializeRj45Sensors();
  initializeLcd1602();
  printRadarStartupPresence();
  connectWifi();
  configureMqttClient();
  configureWebServer();
  configureArduinoOta();

  Serial.println("System ready: web server, MQTT, OTA, current sensing, DHT11");
  Serial.println("Serial command 'z' triggers zero calibration.");

  gRuntime.lastReportMs = millis();
  gLastMqttPublishMs = millis();
  gEnergy.lastPersistMs = millis();
}

void loop() {
  maintainLcdInitialization();
  handleSerialCommands();
  pollRadarPresence();
  maintainWifiFallbackRetry();
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
