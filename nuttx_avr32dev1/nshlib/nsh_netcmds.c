/****************************************************************************
 * apps/nshlib/nsh_netcmds.c
 *
 *   Copyright (C) 2007-2011 Gregory Nutt. All rights reserved.
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

#include <nuttx_config.h>
#ifdef CONFIG_NET

#include <sys/stat.h>    /* Needed for open */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>       /* Needed for open */
#include <libgen.h>      /* Needed for basename */
#include <errno.h>

#include <nuttx/net.h>
#include <nuttx/clock.h>
#include <net/ethernet.h>
#include <net/uip/uip.h>
#include <net/uip/uip-arch.h>
#include <netinet/ether.h>

#ifdef CONFIG_NET_STATISTICS
#  include <net/uip/uip.h>
#endif

#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING) && \
   !defined(CONFIG_DISABLE_CLOCK) && !defined(CONFIG_DISABLE_SIGNALS)
#  include <apps/netutils/uiplib.h>
#endif

#if defined(CONFIG_NET_UDP) && CONFIG_NFILE_DESCRIPTORS > 0
#  include <apps/netutils/uiplib.h>
#  include <apps/netutils/tftp.h>
#endif

#if defined(CONFIG_NET_TCP) && CONFIG_NFILE_DESCRIPTORS > 0
#  ifndef CONFIG_NSH_DISABLE_WGET
#    include <apps/netutils/uiplib.h>
#    include <apps/netutils/webclient.h>
#  endif
#endif

#include "nsh.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

#define DEFAULT_PING_DATALEN 56

/****************************************************************************
 * Private Types
 ****************************************************************************/

#if defined(CONFIG_NET_UDP) && CONFIG_NFILE_DESCRIPTORS > 0
struct tftpc_args_s
{
  bool        binary;    /* true:binary ("octect") false:text ("netascii") */
  bool        allocated; /* true: destpath is allocated */
  char       *destpath;  /* Path at destination */
  const char *srcpath;   /* Path at src */
  in_addr_t   ipaddr;    /* Host IP address */
};
#endif

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING) && \
   !defined(CONFIG_DISABLE_CLOCK) && !defined(CONFIG_DISABLE_SIGNALS)
static uint16_t g_pingid = 0;
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ping_newid
 ****************************************************************************/

#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING) && \
   !defined(CONFIG_DISABLE_CLOCK) && !defined(CONFIG_DISABLE_SIGNALS)
static inline uint16_t ping_newid(void)
{
  irqstate_t save = irqsave();
  uint16_t ret = ++g_pingid;
  irqrestore(save);
  return ret;
}
#endif

/****************************************************************************
 * Name: uip_statistics
 ****************************************************************************/

#if defined(CONFIG_NET_STATISTICS) && !defined(CONFIG_NSH_DISABLE_IFCONFIG)
static inline void uip_statistics(FAR struct nsh_vtbl_s *vtbl)
{
  nsh_output(vtbl, "uIP         IP ");
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, "  TCP");
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, "  UDP");
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, "  ICMP");
#endif
  nsh_output(vtbl, "\n");

  /* Received packets */

  nsh_output(vtbl, "Received    %04x",uip_stat.ip.recv);
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, " %04x",uip_stat.tcp.recv);
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " %04x",uip_stat.udp.recv);
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, " %04x",uip_stat.icmp.recv);
#endif
  nsh_output(vtbl, "\n");

  /* Dropped packets */

  nsh_output(vtbl, "Dropped     %04x",uip_stat.ip.drop);
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, " %04x",uip_stat.tcp.drop);
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " %04x",uip_stat.udp.drop);
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, " %04x",uip_stat.icmp.drop);
#endif
  nsh_output(vtbl, "\n");

  nsh_output(vtbl, "  IP        VHL: %04x HBL: %04x\n",
             uip_stat.ip.vhlerr, uip_stat.ip.hblenerr);
  nsh_output(vtbl, "            LBL: %04x Frg: %04x\n",
             uip_stat.ip.lblenerr, uip_stat.ip.fragerr);

  nsh_output(vtbl, "  Checksum  %04x",uip_stat.ip.chkerr);
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, " %04x",uip_stat.tcp.chkerr);
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " %04x",uip_stat.udp.chkerr);
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, " ----");
#endif
  nsh_output(vtbl, "\n");

#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, "  TCP       ACK: %04x SYN: %04x\n", 
            uip_stat.tcp.ackerr, uip_stat.tcp.syndrop);
  nsh_output(vtbl, "            RST: %04x %04x\n", 
            uip_stat.tcp.rst, uip_stat.tcp.synrst);
