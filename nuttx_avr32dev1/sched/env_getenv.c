/****************************************************************************
 * env_getenv.c
 *
 *   Copyright (C) 2007, 2008 Gregory Nutt. All rights reserved.
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
#include <string.h>
#include <stdlib.h>
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
 * Function:  getenv
 *
 * Description:
 *   The getenv() function searches the environment list for a string that
 *   matches the string pointed to by name.
 *
 * Parameters:
 *   name - The name of the variable to find. 
 *
 * Return Value:
 *   The value of the valiable (read-only) or NULL on failure
 *
 * Assumptions:
 *   Not called from an interrupt handler
 *
 ****************************************************************************/

FAR char *getenv(const char *name)
{
  FAR _TCB      *rtcb;
  FAR environ_t *envp;
  FAR char      *pvar;
  FAR char      *pvalue = NULL;
  int ret = OK;

  /* Verify that a string was passed */

  if (!name)
    {
      ret = EINVAL;
      goto errout;
    }


  /* Get a reference to the thread-private environ in the TCB.*/

  sched_lock();
  rtcb = (FAR _TCB*)g_readytorun.head;
  envp = rtcb->envp;

  /* Check if the variable exists */

  if ( !envp || (pvar = env_findvar(envp, name)) == NULL)
    {
      ret = ENOENT;
      goto errout_with_lock;
    }

  /* It does!  Get the value sub-string from the name=value string */

  pvalue = strchr(pvar, '=');
  if (!pvalue)
    {
      /* The name=value string has no '='  This is a bug! */

      ret = EINVAL;
      goto errout_with_lock;
    }

  /* Adjust the pointer so that it points to the value right after the '=' */

  pvalue++;
  sched_unlock();
  return pvalue;

errout_with_lock:
  sched_unlock();
errout:
  *get_errno_ptr() = ret;
  return NULL;
}

#endif /* CONFIG_DISABLE_ENVIRON */



