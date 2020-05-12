#pragma once

#include <ESPAsyncWebServer.h>
#include <IPAddress.h>

#define S_TO_mSECONDS 1000
#define S_TO_uSECONDS 1000000

extern const char* SSID;
extern const char* PASSWORD;
extern const char* ESP_LOCAL_AP_SSID;
extern const char* ESP_LOCAL_AP_PASSWORD;
extern const IPAddress ESP_LOCAL_IP;
extern const bool ESP_SHOULD_LOCAL_WIFI_BE_HIDDEN;
extern const uint8_t ESP_LOCAL_NETWORK_CHANNEL_ID;
extern const char* AUTH_LOGIN_USERNAME;
extern const char* AUTH_LOGIN_PASSWORD;
extern const char* AUTH_COOKIE_NAME;
extern const uint8_t hBridgeOpen;
extern const uint8_t hBridgeRelay;
extern const uint8_t hBridgeClose;
extern const uint8_t insideDoorOpen;
extern const uint8_t outsideESPWakeup;
extern const uint8_t limitSwitchOpen;
extern const uint8_t limitSwitchClose;
extern const uint8_t doorReedSwitch;
extern const uint8_t indicatorPin;
extern uint8_t RTC_WiFiRetriesCount;
extern bool RTC_ShouldHostOwnNetwork;
extern const uint16_t SLEEPER_LONG_DEEP_SLEEP_SECONDS;
extern const uint16_t SLEEPER_SHORT_DEEP_SLEEP_SECONDS;
extern const uint8_t WiFi_TRY_TO_CONNECT_THEN_SLEEP_LIMIT;
extern const uint16_t WIFI_HOW_LONG_TO_HOST_OWN_WIFI_SECONDS;
extern const uint16_t WIFI_HOW_OFTEN_TO_CHECK_WIFI_STILL_RUNNING_SECONDS;
extern const uint16_t INDICATOR_SHORT_BLINK_TIME_MS;
extern const uint16_t INDICATOR_LONG_BLINK_TIME_MS;
extern const uint16_t INDICATOR_WAIT_BETWEEN_IMPORTANT_BLINK_CODES_MS;
extern const uint16_t INDICATOR_WAIT_BETWEEN_NONIMPORTANT_BLINK_CODES_MS;
extern const uint8_t OUTSIDE_WAKE_BUTTON_TOUCH_THRESHOLD;