#include <Arduino.h>
#include <Config.h>
#include <Log.h>
#include <Sleeper.h>
#include <WiFi.h>
#include <WiFiHelper.h>

void Sleeper::checkWiFiLoop() {
    if (RTC_WiFiRetriesCount > WiFi_TRY_TO_CONNECT_THEN_SLEEP_LIMIT) {
        DEBUG_PRINTLN("Exausted retries, Going into long Deep Sleep");
        RTC_WiFiRetriesCount = 0;
        deepSleepSeconds(SLEEPER_LONG_DEEP_SLEEP_SECONDS);
    }

    DEBUG_PRINT("Wifi not found, sleeping then retrying, retry count: ");
    DEBUG_PRINTLN(RTC_WiFiRetriesCount);
    RTC_WiFiRetriesCount += 1;
    deepSleepSeconds(SLEEPER_SHORT_DEEP_SLEEP_SECONDS);
}

void Sleeper::deepSleepSeconds(uint16_t sleep_time) {
    ESP.deepSleep(sleep_time * S_TO_uSECONDS);
}

uint8_t Sleeper::interruptWakeupPin() {
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if (wakeupBit & (1LL << insideDoorOpen)) {
        DEBUG_PRINTLN("Woken up using insideDoorOpen");
        return insideDoorOpen;
    }
    if (wakeupBit & (1LL << outsideESPWakeup)) {
        DEBUG_PRINTLN("Woken up using outsideESPWakeup as a button");
        return outsideESPWakeup;
    }
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD) {
        DEBUG_PRINTLN("Woken up using outsideESPWakeup as a touch sensor");
        return outsideESPWakeup;
    }
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        DEBUG_PRINTLN("ESP Woken up from a timer");
        return 0;
    }
    DEBUG_PRINTLN("ESP Did Not wake from an interrupt");
    return 0;
}
