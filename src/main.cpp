#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>
#include <math.h>

namespace {
constexpr uint32_t kBaudRate = 115200;
constexpr uint32_t kReportIntervalMs = 2000;
constexpr uint32_t kSamplingWindowMs = 400;
constexpr uint32_t kZeroCalibrationWindowMs = 2000;
constexpr float kAdcReferenceVoltage = 3.3F;
constexpr uint16_t kAdcMax = 4095;
constexpr char kPreferencesNamespace[] = "sct013";

struct CurrentChannel {
  const char* name;
  uint8_t pin;
  float ampsPerVolt;
};

CurrentChannel gChannels[] = {
  {"L1", 34, 0.778F},
  {"L2", 35, 0.377F},
  {"L3", 32, 0.863F},
};

constexpr float kDefaultZeroOffsetVrms[] = {
  0.203F / 50.0F,
  0.198F / 50.0F,
  0.137F / 50.0F,
};

float gZeroOffsetVrms[sizeof(gChannels) / sizeof(gChannels[0])] = {};
uint32_t gLastReportMs = 0;
Preferences gPreferences;
}

void printChipInfo() {
  esp_chip_info_t chipInfo;
  esp_chip_info(&chipInfo);

  Serial.println("=== ESP32 Startup ===");
  Serial.printf("Model: ESP32 family\n");
  Serial.printf("Cores: %u\n", chipInfo.cores);
  Serial.printf("Revision: %u\n", chipInfo.revision);
  Serial.printf("Features: ");

  if ((chipInfo.features & CHIP_FEATURE_WIFI_BGN) != 0) {
    Serial.print("WiFi ");
  }
  if ((chipInfo.features & CHIP_FEATURE_BT) != 0) {
    Serial.print("BT ");
  }
  if ((chipInfo.features & CHIP_FEATURE_BLE) != 0) {
    Serial.print("BLE ");
  }

  Serial.println();
  Serial.printf("Flash size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("CPU frequency: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.println("=====================");
}

struct ChannelMeasurement {
  float irms;
  float meanVoltage;
  float vrmsVoltage;
};

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

void loadDefaultZeroCalibration() {
  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    gZeroOffsetVrms[index] = kDefaultZeroOffsetVrms[index];
  }
}

bool loadPersistedZeroCalibration() {
  if (!gPreferences.begin(kPreferencesNamespace, true)) {
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
  if (!gPreferences.begin(kPreferencesNamespace, false)) {
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

void printZeroCalibration() {
  Serial.println("=== Zero Calibration ===");

  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    Serial.printf(
        "%s GPIO%u zero_offset=%.3f A (vrms=%.5f V)\n",
        gChannels[index].name,
        gChannels[index].pin,
        gZeroOffsetVrms[index] * gChannels[index].ampsPerVolt,
        gZeroOffsetVrms[index]);
  }

  Serial.println("========================");
}

void runZeroCalibration(bool persist) {
  Serial.println("=== Zero Calibration ===");
  Serial.println("Ensure all CT clamps have no conductor inside.");

  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    const ChannelMeasurement measurement = readChannelMeasurement(gChannels[index], kZeroCalibrationWindowMs);
    gZeroOffsetVrms[index] = measurement.vrmsVoltage;
    Serial.printf(
        "%s GPIO%u zero_offset=%.3f A (mean=%.3f V)\n",
        gChannels[index].name,
        gChannels[index].pin,
        gZeroOffsetVrms[index] * gChannels[index].ampsPerVolt,
        measurement.meanVoltage);
  }

  if (persist) {
    savePersistedZeroCalibration();
    Serial.println("Zero calibration saved.");
  }

  Serial.println("========================");
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

void printCurrentReport() {
  Serial.println("=== Current Report (A RMS) ===");

  for (size_t index = 0; index < (sizeof(gChannels) / sizeof(gChannels[0])); ++index) {
    const CurrentChannel& channel = gChannels[index];
    const ChannelMeasurement measurement = readChannelMeasurement(channel, kSamplingWindowMs);
    const float zeroOffsetAmps = gZeroOffsetVrms[index] * channel.ampsPerVolt;
    const float correctedIrms = measurement.irms > zeroOffsetAmps
                                    ? measurement.irms - zeroOffsetAmps
                                    : 0.0F;
    Serial.printf(
        "%s GPIO%u: %.3f A raw=%.3f A zero=%.3f A (mean=%.3f V)\n",
        channel.name,
        channel.pin,
        correctedIrms,
        measurement.irms,
        zeroOffsetAmps,
        measurement.meanVoltage);
  }

  Serial.printf("free_heap=%u bytes\n", ESP.getFreeHeap());
  Serial.println("==============================");
}

void setup() {
  Serial.begin(kBaudRate);
  delay(200);

  analogReadResolution(12);
  for (const CurrentChannel& channel : gChannels) {
    analogSetPinAttenuation(channel.pin, ADC_11db);
  }

  printChipInfo();
  Serial.println("Three-phase current channels active: L1=GPIO34, L2=GPIO35, L3=GPIO32");
  loadDefaultZeroCalibration();

  if (loadPersistedZeroCalibration()) {
    Serial.println("Loaded saved zero calibration.");
  } else {
    Serial.println("Using built-in zero calibration defaults.");
  }

  printZeroCalibration();
  Serial.println("Send 'z' in Serial Monitor with empty clamps to refresh zero calibration.");
  Serial.println("Adjust ampsPerVolt in source after calibration.");

  gLastReportMs = millis();
}

void loop() {
  handleSerialCommands();

  const uint32_t now = millis();
  if (now - gLastReportMs >= kReportIntervalMs) {
    gLastReportMs = now;
    printCurrentReport();
  }
}
