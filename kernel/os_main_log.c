/**
 * @file os_main_log.c
 * @brief Ahura kernel example: buffered debug logging (OS_LOG_*).
 *
 * Two tasks log at different rates while os_main logs a heartbeat, showing that a log call
 * costs the caller almost nothing: it formats into a ring buffer and returns, and the kernel
 * log task transmits in the background through os_log_output_cb (implement that in os_cb.c).
 *
 * The fast task deliberately outruns a slow serial port to demonstrate the drop counter: lines
 * that do not fit are dropped whole and reported, rather than blocking the task or writing a
 * half line. Copy this file into the application source tree as os_main.c to run it; needs
 * OS_CONFIG_LOG_ENABLE=1 in os_config.h (the default).
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

#if !(OS_CONFIG_LOG_ENABLE == 1U)
#error "os_main_log.c needs OS_CONFIG_LOG_ENABLE=1 in os_config.h"
#endif

/*
 * ***********************************************************************************************************
 * Private objects
 * ***********************************************************************************************************
*/

OS_TASK_DEFINE(sensor, 512U);
OS_TASK_DEFINE(chatty, 512U);

/*
 * ***********************************************************************************************************
 * Private function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Logs a plausible sensor reading once a second, at a mix of severities.
 */
static void sensor_entry(void *context)
{
    uint32_t reading = 0U;

    (void)context;

    while (1)
    {
        reading = (reading + 7U) % 100U;

        if (reading > 90U)
        {
            OS_LOG_ERROR("sensor over range: %lu", (unsigned long)reading);
        }
        else if (reading > 70U)
        {
            OS_LOG_WARN("sensor high: %lu", (unsigned long)reading);
        }
        else
        {
            OS_LOG_INFO("sensor = %lu", (unsigned long)reading);
        }

        (void)os_delay_ms(1000U);
    }
}

/******************************************************************************************************/
/**
 * @brief Logs far faster than a serial port can drain, so the ring fills and the kernel starts
 *        dropping lines. Watch for the "log lines dropped" notice: the task itself never blocks
 *        and never slows down, which is the whole point of buffering.
 */
static void chatty_entry(void *context)
{
    uint32_t counter = 0U;

    (void)context;

    while (1)
    {
        OS_LOG_DEBUG("chatty tick %lu (dropped so far: %lu)",
                     (unsigned long)counter, (unsigned long)os_log_dropped_get());
        counter++;

        (void)os_delay_ms(5U);
    }
}

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Default application task body: starts the two logging tasks, then logs a heartbeat.
 *
 * @return None.
 */
void os_main(void)
{
    OS_LOG_INFO("kernel up, log level compiled in at %u", (unsigned)OS_CONFIG_LOG_LEVEL);

    (void)os_task_create(&sensor, OS_TASK_CONFIG(sensor, sensor_entry, NULL, OS_TASK_PRIO_3));
    (void)os_task_start(&sensor);

    (void)os_task_create(&chatty, OS_TASK_CONFIG(chatty, chatty_entry, NULL, OS_TASK_PRIO_2));
    (void)os_task_start(&chatty);

    while (1)
    {
        OS_LOG_INFO("heartbeat, uptime %lu ms, cpu %lu%%",
                    (unsigned long)os_tick_get(), (unsigned long)os_cpu_usage_get());

        (void)os_delay_ms(5000U);
    }
}
