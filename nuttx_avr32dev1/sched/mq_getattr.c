/************************************************************************
 * sched/mq_getattr.c
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
 ************************************************************************/

/************************************************************************
 * Included Files
 ************************************************************************/

#include "nuttx_config.h"

#include <stdarg.h>         /* va_list */
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <mqueue.h>
#include <sched.h>
#include <debug.h>
#include <kmalloc.h>
#include <nuttx_arch.h>
#include "os_internal.h"
#include "sig_internal.h"
#include "mq_internal.h"

/************************************************************************
 * Definitions
 ************************************************************************/

/************************************************************************
 * Private Type Declarations
 ************************************************************************/

/************************************************************************
 * Global Variables
 ************************************************************************/

/************************************************************************
 * Private Variables
 ************************************************************************/

/************************************************************************
 * Private Functions
 ************************************************************************/

/************************************************************************
 * Public Functions
 ************************************************************************/

/************************************************************************
 * Function:  mq_getattr
 *
 * Description:
 *   This functions gets status information and attributes
 *   associated with the specified message queue.
 *
 * Parameters:
 *   mqdes - Message queue descriptor
 *   mq_stat - Buffer in which to return attributes
 *
 * Return Value:
 *   0 (OK) if attributes provided, -1 (ERROR) otherwise.
 *
 * Assumptions:
 *
 ************************************************************************/

int mq_getattr(mqd_t mqdes, struct mq_attr *mq_stat)
{
  int ret = ERROR;

  if (mqdes && mq_stat)
    {
      /* Return the attributes */

      mq_stat->mq_maxmsg  = mqdes->msgq->maxmsgs;
      mq_stat->mq_msgsize = mqdes->msgq->maxmsgsize;
      mq_stat->mq_flags   = mqdes->oflags;
      mq_stat->mq_curmsgs = mqdes->msgq->nmsgs;

      ret = OK;
    }

  return ret;
}
