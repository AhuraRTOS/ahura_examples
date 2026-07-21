/**
 * @file os_main_queue.c
 * @brief Ahura kernel example: message queue (os_queue_*).
 *
 * os_main is the producer: it sends an incrementing value every 300 ms. A
 * higher-priority consumer task blocks in os_queue_receive() and drains
 * each item as soon as it arrives. Copy this file into the application
 * source tree as os_main.c to run it; needs OS_CONFIG_QUEUE_ENABLE=1 in
 * os_config.h (the default).
 *
 * @copyright (c) 2026 Ahura Project Contributors
 *            SPDX-License-Identifier: MIT
 *            See LICENSE.md in the project root for the full license text.
 */

/*
 * ***********************************************************************************************************
 * Includes
 * ***********************************************************************************************************
*/

#include "ahura.h"

#include <stdio.h>

#if !(OS_CONFIG_QUEUE_ENABLE == 1U)
#error "os_main_queue.c needs OS_CONFIG_QUEUE_ENABLE=1 in os_config.h"
#endif

/*
 * ***********************************************************************************************************
 * Private objects
 * ***********************************************************************************************************
*/

OS_TASK_DEFINE(consumer, 512U);

#define QUEUE_CAPACITY 4U

static os_queue_t g_queue;
static uint32_t   g_queue_buffer[QUEUE_CAPACITY];

/*
 * ***********************************************************************************************************
 * Private function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
static void consumer_entry(void *context)
{
    (void)context;

    while (1)
    {
        uint32_t value;

        if (os_queue_receive(&g_queue, &value, OS_WAIT_FOREVER) == OS_STATUS_OK)
        {
            printf("[queue] consumer received %lu\r\n", (unsigned long)value);
        }
    }
}

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Default application task body: sends values for a higher-priority consumer to receive.
 *
 * @return None.
 */
void os_main(void)
{
    uint32_t next_value = 0U;

    (void)os_queue_init(&g_queue, g_queue_buffer, sizeof(g_queue_buffer[0]), QUEUE_CAPACITY);
    (void)os_task_create(&consumer, OS_TASK_CONFIG(consumer, consumer_entry, NULL, OS_TASK_PRIO_2));
    (void)os_task_start(&consumer);

    while (1)
    {
        printf("[queue] producer sending %lu (count=%lu before send)\r\n", (unsigned long)next_value,
               (unsigned long)os_queue_count_get(&g_queue));
        (void)os_queue_send(&g_queue, &next_value, OS_WAIT_FOREVER);
        next_value++;
        (void)os_delay_ms(300U);
    }
}
