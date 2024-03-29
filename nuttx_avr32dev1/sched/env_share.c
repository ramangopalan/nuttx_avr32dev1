/****************************************************************************
 * sched/env_share.c
 *
 *   Copyright (C) 2007, 2009 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
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

#ifndef CONFIG_DISABLE_ENVIRON

#include <sched.h>
#include <errno.h>
#include "os_internal.h"
#include "env_internal.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function:  env_share
 *
 * Description:
 *   Increment the reference count on the internal environment structure of
 *   a task.  This is the action that is performed when a new pthread is
 *   created: The new pthread shares the environment with its parent.
 *
 * Parameters:
 *   ptcb The new TCB to receive the shared environment.
 *
 * Return Value:
 *   A pointer to a specified TCB's environment structure with an incremented
 *   reference count.
 *
 * Assumptions:
 *   Not called from an interrupt handler.
 *
 ****************************************************************************/

int env_share(FAR _TCB *ptcb)
{
  int ret = OK;
  if (!ptcb)
    {
      ret = -EINVAL;
    }
  else
    {
      FAR _TCB *parent = (FAR _TCB*)g_readytorun.head;
      environ_t *envp = parent->envp;

      /* Pre-emption must be disabled throughout the following because the
       * environment is shared.
       */

      sched_lock();

      /* Does the parent task have an environment? */

      if (envp)
        {
          /* Yes.. increment the reference count on the environment */

          envp->ev_crefs++;
        }

      /* Then share the environment */

      ptcb->envp = envp;
      sched_unlock();
    }
  return ret;
}

#endif /* CONFIG_DISABLE_ENVIRON */



