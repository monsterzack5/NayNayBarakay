#pragma once

enum class DoorStatus : uint8_t {
    ERROR,
    DOOROPEN,
    DOOROPENING,
    DOORCLOSED,
    DOORCLOSING,
    DOORFLOATING
};
