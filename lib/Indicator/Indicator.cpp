#include <Arduino.h>
#include <Config.h>
#include <Indicator.h>
#include <IndicatorState.h>
#include <Log.h>

/**
 * One thing to note about the Arduino `delay()` function
 * is that the ESP32_HAL file defines it as:
 * void delay(ms) { vTaskDelay(ms / portTICK_PERIOD_MS ); }
 * which means it will only delay the current thread,
 * not the whole chip/cpu core.
 * 
 * Another thing to note is that since freeRTOS is a realtime OS
 * the actual delay timings might be incorrect by a few milliseconds
 * but that does not really matter much for this purpose.
*/

TaskHandle_t Indicator::m_task_handler = NULL;

void Indicator::init() {
    createNewTask(SETUP_TASK);
}

void Indicator::createNewTask(TaskFunction_t task) {
    if (Indicator::m_task_handler != NULL)
        vTaskDelete(Indicator::m_task_handler);
    xTaskCreatePinnedToCore(task, "IndicatorTask", 1000, NULL, 1, &m_task_handler, 0);
}

void Indicator::indicate(IndicatorState state) {
    createNewTask(getTaskFromIndiactorState(state));
}

// immediately indicates the `state` for `showStateForMS`, then switches the state to `newState`
void Indicator::indicateForThenIndicate(IndicatorState state, uint16_t showStateForMS, IndicatorState newState) {
    indicate(state);
    delay(showStateForMS);
    indicate(newState);
}

TaskFunction_t Indicator::getTaskFromIndiactorState(IndicatorState state) {
    switch (state) {
        case IndicatorState::SETUP:
            DEBUG_PRINTLN("Trying to indicate: SETUP");
            return SETUP_TASK;
        case IndicatorState::NORMALOPERATION:
            DEBUG_PRINTLN("Trying to indicate: NORMALOPERATION");
            return NORMALOPERATION_TASK;
        case IndicatorState::HOSTINGOWNWIFI:
            DEBUG_PRINTLN("Trying to indicate: HOSTINGOWNWIFI");
            return HOSTINGOWNWIFI_TASK;
        case IndicatorState::DOORSTATECHANGING:
            DEBUG_PRINTLN("Trying to indicate: DOORSTATECHANING");
            return DOORSTATECHANGING_TASK;
        case IndicatorState::DOORNOTSHUTWITHREED:
            DEBUG_PRINTLN("Trying to indicate: DOORNOTSHUTWITHREED");
            return DOORNOTSHUTWITHREED_TASK;
        case IndicatorState::NONE:
            DEBUG_PRINTLN("Trying to indicate: NONE");
            return NONE_TASK;
        default:
            DEBUG_PRINTLN("Indicator Called with an Invalid state");
            return NONE_TASK;
    }
}

/* Indicator Tasks, These are meant to run forever in a seperate thread on Core 0 */

// 2 short blinks
void Indicator::SETUP_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_WAIT_BETWEEN_IMPORTANT_BLINK_CODES_MS);
    }
}

// 1 short blink
void Indicator::NORMALOPERATION_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_WAIT_BETWEEN_NONIMPORTANT_BLINK_CODES_MS);
    }
}

// 1 short, 1 long blink
void Indicator::HOSTINGOWNWIFI_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_SHORT_BLINK_TIME_MS);
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_LONG_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_WAIT_BETWEEN_NONIMPORTANT_BLINK_CODES_MS);
    }
}

// 1 long blink
void Indicator::DOORSTATECHANGING_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, HIGH);
        delay(INDICATOR_LONG_BLINK_TIME_MS);
        digitalWrite(indicatorPin, LOW);
        delay(INDICATOR_WAIT_BETWEEN_IMPORTANT_BLINK_CODES_MS);
    }
}

void Indicator::DOORNOTSHUTWITHREED_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, HIGH);
        // We delay here so we don't starve core 0 of cycles
        vTaskDelay(portMAX_DELAY);
    }
}

void Indicator::NONE_TASK(void* args) {
    while (true) {
        digitalWrite(indicatorPin, LOW);
        vTaskDelay(portMAX_DELAY);
    }
}
