/**
 * @file os_main_task.c
 * @brief Ahura kernel example: task lifecycle (create / start / pause / resume / delete).
 *
 * Creates one worker task and walks its handle through every state
 * os_task_state_get() can report: SUSPENDED right after os_task_create(),
 * READY/RUNNING after os_task_start(), back to SUSPENDED after os_task_pause(),
 * and finally INACTIVE after os_task_delete(). Copy this file into the
 * application source tree as os_main.c to run it - no extra os_config.h
 * switch needed, tasks are always available.
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

/*
 * ***********************************************************************************************************
 * Private objects
 * ***********************************************************************************************************
*/

OS_TASK_DEFINE(worker, 512U);

static volatile uint32_t g_worker_iterations = 0U;

/*
 * ***********************************************************************************************************
 * Private function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
static const char* task_state_name(os_task_state_t state)
{
    switch (state)
    {
        case OS_TASK_STATE_INACTIVE:  return "INACTIVE";
        case OS_TASK_STATE_READY:     return "READY";
        case OS_TASK_STATE_RUNNING:   return "RUNNING";
        case OS_TASK_STATE_BLOCKED:   return "BLOCKED";
        case OS_TASK_STATE_SUSPENDED: return "SUSPENDED";
        default:                      return "?";
    }
}

/******************************************************************************************************/
/**
 * @brief Worker entry: counts its own iterations and yields, so os_main can watch it progress.
 */
static void worker_entry(void *context)
{
    (void)context;

    while (1)
    {
        g_worker_iterations++;
        os_task_yield();
    }
}

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Default application task body: drives the worker task through its full lifecycle.
 *
 * @return None.
 */
void os_main(void)
{
    (void)os_task_create(&worker, OS_TASK_CONFIG(worker, worker_entry, NULL, OS_TASK_PRIO_1));
    printf("[task] created: state=%s\r\n", task_state_name(os_task_state_get(&worker)));

    (void)os_task_start(&worker);
    (void)os_delay_ms(20U);
    printf("[task] started: state=%s, iterations=%lu\r\n", task_state_name(os_task_state_get(&worker)),
           (unsigned long)g_worker_iterations);

    (void)os_task_pause(&worker);
    printf("[task] paused:  state=%s\r\n", task_state_name(os_task_state_get(&worker)));
    (void)os_delay_ms(20U);
    printf("[task] iterations frozen while paused: %lu\r\n", (unsigned long)g_worker_iterations);

    (void)os_task_start(&worker); /* os_task_start() also resumes a paused task */
    (void)os_delay_ms(20U);
    printf("[task] resumed: state=%s, iterations=%lu\r\n", task_state_name(os_task_state_get(&worker)),
           (unsigned long)g_worker_iterations);

    (void)os_task_delete(&worker);
    printf("[task] deleted: state=%s\r\n", task_state_name(os_task_state_get(&worker)));

    while (1)
    {
        (void)os_delay_ms(1000U);
    }
}
