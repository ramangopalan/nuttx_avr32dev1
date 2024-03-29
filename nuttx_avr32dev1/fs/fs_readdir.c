/****************************************************************************
 * fs/fs_readdir.c
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

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <nuttx/fs.h>

#include "fs_internal.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: readpsuedodir
 ****************************************************************************/

static inline int readpsuedodir(struct internal_dir_s *idir)
{
  FAR struct inode *prev;

  /* Check if we are at the end of the list */

  if (!idir->u.psuedo.fd_next)
    {
      /* End of file and error conditions are not distinguishable
       * with readdir.  Here we return -ENOENT to signal the end
       * of the directory.
       */

      return -ENOENT;
    }

  /* Copy the inode name into the dirent structure */

  strncpy(idir->fd_dir.d_name, idir->u.psuedo.fd_next->i_name, NAME_MAX+1);

  /* If the node has file operations, we will say that it is
   * a file.
   */

  idir->fd_dir.d_type = 0;
  if (idir->u.psuedo.fd_next->u.i_ops)
    {
#ifndef CONFIG_DISABLE_MOUNTPOINT
      if (INODE_IS_BLOCK(idir->u.psuedo.fd_next)) 
        {
           idir->fd_dir.d_type |= DTYPE_BLK;
        }
      if (INODE_IS_MOUNTPT(idir->u.psuedo.fd_next)) 
        {
           idir->fd_dir.d_type |= DTYPE_DIRECTORY;
        }
      else
#endif
        {
           idir->fd_dir.d_type |= DTYPE_CHR;
        }
    }

  /* If the node has child node(s), then we will say that it
   * is a directory.  NOTE: that the node can be both!
   */

  if (idir->u.psuedo.fd_next->i_child || !idir->u.psuedo.fd_next->u.i_ops)
    {
      idir->fd_dir.d_type |= DTYPE_DIRECTORY;
    }

  /* Now get the inode to vist next time that readdir() is called */

  inode_semtake();

  prev                   = idir->u.psuedo.fd_next;
  idir->u.psuedo.fd_next = prev->i_peer; /* The next node to visit */

  if (idir->u.psuedo.fd_next)
    {
      /* Increment the reference count on this next node */

      idir->u.psuedo.fd_next->i_crefs++;
    }

  inode_semgive();

  if (prev)
    {
      inode_release(prev);
    }

  return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: readdir
 *
 * Description:
 *   The readdir() function returns a pointer to a dirent
 *   structure representing the next directory entry in the
 *   directory stream pointed to by dir.  It returns NULL on
 *   reaching the end-of-file or if an error occurred.
 *
 * Inputs:
 *   dirp -- An instance of type DIR created by a previous
 *     call to opendir();
 *
 * Return:
 *   The readdir() function returns a pointer to a dirent
 *   structure, or NULL if an error occurs or end-of-file
 *   is reached.  On error, errno is set appropriately.
 *
 *   EBADF   - Invalid directory stream descriptor dir
 *
 ****************************************************************************/

FAR struct dirent *readdir(DIR *dirp)
{
  FAR struct internal_dir_s *idir = (struct internal_dir_s *)dirp;
#ifndef CONFIG_DISABLE_MOUNTPOINT
  struct inode *inode;
#endif
  int ret;

  /* Sanity checks */

  if (!idir || !idir->fd_root)
    {
      ret = EBADF;
      goto errout;
    }

  /* The way we handle the readdir depends on the type of inode
   * that we are dealing with.
   */

#ifndef CONFIG_DISABLE_MOUNTPOINT
  inode = idir->fd_root;
  if (INODE_IS_MOUNTPT(inode) && !DIRENT_ISPSUEDONODE(idir->fd_flags))
    {
      /* The node is a file system mointpoint. Verify that the mountpoint
       * supports the readdir() method
       */

      if (!inode->u.i_mops || !inode->u.i_mops->readdir)
         {
           ret = EACCES;
            goto errout;
         }

      /* Perform the readdir() operation */

      ret = inode->u.i_mops->readdir(inode, idir);
    }
  else
#endif
    {
      /* The node is part of the root psuedo file system */

      ret = readpsuedodir(idir);
    }

  /* ret < 0 is an error.  Special case: ret = -ENOENT is end of file */

  if ( ret < 0)
    {
      if (ret == -ENOENT)
        {
          ret = OK;
        }
      else
        {
          ret = -ret;
        }
      goto errout;
    }

  /* Success */

  idir->fd_position++;
  return &idir->fd_dir;

errout:
  *get_errno_ptr() = ret;
  return NULL;
}

