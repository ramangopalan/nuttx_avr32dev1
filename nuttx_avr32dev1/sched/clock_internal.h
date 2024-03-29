/********************************************************************************
 * clock_internal.h
 *
 *   Copyright (C) 2007-2009 Gregory Nutt. All rights reserved.
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
 ********************************************************************************/

#ifndef __CLOCK_INTERNAL_H
#define __CLOCK_INTERNAL_H

/********************************************************************************
 * Included Files
 ********************************************************************************/

#include "nuttx_config.h"

#include <stdint.h>
#include "clock.h"
#include "compiler.h"

/********************************************************************************
 * Definitions
 ********************************************************************************/

/********************************************************************************
 * Public Type Definitions
 ********************************************************************************/

/********************************************************************************
 * Global Variables
 ********************************************************************************/

extern struct timespec g_basetime;
extern uint32_t        g_tickbias;

/********************************************************************************
 * Public Function Prototypes
 ********************************************************************************/

extern void weak_function clock_initialize(void);
extern void weak_function clock_timer(void);

extern int    clock_abstime2ticks(/* clockid_t */ uint8_t clockid,
                                  FAR const struct timespec *abstime,
                                  FAR int *ticks);
extern int    clock_time2ticks(FAR const struct timespec *reltime, FAR int *ticks);
extern int    clock_ticks2time(int ticks, FAR struct timespec *reltime);

#endif /* __CLOCK_INTERNAL_H */
