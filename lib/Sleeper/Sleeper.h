#pragma once

#include <Arduino.h>
#include <esp_sleep.h>

class Sleeper {
public:
    static void checkWiFiLoop();
    static void deepSleepSeconds(uint16_t);
    static uint8_t interruptWakeupPin();
};
