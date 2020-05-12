#include <Arduino.h>
#include <Button.h>
#include <ButtonState.h>
#include <Config.h>
#include <DoorStatus.h>
#include <Indicator.h>
#include <IndicatorState.h>
#include <Log.h>
#include <MotorController.h>
#include <WiFiHelper.h>

Button MotorController::m_limit_switch_open = Button(limitSwitchOpen, 20);
Button MotorController::m_limit_switch_close = Button(limitSwitchClose, 20);
// Used so we know when the Door is moving
bool MotorController::m_is_moving = false;
// Always checked when moving the door, use as a hard STOP
bool MotorController::m_allowed_to_move = true;
// Whether or not we should allow input from web commands
bool MotorController::m_allow_web_commands = true;
// Used to know if the door is Opening/Closing/Idle
DoorStatus MotorController::m_state = DoorStatus::DOORFLOATING;

void MotorController::init() {
    DEBUG_PRINTLN("MotorController::init()");

    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::OPEN) {
        // We're here if they are both OPEN on boot,
        // so lets try to close the door
        DEBUG_PRINTLN("Both Switches are OPEN on BOOT, Trying to Close the Door");
        closeDoor();
    }
}

bool MotorController::openDoor() {
    MotorController::m_allow_web_commands = false;
    DEBUG_PRINTLN("MOTORCONTROLLER::OPENDOOR()");
    if (!isDoorShutWithReed() || !MotorController::m_allowed_to_move) {
        DEBUG_PRINTLN("Not Opening door as its either not shut with reed, or isnt allowed to move");
        Indicator::indicateForThenIndicate(IndicatorState::DOORNOTSHUTWITHREED, 3000, getWhatToIndicateAfterDoorOP());
        return false;
    }

    if (!MotorController::m_is_moving && checkLimitSwitches() != DoorStatus::DOOROPEN) {
        MotorController::m_is_moving = true;
        MotorController::m_state = DoorStatus::DOOROPENING;
        Indicator::indicate(IndicatorState::DOORSTATECHANGING);
        DEBUG_PRINTLN("Trying to Open the Door!");

        // Write the relay pin High first, then wait, as to not burn it out
        digitalWrite(hBridgeRelay, HIGH);
        delay(75);

        while (checkLimitSwitches() != DoorStatus::DOOROPEN && MotorController::m_allowed_to_move == true) {
            digitalWrite(hBridgeOpen, HIGH);
        }
        digitalWrite(hBridgeOpen, LOW);
        delay(75);
        digitalWrite(hBridgeRelay, LOW);


        MotorController::m_is_moving = false;
        MotorController::m_allow_web_commands = true;
        MotorController::m_state = DoorStatus::DOOROPEN;
        DEBUG_PRINTLN("Door Opened!");
    }
    Indicator::indicate(getWhatToIndicateAfterDoorOP());
    return true;
}

bool MotorController::closeDoor() {
    MotorController::m_allow_web_commands = false;
    DEBUG_PRINTLN("MOTORCONTROLLER::CLOSEDOOR()");

    if (!isDoorShutWithReed() || !MotorController::m_allowed_to_move) {
        DEBUG_PRINTLN("Not Closing door as its either not shut with reed, or isnt allowed to move");
        Indicator::indicateForThenIndicate(IndicatorState::DOORNOTSHUTWITHREED, 5000, getWhatToIndicateAfterDoorOP());
        return false;
    }

    if (!MotorController::m_is_moving && checkLimitSwitches() != DoorStatus::DOORCLOSED) {
        MotorController::m_is_moving = true;
        MotorController::m_state = DoorStatus::DOORCLOSING;
        Indicator::indicate(IndicatorState::DOORSTATECHANGING);
        DEBUG_PRINTLN("Trying to Close the Door!");

        while (checkLimitSwitches() != DoorStatus::DOORCLOSED && MotorController::m_allowed_to_move == true) {
            digitalWrite(hBridgeClose, HIGH);
        }
        digitalWrite(hBridgeClose, LOW);
        MotorController::m_is_moving = false;
        MotorController::m_allow_web_commands = true;
        MotorController::m_state = DoorStatus::DOORCLOSED;
        DEBUG_PRINTLN("Door Closed!");
    }
    Indicator::indicate(getWhatToIndicateAfterDoorOP());
    return true;
}

/* This function will only ever return the KNOWN status of the door via the limit switches */
DoorStatus MotorController::checkLimitSwitches() {
    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::PRESSED) {
        return DoorStatus::DOOROPEN;
    }
    if (m_limit_switch_close.getState() == ButtonState::PRESSED && m_limit_switch_open.getState() == ButtonState::OPEN) {
        return DoorStatus::DOORCLOSED;
    }
    if (m_limit_switch_close.getState() == ButtonState::OPEN && m_limit_switch_open.getState() == ButtonState::OPEN) {
        // if Both switches are OPEN, the door is floating
        return DoorStatus::DOORFLOATING;
    }
    if (m_limit_switch_close.getState() == ButtonState::PRESSED && m_limit_switch_open.getState() == ButtonState::PRESSED) {
        // if both switches are pressed, report an ERROR
        return DoorStatus::ERROR;
    }
    DEBUG_PRINTLN("Some combination of possible button states are not being handled");
    // I have literally no idea why we end up here sometimes
    return checkLimitSwitches();
}

void MotorController::openDoorFromInside() {
    DEBUG_PRINTLN("Opening Door from inside");
    if (!openDoor()) return;
    DEBUG_PRINTLN("Door Opened from inside, waiting then closing the door");
    delay(5000);
    if (!closeDoor()) return;
}

// We Loop this function on every single cycle, so we always
// can known the current state of the limit switches
void MotorController::loop() {
    // if we're moving, state is being handled elsewhere
    if (!MotorController::m_is_moving) {
        DoorStatus status = checkLimitSwitches();
        if (status != MotorController::m_state) {
            MotorController::m_state = status;
        }
        if (digitalRead(insideDoorOpen) == HIGH) {
            DEBUG_PRINTLN("insideDoorOpen Button Pressed");
            openDoorFromInside();
        }
    }
}

IndicatorState MotorController::getWhatToIndicateAfterDoorOP() {
    switch (WiFi.getMode()) {
        case WIFI_MODE_AP:
            return IndicatorState::HOSTINGOWNWIFI;
        case WIFI_MODE_STA:
            return IndicatorState::NORMALOPERATION;
        default:
            return IndicatorState::NONE;
    }
}

DoorStatus MotorController::getDoorState() {
    return MotorController::m_state;
}

bool MotorController::isDoorShutWithReed() {
    return digitalRead(doorReedSwitch) == LOW;
}

bool MotorController::isMoving() {
    return MotorController::m_is_moving;
}

void MotorController::setAllowedToMove(bool value) {
    MotorController::m_allowed_to_move = value;
}

bool MotorController::isAllowedToMove() {
    return MotorController::m_allowed_to_move;
}

bool MotorController::areWebCommandsAllowed() {
    return MotorController::m_allow_web_commands;
}
