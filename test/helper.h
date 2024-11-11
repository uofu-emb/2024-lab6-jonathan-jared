#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>

void busy_busy(void);

void busy_yield(void);

void measure_runtime(void (*func0)(void), UBaseType_t priority0,
                     void (*func1)(void), UBaseType_t priority1,
                     configRUN_TIME_COUNTER_TYPE *runtime0, configRUN_TIME_COUNTER_TYPE *runtime1);