/****************************************************************************
 * fs/fs_stat.c
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
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx_config.h>

#include <sys/stat.h>
#include <string.h>
#include <sched.h>
#include <errno.h>

#include "fs_internal.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: statpsuedo
 ****************************************************************************/

static inline int statpsuedo(FAR struct inode *inode, FAR struct stat *buf)
{
  /* Most of the stat entries just do not apply */

  memset(buf, 0, sizeof(struct stat) );
  if (inode->u.i_ops)
    {
      if (inode->u.i_ops->read)
        {
          buf->st_mode = S_IROTH|S_IRGRP|S_IRUSR;
        }

      if (inode->u.i_ops->write)
        {
          buf->st_mode |= S_IWOTH|S_IWGRP|S_IWUSR;
        }

      if (INODE_IS_MOUNTPT(inode))
        {
          buf->st_mode |= S_IFDIR;
        }
      else if (INODE_IS_BLOCK(inode))
        {
          /* What is it also has child inodes? */

          buf->st_mode |= S_IFBLK;
        }
      else
        {
          /* What is it if it also has child inodes? */

          buf->st_mode |= S_IFCHR;
        }
    }
  else
    {
      /* If it has no operations, then it must just be a intermediate
       * node in the inode tree.  It is something like a directory.
       * We'll say that all psuedo-directories are read-able but not
       * write-able.
       */

      buf->st_mode |= S_IFDIR|S_IROTH|S_IRGRP|S_IRUSR;
    }

  return OK;
}

/****************************************************************************
 * Name: statroot
 ****************************************************************************/

static inline int statroot(FAR struct stat *buf)
{
  /* There is no inode associated with the fake root directory */
  
  memset(buf, 0, sizeof(struct stat) );
  buf->st_mode = S_IFDIR|S_IROTH|S_IRGRP|S_IRUSR;
  return OK;
}

/****************************************************************************
 * Global Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stat
 *
 * Return: Zero on success; -1 on failure with errno set:
 *
 *   EACCES  Search permission is denied for one of the directories in the
 *           path prefix of path.
 *   EFAULT  Bad address.
 *   ENOENT  A component of the path path does not exist, or the path is an
 *           empty string.
 *   ENOMEM  Out of memory
 *   ENOTDIR A component of the path is not a directory.
 *
 ****************************************************************************/

int stat(const char *path, FAR struct stat *buf)
{
  FAR struct inode *inode;
  const char       *relpath = NULL;
  int               ret     = OK;

  /* Sanity checks */

  if (!path || !buf)
    {
      ret = EFAULT;
      goto errout;
    }

  if (!path[0])
    {
      ret = ENOENT;
      goto errout;
    }

  /* Check for the fake root directory (which has no inode) */

  if (strcmp(path, "/") == 0)
    {
      return statroot(buf);
    }

  /* Get an inode for this file */

  inode = inode_find(path, &relpath);
  if (!inode)
    {
      /* This name does not refer to a psudeo-inode and there is no
       * mountpoint that includes in this path.
       */

      ret = ENOENT;
      goto errout;
    }

  /* The way we handle the stat depends on the type of inode that we
   * are dealing with.
   */

#ifndef CONFIG_DISABLE_MOUNTPOINT
  if (INODE_IS_MOUNTPT(inode))
    {
      /* The node is a file system mointpoint. Verify that the mountpoint
       * supports the stat() method
       */

      if (inode->u.i_mops && inode->u.i_mops->stat)
        {
          /* Perform the rewinddir() operation */

          ret = inode->u.i_mops->stat(inode, relpath, buf);
        }
    }
  else
#endif
    {
      /* The node is part of the root psuedo file system */

      ret = statpsuedo(inode, buf);
    }

  /* Check if the stat operation was successful */

  if (ret < 0)
    {
      ret = -ret;
      goto errout_with_inode;
    }

  /* Successfully stat'ed the file */

  inode_release(inode);
  return OK;

/* Failure conditions always set the errno appropriately */

 errout_with_inode:
  inode_release(inode);
 errout:
  *get_errno_ptr() = ret;
  return ERROR;
}

