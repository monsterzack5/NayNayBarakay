#pragma once

enum class DoorState : uint8_t {
    ERROR,
    DOOROPEN,
    DOOROPENING,
    DOORCLOSED,
    DOORCLOSING,
    DOORFLOATING
};
