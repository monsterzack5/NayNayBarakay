#pragma once

#include <Arduino.h>
#include <ButtonState.h>

class Button {
public:
    Button(uint8_t pin, uint16_t debounceSeconds);
    void setDebounce(uint16_t);
    uint16_t getDebounce();
    virtual ButtonState getState();

private:
    uint8_t m_pin;
    uint16_t m_debounce;
    ButtonState m_state;
    ButtonState m_last_state;
    uint64_t m_last_swtch_time;
};
