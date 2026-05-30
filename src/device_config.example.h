#ifndef X42_DEVICE_CONFIG_EXAMPLE_H
#define X42_DEVICE_CONFIG_EXAMPLE_H

// Copy this file to src/device_config.h for per-device build defaults.
// src/device_config.h is gitignored so production credentials stay local.

#define X42_CFG_ADMIN_USER "admin"
#define X42_CFG_ADMIN_PASSWORD "admin1234"
#define X42_CFG_WIFI_SSID ""
#define X42_CFG_WIFI_PASSWORD ""
#define X42_CFG_HOSTNAME "x42-whirlpool-station"

#define X42_CFG_MQTT_SERVER ""
#define X42_CFG_MQTT_PORT 1883
#define X42_CFG_MQTT_USER ""
#define X42_CFG_MQTT_PASSWORD ""
#define X42_CFG_MQTT_BASE_TOPIC "whirlpool/station"
#define X42_CFG_NTP_SERVER "pool.ntp.org"
#define X42_CFG_TZ_OFFSET_MIN 60
#define X42_CFG_TEMP_OFFSET_C 0.0F
#define X42_CFG_HUMIDITY_OFFSET_PCT 0.0F

// Flash policy: when firmware revision changes, choose whether build defaults
// from device_config should overwrite persisted config for each update path.
#define X42_CFG_APPLY_ON_USB_FLASH 1
#define X42_CFG_APPLY_ON_OTA_FLASH 1

#define X42_CFG_PUBLISH_INTERVAL_SEC 5

#define X42_CFG_SENSOR_L1 "current_l1"
#define X42_CFG_SENSOR_L2 "current_l2"
#define X42_CFG_SENSOR_L3 "current_l3"
#define X42_CFG_SENSOR_TEMP "temp_outdoor"
#define X42_CFG_SENSOR_HUMIDITY "humidity_outdoor"

#endif
