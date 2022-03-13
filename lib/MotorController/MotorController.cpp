#include <Arduino.h>
#include <Button.h>
#include <ButtonState.h>
#include <Config.h>
#include <DoorState.h>
#include <Indicator.h>
#include <IndicatorState.h>
#include <Log.h>
#include <MotorController.h>
#include <WiFiHelper.h>

Button MotorController::m_limit_switch_open = Button(limitSwitchOpen, 20);
Button MotorController::m_limit_switch_close = Button(limitSwitchClose, 20);
// Used so we know when the barricade is moving
bool MotorController::m_is_moving = false;
// Always checked when moving the door, use as a hard STOP
bool MotorController::m_allowed_to_move = true;
// Used to know if the barricade is Opening/Closing/Floating
DoorState MotorController::m_state = DoorState::DOORFLOATING;

void MotorController::init()
{
    DEBUG_PRINTLN("MotorController::init()");

    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::OPEN) {
        // We're here if they are both OPEN on boot,
        // so lets try to close the door
        DEBUG_PRINTLN("Both Switches are OPEN on BOOT, Trying to Close the Door");
        setDoorState(DoorState::DOORCLOSED);
    }
}

bool MotorController::setDoorState(DoorState newState)
{
    if (newState != DoorState::DOOROPEN && newState != DoorState::DOORCLOSED) {
        DEBUG_PRINTLN("MotorController::setDoorState called with an invaild new state");
        return false;
    }

    DEBUG_PRINT("Changing barricade State To: ");
    DEBUG_PRINTLN((newState == DoorState::DOOROPEN) ? "DOOROPEN" : "DOORCLOSED");

    if (!isDoorShutWithReed() || !m_allowed_to_move || m_is_moving || (checkLimitSwitches() == newState)) {
        DEBUG_PRINTLN("Not moving the barricade as: ");
        if (!isDoorShutWithReed())
            DEBUG_PRINTLN("    The Door is not physically closed");
        if (!m_allowed_to_move)
            DEBUG_PRINTLN("    Movement was disabled from the WebUI");
        if (m_is_moving)
            DEBUG_PRINTLN("    The barricade is already moving!");
        if (checkLimitSwitches() == newState)
            DEBUG_PRINTLN("    The barricade is already at the new position");

        indicateDoorError();
        return false;
    }

    m_is_moving = true;
    uint8_t pinToActOn;
    DoorState oldState;

    if (newState == DoorState::DOOROPEN) {
        m_state = DoorState::DOOROPENING;
        oldState = DoorState::DOORCLOSED;
        pinToActOn = hBridgeOpen;
    } else {
        m_state = DoorState::DOORCLOSING;
        oldState = DoorState::DOOROPEN;
        pinToActOn = hBridgeClose;
    }

    Indicator::indicate(IndicatorState::DOORSTATECHANGING);

    bool barrierTimedOut = false;
    uint64_t startTime = esp_timer_get_time();
    DoorState currentDoorState = checkLimitSwitches();
    while (currentDoorState != newState && m_allowed_to_move) {
        currentDoorState = checkLimitSwitches();
        digitalWrite(pinToActOn, HIGH);
        // Stop the barricade if the opposing button is pressed after 1 second, Or
        // timeout after 20 seconds of moving the barricade
        if ((esp_timer_get_time() > startTime + 1000000 && currentDoorState == oldState) || esp_timer_get_time() > startTime + 20000000) {
            barrierTimedOut = true;
            break;
        }
    }

    digitalWrite(pinToActOn, LOW);
    m_is_moving = false;

    if (!m_allowed_to_move || barrierTimedOut) {
        DEBUG_PRINTLN("WARN: Barrier stop condition occured, hard stopping!");
        m_state = DoorState::DOORFLOATING;
        indicateDoorError();
        return false;
    }

    m_state = newState;
    DEBUG_PRINT("barricade ");
    DEBUG_PRINTLN((newState == DoorState::DOOROPEN) ? "Opened!" : "Closed!");

    Indicator::indicate(getWhatToIndicateAfterDoorOP());
    return true;
}

/* This function will only ever return the KNOWN status of the barricade via the limit switches */
DoorState MotorController::checkLimitSwitches()
{
    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::PRESSED) {
        return DoorState::DOOROPEN;
    }
    if (m_limit_switch_close.getState() == ButtonState::PRESSED && m_limit_switch_open.getState() == ButtonState::OPEN) {
        return DoorState::DOORCLOSED;
    }
    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::OPEN) {
        // if Both switches are OPEN, the barricade is floating
        return DoorState::DOORFLOATING;
    }
    if (m_limit_switch_close.getState() == ButtonState::PRESSED && m_limit_switch_open.getState() == ButtonState::PRESSED) {
        // if both switches are pressed, report an ERROR
        return DoorState::ERROR;
    }
    // I have literally no idea why we end up here sometimes
    return checkLimitSwitches();
}

// The barricade Operations will indicate DOORSTATECHANGING, So we should
// indicate DOOROPENINGFROMINSIDE After, well its waiting for the door
// to open, then close.
void MotorController::changeDoorStateAndWaitForDoor()
{
    DEBUG_PRINTLN("in changeDoorStateAndWaitForDoor");

    if (getDoorState() == DoorState::DOOROPEN && isDoorShutWithReed()) {
        setDoorState(DoorState::DOORCLOSED);
        return;
    }

    if (!setDoorState(DoorState::DOOROPEN))
        return;
    Indicator::indicate(IndicatorState::DOOROPENINGFROMINSIDE);

    // wait for the reed switch to say the barricade is open
    // then wait for the reed switch to say the barricade is closed.
    DEBUG_PRINTLN("Waiting for the Reed switch to indicate DOOROPEN");
    while (isDoorShutWithReed()) {
        delay(100);
    }
    DEBUG_PRINTLN("Waiting for the Reed switch to indicate DOORCLOSED");
    while (!isDoorShutWithReed()) {
        delay(100);
    }

    // Wait 1 second, then close the door
    delay(1000);

    if (!setDoorState(DoorState::DOORCLOSED))
        return;
}

// We Loop this function on every single cycle, so we always
// can known the current state of the limit switches
void MotorController::loop()
{
    // if we're moving, state is being handled elsewhere
    if (!m_is_moving) {
        DoorState status = checkLimitSwitches();
        if (status != m_state) {
            m_state = status;
        }
        if (digitalRead(insideDoorOpen) == HIGH) {
            DEBUG_PRINTLN("insideDoorOpen Button Pressed");
            changeDoorStateAndWaitForDoor();
        }
    }
}

IndicatorState MotorController::getWhatToIndicateAfterDoorOP()
{
    switch (WiFi.getMode()) {
    case WIFI_MODE_AP:
        return IndicatorState::HOSTINGOWNWIFI;
    case WIFI_MODE_STA:
        return IndicatorState::NORMALOPERATION;
    default:
        return IndicatorState::NONE;
    }
}

void MotorController::indicateDoorError()
{
    Indicator::indicateForThenIndicate(IndicatorState::DOORERROR, 5000, getWhatToIndicateAfterDoorOP());
}

DoorState MotorController::getDoorState()
{
    return m_state;
}

bool MotorController::isDoorShutWithReed()
{
    return digitalRead(doorReedSwitch) == LOW;
}

bool MotorController::isMoving()
{
    return m_is_moving;
}

void MotorController::setAllowedToMove(bool value)
{
    m_allowed_to_move = value;
}

bool MotorController::isAllowedToMove()
{
    return m_allowed_to_move;
}
