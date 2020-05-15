#pragma once

#include <Button.h>
#include <ButtonState.h>
#include <DoorState.h>
#include <IndicatorState.h>

class MotorController {
   public:
    static void init();
    // static bool openDoor();
    // static bool closeDoor();
    static bool setDoorState(DoorState);
    static DoorState getDoorState();
    static DoorState checkLimitSwitches();
    static void openDoorFromInside();
    static bool isDoorShutWithReed();
    static void loop();
    static bool isMoving();
    static bool isAllowedToMove();
    static void setAllowedToMove(bool);
    static void indicateDoorError();

   private:
    static IndicatorState getWhatToIndicateAfterDoorOP();
    static Button m_limit_switch_open;
    static Button m_limit_switch_close;
    static DoorState m_state;
    static bool m_is_moving;
    static bool m_allowed_to_move;
};
