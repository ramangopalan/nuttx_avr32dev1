/****************************************************************************
 * sched/os_bringup.c
 *
 *   Copyright (C) 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
 *
 * With extensions by:
 *
 *   Author: Uros Platise <uros.platise@isotel.eu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "nuttx_config.h"

#include <sched.h>
#include <debug.h>

#include <nuttx/init.h>

#ifdef CONFIG_PAGING
# include "pg_internal.h"
#endif
#ifdef CONFIG_SCHED_WORKQUEUE
# include "work_internal.h"
#endif
#ifdef CONFIG_BUILTIN_APP_START
# include "apps/apps.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_CUSTOM_STACK
#  define START_TASK(n,p,s,e,a) task_create(n,p,s,e,a)
#else
#  define START_TASK(n,p,s,e,a) task_create(n,p,e,a)
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: os_bringup
 *
 * Description:
 *   Start all initial system tasks.  This does the "system bring-up" after
 *   the conclusion of basic OS initialization.  These initial system tasks
 *   may include:
 *
 *   - pg_worker:   The page-fault worker thread (only if CONFIG_PAGING is
 *                  defined.
 *   - work_thread: The work thread.  This general thread can be used to
 *                  perform most any kind of queued work.  Its primary
 *                  function is to serve as the "bottom half" of device
 *                  drivers.
 *
 *   And the main application entry point.  This may be one of two different
 *   symbols:
 *
 *   - user_start:  This is the default entry point used for all of the
 *                  examples.
 *   - CONFIG_BUILTIN_APP_START: The system can also be configured to start
 *                  custom applications at however CONFIG_BUILTIN_APP_START
 *                  is defined in the NuttX start-up file.
 *                 
 ****************************************************************************/

int os_bringup(void)
{
#ifdef CONFIG_BUILTIN_APP_START 
  static const char *argv[3] = {NULL, "init", NULL};
#endif
  int init_taskid;

  /* Start the page fill worker thread that will resolve page faults.
   * This should always be the first thread started because it may
   * have to resolve page faults in other threads
   */

#ifdef CONFIG_PAGING
  svdbg("Starting paging thread\n");

  g_pgworker = START_TASK("pgfill", CONFIG_PAGING_DEFPRIO,
                          CONFIG_PAGING_STACKSIZE,
                          (main_t)pg_worker, (const char **)NULL);
  ASSERT(g_pgworker != ERROR);
#endif

  /* Start the worker thread that will perform misc garbage clean-up */

#ifdef CONFIG_SCHED_WORKQUEUE
  svdbg("Starting worker thread\n");

  g_worker = START_TASK("work", CONFIG_SCHED_WORKPRIORITY,
                        CONFIG_SCHED_WORKSTACKSIZE,
                        (main_t)work_thread, (const char **)NULL);
  ASSERT(g_worker != ERROR);
#endif

  /* Once the operating system has been initialized, the system must be
   * started by spawning the user init thread of execution.
   */

  svdbg("Starting init thread\n");

#ifdef CONFIG_BUILTIN_APP_START
  /* Start the built-in application, passing an "init" argument, so that
   * application can distinguish different run-levels 
   */
  
  init_taskid = exec_nuttapp(CONFIG_BUILTIN_APP_START, argv);
#else
  /* Start the default application at user_start() */

  init_taskid = START_TASK("init", SCHED_PRIORITY_DEFAULT,
                           CONFIG_USERMAIN_STACKSIZE,
                           (main_t)user_start, (const char **)NULL);
#endif
  ASSERT(init_taskid != ERROR);
  return OK;
}