#endif

  nsh_output(vtbl, "  Type      %04x",uip_stat.ip.protoerr);
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, " ----");
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " ----");
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, " %04x",uip_stat.icmp.typeerr);
#endif
  nsh_output(vtbl, "\n");

  /* Sent packets */

  nsh_output(vtbl, "Sent        ----",uip_stat.ip.sent);
#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, " %04x",uip_stat.tcp.sent);
#endif
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " %04x",uip_stat.udp.sent);
#endif
#ifdef CONFIG_NET_ICMP
  nsh_output(vtbl, " %04x",uip_stat.icmp.sent);
#endif
  nsh_output(vtbl, "\n");

#ifdef CONFIG_NET_TCP
  nsh_output(vtbl, "  Rexmit    ---- %04x",uip_stat.tcp.rexmit);
#ifdef CONFIG_NET_UDP
  nsh_output(vtbl, " ----");
#endif
#ifdef CONFIG_NET_ICMP
   nsh_output(vtbl, " ----");
#endif
  nsh_output(vtbl, "\n");
#endif
  nsh_output(vtbl, "\n");
}
#else
# define uip_statistics(vtbl)
#endif


/****************************************************************************
 * Name: ifconfig_callback
 ****************************************************************************/

int ifconfig_callback(FAR struct uip_driver_s *dev, void *arg)
{
  struct nsh_vtbl_s *vtbl = (struct nsh_vtbl_s*)arg;
  struct in_addr addr;

  nsh_output(vtbl, "%s\tHWaddr %s\n", dev->d_ifname, ether_ntoa(&dev->d_mac));
  addr.s_addr = dev->d_ipaddr;
  nsh_output(vtbl, "\tIPaddr:%s ", inet_ntoa(addr));
  addr.s_addr = dev->d_draddr;
  nsh_output(vtbl, "DRaddr:%s ", inet_ntoa(addr));
  addr.s_addr = dev->d_netmask;
  nsh_output(vtbl, "Mask:%s\n\n", inet_ntoa(addr));
  return OK;
}

/****************************************************************************
 * Name: tftpc_parseargs
 ****************************************************************************/

#if defined(CONFIG_NET_UDP) && CONFIG_NFILE_DESCRIPTORS > 0
int tftpc_parseargs(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv,
                    struct tftpc_args_s *args)
{
  FAR const char *fmt = g_fmtarginvalid;
  bool badarg = false;
  int option;

  /* Get the ping options */

  memset(args, 0, sizeof(struct tftpc_args_s));
  while ((option = getopt(argc, argv, ":bnf:h:")) != ERROR)
    {
      switch (option)
        {
          case 'b':
            args->binary = true;
            break;

          case 'n':
            args->binary = false;
            break;

          case 'f':
            args->destpath = optarg;
            break;

          case 'h':
            if (!uiplib_ipaddrconv(optarg, (FAR unsigned char*)&args->ipaddr))
              {
                nsh_output(vtbl, g_fmtarginvalid, argv[0]);
                badarg = true;
              }
            break;

          case ':':
            nsh_output(vtbl, g_fmtargrequired, argv[0]);
            badarg = true;
            break;

          case '?':
          default:
            nsh_output(vtbl, g_fmtarginvalid, argv[0]);
            badarg = true;
            break;
        }
    }

  /* If a bad argument was encountered, then return without processing the command */

  if (badarg)
    {
      return ERROR;
    }

  /* There should be exactly on parameter left on the command-line */

  if (optind == argc-1)
    {
      args->srcpath = argv[optind];
    }
  else if (optind >= argc)
    {
      fmt = g_fmttoomanyargs;
      goto errout;
    }
  else
    {
      fmt = g_fmtargrequired;
      goto errout;
    }

  /* The HOST IP address is also required */

  if (!args->ipaddr)
    {
      fmt = g_fmtargrequired;
      goto errout;
    }

  /* If the destpath was not provided, then we have do a little work. */

  if (!args->destpath)
    {
      char *tmp1;
      char *tmp2;

      /* Copy the srcpath... baseanme might modify it */

      fmt = g_fmtcmdoutofmemory;
      tmp1 = strdup(args->srcpath);
      if (!tmp1)
        {
          goto errout;
        }

      /* Get the basename of the srcpath */

      tmp2 = basename(tmp1);
      if (!tmp2)
        {
          free(tmp1);
          goto errout;
        }

      /* Use that basename as the destpath */

      args->destpath  = strdup(tmp2);
      free(tmp1);
      if (!args->destpath)
        {
          goto errout;
        }
      args->allocated = true;
    }

  return OK;

errout:
  nsh_output(vtbl, fmt, argv[0]);
  return ERROR;
}
#endif

/****************************************************************************
 * Name: wget_callback
 ****************************************************************************/

