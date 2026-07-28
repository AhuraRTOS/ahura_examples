/**
 * @file os_main_work.c
 * @brief Ahura kernel example: deferrable work items (os_work_*).
 *
 * Submits a work item that runs after a delay on the kernel work task
 * (tsk_work), then submits a second one and cancels it before it gets a
 * chance to run - os_work_is_pending() shows the state either way. Copy
 * this file into the application source tree as os_main.c to run it; needs
 * OS_CONFIG_WORK_ENABLE=1 in os_config.h (the default).
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

#if !(OS_CONFIG_WORK_ENABLE == 1U)
#error "os_main_work.c needs OS_CONFIG_WORK_ENABLE=1 in os_config.h"
#endif

/*
 * ***********************************************************************************************************
 * Private objects
 * ***********************************************************************************************************
*/

static os_work_t         g_work;
static __IO uint32_t g_work_run_count = 0U;

/*
 * ***********************************************************************************************************
 * Private function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
static void work_handler(void *context)
{
    (void)context;
    g_work_run_count++;
    printf("[work] handler ran (count=%lu)\r\n", (unsigned long)g_work_run_count);
}

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Default application task body: submits work, then submits-and-cancels work.
 *
 * @return None.
 */
void os_main(void)
{
    (void)os_work_init(&g_work, work_handler, NULL);

    while (1)
    {
        printf("[work] submitting work (runs in 200 ms)\r\n");
        (void)os_work_submit(&g_work, 200U);
        (void)os_delay_ms(50U);
        printf("[work] is_pending=%d\r\n", (int)os_work_is_pending(&g_work));
        (void)os_delay_ms(500U);

        printf("[work] submitting work then cancelling before it runs\r\n");
        (void)os_work_submit(&g_work, 500U);
        (void)os_delay_ms(50U);
        (void)os_work_cancel(&g_work);
        printf("[work] cancelled: is_pending=%d\r\n", (int)os_work_is_pending(&g_work));
        (void)os_delay_ms(2000U);
    }
}
