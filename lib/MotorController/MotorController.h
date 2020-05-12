#pragma once

#include <Button.h>
#include <ButtonState.h>
#include <DoorStatus.h>
#include <IndicatorState.h>

class MotorController {
   public:
    static void init();
    static bool openDoor();
    static bool closeDoor();
    static DoorStatus getDoorState();
    static DoorStatus checkLimitSwitches();
    static void openDoorFromInside();
    static bool isDoorShutWithReed();
    static void loop();
    static bool isMoving();
    static bool isAllowedToMove();
    static void setAllowedToMove(bool);
    static bool areWebCommandsAllowed();

   private:
    static IndicatorState getWhatToIndicateAfterDoorOP();
    static Button m_limit_switch_open;
    static Button m_limit_switch_close;
    static DoorStatus m_state;
    static bool m_is_moving;
    static bool m_allowed_to_move;
    static bool m_allow_web_commands;
};