#if defined(CONFIG_NET_TCP) && CONFIG_NFILE_DESCRIPTORS > 0
#ifndef CONFIG_NSH_DISABLE_WGET
static void wget_callback(FAR char **buffer, int offset, int datend,
                          FAR int *buflen, FAR void *arg)
{
  (void)write((int)arg, &((*buffer)[offset]), datend - offset);
}
#endif
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cmd_get
 ****************************************************************************/

#if defined(CONFIG_NET_UDP) && CONFIG_NFILE_DESCRIPTORS > 0
#ifndef CONFIG_NSH_DISABLE_GET
int cmd_get(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
  struct tftpc_args_s args;
  char *fullpath;

  /* Parse the input parameter list */

  if (tftpc_parseargs(vtbl, argc, argv, &args) != OK)
    {
      return ERROR;
    }

  /* Get the full path to the local file */

  fullpath = nsh_getfullpath(vtbl, args.srcpath);

  /* Then perform the TFTP get operation */

  if (tftpget(args.srcpath, fullpath, args.ipaddr, args.binary) != OK)
    {
      nsh_output(vtbl, g_fmtcmdfailed, argv[0], "tftpget", NSH_ERRNO);
    }

  /* Release any allocated memory */

  if (args.allocated)
    {
      free(args.destpath);
    }
  free(fullpath);
  return OK;
}
#endif
#endif

/****************************************************************************
 * Name: cmd_ifconfig
 ****************************************************************************/

#ifndef CONFIG_NSH_DISABLE_IFCONFIG
int cmd_ifconfig(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
  netdev_foreach(ifconfig_callback, vtbl);
  uip_statistics(vtbl);
  return OK;
}
#endif

/****************************************************************************
 * Name: cmd_ping
 ****************************************************************************/

#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING) && \
   !defined(CONFIG_DISABLE_CLOCK) && !defined(CONFIG_DISABLE_SIGNALS)
#ifndef CONFIG_NSH_DISABLE_PING
int cmd_ping(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
  FAR const char *fmt = g_fmtarginvalid;
  const char *staddr;
  uip_ipaddr_t ipaddr;
  uint32_t start;
  uint32_t next;
  uint32_t dsec = 10;
  uint16_t id;
  bool badarg = false;
  int count = 10;
  int option;
  int seqno;
  int replies = 0;
  int elapsed;
  int tmp;
  int i;

  /* Get the ping options */

  while ((option = getopt(argc, argv, ":c:i:")) != ERROR)
    {
      switch (option)
        {
          case 'c':
            count = atoi(optarg);
            if (count < 1 || count > 10000)
              {
                nsh_output(vtbl, g_fmtargrange, argv[0]);
                badarg = true;
              }
            break;

          case 'i':
            tmp = atoi(optarg);
            if (tmp < 1 || tmp >= 4294)
              {
                nsh_output(vtbl, g_fmtargrange, argv[0]);
                badarg = true;
              }
            else
              {
                dsec = 10 * tmp;
              }
            break;

          case ':':
            nsh_output(vtbl, g_fmtargrequired, argv[0]);
            badarg = true;
            break;

          case '?':
          default:
            nsh_output(vtbl, g_fmtarginvalid, argv[0]);
            badarg = true;
            break;
        }
    }

  /* If a bad argument was encountered, then return without processing the command */

  if (badarg)
    {
      return ERROR;
    }

  /* There should be exactly on parameter left on the command-line */

  if (optind == argc-1)
    {
      staddr = argv[optind];
      if (!uiplib_ipaddrconv(staddr, (FAR unsigned char*)&ipaddr))
        {
          goto errout;
        }
    }
  else if (optind >= argc)
    {
      fmt = g_fmttoomanyargs;
      goto errout;
    }
  else
    {
      fmt = g_fmtargrequired;
      goto errout;
    }

  /* Get the ID to use */

  id = ping_newid();

  /* Loop for the specified count */

  nsh_output(vtbl, "PING %s %d bytes of data\n", staddr, DEFAULT_PING_DATALEN);
  start = g_system_timer;
  for (i = 1; i <= count; i++)
    {
      /* Send the ECHO request and wait for the response */

      next  = g_system_timer;
      seqno = uip_ping(ipaddr, id, i, DEFAULT_PING_DATALEN, dsec);

      /* Was any response returned? We can tell if a non-negative sequence
       * number was returned.
       */

      if (seqno >= 0 && seqno <= i)
        {
          /* Get the elpased time from the time that the request was
           * sent until the response was received.  If we got a response
           * to an earlier request, then fudge the elpased time.
           */

          elapsed = TICK2MSEC(g_system_timer - next);
          if (seqno < i)
            {
              elapsed += 100*dsec*(i - seqno);
            }

          /* Report the receipt of the reply */

          nsh_output(vtbl, "%d bytes from %s: icmp_seq=%d time=%d ms\n",
                     DEFAULT_PING_DATALEN, staddr, seqno, elapsed);
          replies++;
        }

      /* Wait for the remainder of the interval.  If the last seqno<i,
       * then this is a bad idea... we will probably lose the response
       * to the current request!
       */

      elapsed = TICK2DSEC(g_system_timer - next);
      if (elapsed < dsec)
        {
          usleep(100000*dsec);
        }
    }

  /* Get the total elapsed time */

  elapsed = TICK2MSEC(g_system_timer - start);

  /* Calculate the percentage of lost packets */

  tmp = (100*(count - replies) + (count >> 1)) / count;

  nsh_output(vtbl, "%d packets transmitted, %d received, %d%% packet loss, time %d ms\n",
             count, replies, tmp, elapsed);
  return OK;

errout:
  nsh_output(vtbl, fmt, argv[0]);
  return ERROR;
}
#endif
#endif /* CONFIG_NET_ICMP && CONFIG_NET_ICMP_PING */

