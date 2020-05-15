#pragma once

enum class IndicatorState : uint8_t {
    NONE,
    SETUP,
    NORMALOPERATION,
    HOSTINGOWNWIFI,
    DOORSTATECHANGING,
    DOORERROR,
    DOOROPENINGFROMINSIDE
};
