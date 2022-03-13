#include <Arduino.h>
#include <Button.h>
#include <ButtonState.h>
#include <Config.h>
#include <Log.h>

Button::Button(uint8_t pin, uint16_t debounceSeconds = 25)
{
    m_pin = pin;
    m_debounce = debounceSeconds * S_TO_uSECONDS;
    m_state = ButtonState::UNKNOWN;
    m_last_state = ButtonState::UNKNOWN;
    m_last_swtch_time = 0;
    getState();
}

void Button::setDebounce(uint16_t debounceSeconds)
{
    m_debounce = debounceSeconds * S_TO_uSECONDS;
}

uint16_t Button::getDebounce()
{
    return m_debounce;
}

ButtonState Button::getState()
{
    m_state = digitalRead(m_pin) ? ButtonState::OPEN : ButtonState::PRESSED;
    if (m_state != m_last_state) {
        uint64_t currentTime = esp_timer_get_time();
        if (currentTime - m_last_swtch_time >= m_debounce) {
            m_last_swtch_time = currentTime;
            m_last_state = m_state;
            return m_state;
        } else
            return m_last_state;
    }
    return m_state;
}