/****************************************************************************
 * Name: cmd_put
 ****************************************************************************/

#if defined(CONFIG_NET_UDP) && CONFIG_NFILE_DESCRIPTORS > 0
#ifndef CONFIG_NSH_DISABLE_PUT
int cmd_put(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
  struct tftpc_args_s args;
  char *fullpath;

  /* Parse the input parameter list */

  if (tftpc_parseargs(vtbl, argc, argv, &args) != OK)
    {
      return ERROR;
    }

  /* Get the full path to the local file */

  fullpath = nsh_getfullpath(vtbl, args.srcpath);

  /* Then perform the TFTP put operation */

  if (tftpput(fullpath, args.destpath, args.ipaddr, args.binary) != OK)
    {
      nsh_output(vtbl, g_fmtcmdfailed, argv[0], "tftpput", NSH_ERRNO);
    }

  /* Release any allocated memory */

  if (args.allocated)
    {
      free(args.destpath);
    }
  free(fullpath);
  return OK;
}
#endif
#endif

/****************************************************************************
 * Name: cmd_wget
 ****************************************************************************/

#if defined(CONFIG_NET_TCP) && CONFIG_NFILE_DESCRIPTORS > 0
#ifndef CONFIG_NSH_DISABLE_WGET
int cmd_wget(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
  char *localfile = NULL;
  char *allocfile = NULL;
  char *buffer    = NULL;
  char *fullpath  = NULL;
  char *url;
  const char *fmt;
  bool badarg = false;
  int option;
  int fd = -1;
  int ret;

  /* Get the wget options */

  while ((option = getopt(argc, argv, ":o:")) != ERROR)
    {
      switch (option)
        {
          case 'o':
            localfile = optarg;
            break;

          case ':':
            nsh_output(vtbl, g_fmtargrequired, argv[0]);
            badarg = true;
            break;

          case '?':
          default:
            nsh_output(vtbl, g_fmtarginvalid, argv[0]);
            badarg = true;
            break;
        }
    }

  /* If a bad argument was encountered, then return without processing the command */

  if (badarg)
    {
      return ERROR;
    }

  /* There should be exactly on parameter left on the command-line */

  if (optind == argc-1)
    {
      url = argv[optind];
    }
  else if (optind >= argc)
    {
      fmt = g_fmttoomanyargs;
      goto errout;
    }
  else
    {
      fmt = g_fmtargrequired;
      goto errout;
    }

  /* Get the local file name */

  if (!localfile)
    {
      allocfile = strdup(url);
      localfile = basename(allocfile);
    }

  /* Get the full path to the local file */

  fullpath = nsh_getfullpath(vtbl, localfile);

  /* Open the local file for writing */

  fd = open(fullpath, O_WRONLY|O_CREAT|O_TRUNC, 0644);
  if (fd < 0)
    {
      nsh_output(vtbl, g_fmtcmdfailed, argv[0], "open", NSH_ERRNO);
      ret = ERROR;
      goto exit;
    }

  /* Allocate an I/O buffer */

  buffer = malloc(512);
  if (!buffer)
    {
      fmt = g_fmtcmdoutofmemory;
      goto errout;
    }

  /* And perform the wget */

  ret = wget(url, buffer, 512, wget_callback, (FAR void *)fd);
  if (ret < 0)
    {
      nsh_output(vtbl, g_fmtcmdfailed, argv[0], "wget", NSH_ERRNO);
      goto exit;
     }

  /* Free allocated resources */

exit:
  if (fd >= 0)
    {
      close(fd);
    }
  if (allocfile)
    {
      free(allocfile);
    }
  if (fullpath)
    {
      free(fullpath);
    }
  if (buffer)
    {
      free(buffer);
    }
  return ret;

errout:
  nsh_output(vtbl, fmt, argv[0]);
  ret = ERROR;
  goto exit;
}
#endif
#endif

#endif /* CONFIG_NET */
