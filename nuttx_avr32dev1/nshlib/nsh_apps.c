/****************************************************************************
 * apps/nshlib/nsh_apps.c
 *
 *   Copyright (C) 2011 Gregory Nutt. All rights reserved.
 *   Copyright (C) 2011 Uros Platise. All rights reserved.
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

#include <nuttx_config.h>

#ifdef CONFIG_SCHED_WAITPID
#  include <sys/wait.h>
#endif

#include <stdbool.h>
#include <errno.h>

#include "apps.h"

#include "nsh.h"

#ifdef CONFIG_NSH_BUILTIN_APPS

/****************************************************************************
 * Definitions
 ****************************************************************************/

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
 * Name: nsh_execute
 ****************************************************************************/

int nsh_execapp(FAR struct nsh_vtbl_s *vtbl, FAR const char *cmd,
                FAR char *argv[])
{
   int ret = OK;
   FAR const char * name;

   /* Try to find command within pre-built application list. */

   ret = exec_nuttapp(cmd, argv);
   if (ret < 0)
     {
       int err = -errno;
       int i;

       /* On failure, list the set of available built-in commands */

       nsh_output(vtbl, "Builtin Apps: ");
       for (i = 0; (name = nuttapp_getname(i)) != NULL; i++)
         {
           nsh_output(vtbl, "%s ", name);
         }
       nsh_output(vtbl, "\nand type 'help' for more NSH commands.\n\n");
       
	   return err;
     }

#ifdef CONFIG_SCHED_WAITPID
   if (vtbl->np.np_bg == false)
     {
       waitpid(ret, NULL, 0);
     }
   else
#endif
     {
       struct sched_param param;
       sched_getparam(0, &param);
       nsh_output(vtbl, "%s [%d:%d]\n", cmd, ret, param.sched_priority);
     }

   return OK;
}

#endif /* CONFIG_NSH_BUILTIN_APPS */


