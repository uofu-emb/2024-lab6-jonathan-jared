#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <unity.h>

SemaphoreHandle_t sem, sem_binary, sem_mutex;
bool is_mutex;

#define TEST_RUNNER_PRIORITY      ( tskIDLE_PRIORITY + 4UL )
#define HIGH_TASK_PRIORITY     ( tskIDLE_PRIORITY + 3UL )
#define MEDIUM_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOW_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define TEST_RUNNER_STACK_SIZE configMINIMAL_STACK_SIZE
#define OTHER_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void setUp(void)
{
    sem_binary = xSemaphoreCreateBinary();
    sem_mutex = xSemaphoreCreateMutex();
    sem = (is_mutex ? sem_mutex : sem_binary);
    xSemaphoreGive(sem_binary);
}

void tearDown(void)
{
    vSemaphoreDelete(sem_binary);
    vSemaphoreDelete(sem_mutex);
}

void royal_task(__unused void *args) {
    vTaskDelay(1000);
    while (xSemaphoreTake(sem, portMAX_DELAY) != pdTRUE);
    xSemaphoreGive(sem);
    while (true) {
        // important royal activities
    }
}

void jester_task(__unused void *args) {
    vTaskDelay(500);
    while (true); // never runs out of jokes
}

void serf_task(__unused void *args) {
    while (xSemaphoreTake(sem, 100) != pdTRUE);
    for (volatile int i = 0; i < 300000000; i++); // gone serfing
    xSemaphoreGive(sem);
    while (true);
}

void test_priority_inversion(__unused void *args) {
    TaskHandle_t royal, jester, serf;
    xTaskCreate(royal_task, "RoyalThread",
                OTHER_TASK_STACK_SIZE, NULL, HIGH_TASK_PRIORITY, &royal);
    xTaskCreate(jester_task, "JesterThread",
                OTHER_TASK_STACK_SIZE, NULL, MEDIUM_TASK_PRIORITY, &jester);
    xTaskCreate(serf_task, "SerfThread",
                OTHER_TASK_STACK_SIZE, NULL, LOW_TASK_PRIORITY, &serf);

    vTaskDelay(5000);

    // ready = 1, blocked = 2
    TEST_ASSERT_EQUAL_INT((is_mutex ? 1 : 2), eTaskGetState(royal));
    TEST_ASSERT_EQUAL_INT((is_mutex ? 1 : 1), eTaskGetState(jester));
    TEST_ASSERT_EQUAL_INT((is_mutex ? 1 : 1), eTaskGetState(serf));

    vTaskDelete(royal);
    vTaskDelete(jester);
    vTaskDelete(serf);
}

void runner_thread(__unused void *args)
{
    for (;;) {
        printf("Starting test run\n");
        UNITY_BEGIN();
        is_mutex = false;
        RUN_TEST(test_priority_inversion);
        is_mutex = true;
        RUN_TEST(test_priority_inversion);
        UNITY_END();
        vTaskDelay(10000);
    }
}

int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000);
    printf("Launching runner\n");
    xTaskCreate(runner_thread, "TestRunner",
                TEST_RUNNER_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;
}
