/****************************************************************************
 * include/apps/apps.h
 *
 *   Copyright(C) 2011 Uros Platise. All rights reserved.
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
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_APPS_APPS_H
#define __INCLUDE_APPS_APPS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx_config.h>
#include <sys/types.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct nuttapp_s
{
  const char *name;			/* Invocation name and as seen under /sbin/ */
  int         priority;		/* Use: SCHED_PRIORITY_DEFAULT */
  int         stacksize;	/* Desired stack size */
  main_t      main;			/* Entry point: main(int argc, char *argv[]) */
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: check for availability of builtin NuttX application
 *
 * Description:
 *   Checks for availabiliy of application registerred during compile time.
 *
 * Input Parameter:
 *   filename - Name of the linked-in binary to be started.
 *
 * Returned Value:
 *   This is an end-user function, so it follows the normal convention:
 *   Returns index of builtin application. If it is not found then it
 *   returns -1 (ERROR) and sets errno appropriately.
 *
 ****************************************************************************/

EXTERN int nuttapp_isavail(FAR const char *appname);

/****************************************************************************
 * Name: get name of built-in application
 *
 * Description:
 *   Returns pointer to a name of built-in application pointed by the
 *   index.
 *
 * Input Parameter:
 *   index, from 0 and on ...
 *
 * Returned Value:
 *   Returns valid pointer pointing to the app name if index is valid.
 *   Otherwise NULL is returned.
 *
 ****************************************************************************/

EXTERN const char * nuttapp_getname(int index);

/****************************************************************************
 * Name: execute builtin NuttX application
 *
 * Description:
 *   Executes builtin application registerred during compile time.
 *   New application is run in a separate task context (and thread).
 *
 * Input Parameter:
 *   filename - Name of the linked-in binary to be started.
 *   argv     - Argument list
 *
 * Returned Value:
 *   This is an end-user function, so it follows the normal convention:
 *   Returns the PID of the exec'ed module.  On failure, it.returns
 *   -1 (ERROR) and sets errno appropriately.
 *
 ****************************************************************************/

EXTERN int exec_nuttapp(FAR const char *appname, FAR const char *argv[]);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __INCLUDE_APPS_APPS_H */
