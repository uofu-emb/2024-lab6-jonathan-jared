#include "helper.h"

void busy_busy(void)
{
    for (int i = 0; ; i++);
}

void busy_yield(void)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}

void simple_task(void *args)
{
    ((void (*)(void)) args)();
}

void delayed_task(void *args)
{
    vTaskDelay(100);
    ((void (*)(void)) args)();
}

void measure_runtime(void (*func0)(void), UBaseType_t priority0,
                     void (*func1)(void), UBaseType_t priority1,
                     configRUN_TIME_COUNTER_TYPE *runtime0, configRUN_TIME_COUNTER_TYPE *runtime1)
{
    TaskHandle_t handle0, handle1;
    xTaskCreate(simple_task, "Thread0", configMINIMAL_STACK_SIZE, func0, priority0, &handle0);
    xTaskCreate(delayed_task, "Thread1", configMINIMAL_STACK_SIZE, func1, priority1, &handle1);

    vTaskDelay(2000);

    TaskStatus_t status;
    vTaskGetInfo(handle0, &status, pdFALSE, eInvalid);
    *runtime0 = status.ulRunTimeCounter;
    vTaskGetInfo(handle1, &status, pdFALSE, eInvalid);
    *runtime1 = status.ulRunTimeCounter;

    vTaskDelete(handle0);
    vTaskDelete(handle1);
}