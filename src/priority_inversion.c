/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

SemaphoreHandle_t sem;

#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 4UL )
#define HIGH_TASK_PRIORITY     ( tskIDLE_PRIORITY + 3UL )
#define MEDIUM_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOW_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define OTHER_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void royal_task(__unused void *params) {
    vTaskDelay(1000);
    while (xSemaphoreTake(sem, 100) != pdTRUE);
    xSemaphoreGive(sem);
    while (true) {
        // important royal activities
    }
}

void jester_task(__unused void *params) {
    vTaskDelay(500);
    while (true); // never runs out of jokes
}

void serf_task(__unused void *params) {
    while (xSemaphoreTake(sem, 100) != pdTRUE);
    for (int i = 0; i < 300000000; i++); // gone serfing
    xSemaphoreGive(sem);
    while (true);
}

void main_task(__unused void *params) {
    TaskHandle_t royal, jester, serf;
    xTaskCreate(royal_task, "RoyalThread",
                OTHER_TASK_STACK_SIZE, NULL, HIGH_TASK_PRIORITY, &royal);
    xTaskCreate(jester_task, "JesterThread",
                OTHER_TASK_STACK_SIZE, NULL, MEDIUM_TASK_PRIORITY, &jester);
    xTaskCreate(serf_task, "SerfThread",
                OTHER_TASK_STACK_SIZE, NULL, LOW_TASK_PRIORITY, &serf);

    while (true) {
        // ready = 1, blocked = 2
        printf("Royal: %d\nJester: %d\nSerf: %d\n\n", eTaskGetState(royal), eTaskGetState(jester), eTaskGetState(serf));
        vTaskDelay(500);
    }
}

int main( void )
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sem = xSemaphoreCreateBinary();
    const char *rtos_name;
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);
    vTaskStartScheduler();
    return 0;
}
