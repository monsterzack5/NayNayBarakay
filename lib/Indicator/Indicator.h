#pragma once

#include <IndicatorState.h>

class Indicator {
public:
    static void init();
    static void indicate(IndicatorState);
    static void indicateForThenIndicate(IndicatorState state, uint16_t showStateForMS, IndicatorState newState);
    static void stopIndicating();

private:
    static void createNewTask(TaskFunction_t);
    static TaskFunction_t getTaskFromIndiactorState(IndicatorState);
    static TaskHandle_t m_task_handler;
    static void SETUP_TASK(void*);
    static void NORMALOPERATION_TASK(void*);
    static void HOSTINGOWNWIFI_TASK(void*);
    static void DOORSTATECHANGING_TASK(void*);
    static void DOORERROR_TASK(void*);
    static void DOOROPENINGFROMINSIDE_TASK(void*);
    static void NONE_TASK(void*);
};
