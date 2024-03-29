/****************************************************************************
 * include/nuttx/nxfonts.h
 *
 *   Copyright (C) 2008, 2009 Gregory Nutt. All rights reserved.
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

#ifndef __INCLUDE_NUTTX_NXFONTS_H
#define __INCLUDE_NUTTX_NXFONTS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <nuttx/nxglib.h>

/****************************************************************************
 * Pre-processor definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This structures provides the metrics for one glyph */

struct nx_fontmetic_s
{
  uint32_t stride   : 2;    /* Width of one font row in bytes */
  uint32_t width    : 6;    /* Width of the font in bits */
  uint32_t height   : 6;    /* Height of the font in rows */
  uint32_t xoffset  : 6;    /* Top, left-hand corner X-offset in pixels */
  uint32_t yoffset  : 6;    /* Top, left-hand corner y-offset in pixels */
  uint32_t unused   : 6;
};

/* This structure binds the glyph metrics to the glyph bitmap */

struct nx_fontbitmap_s
{
  struct nx_fontmetic_s metric; /* Character metrics */
  FAR const uint8_t *bitmap;      /* Pointer to the character bitmap */
};

/* This structure describes one contiguous grouping of glyphs that
 * can be described by an array starting with encoding 'first' and
 * extending through (first + nchars - 1).
 */

struct nx_fontset_s
{
  uint8_t  first;           /* First bitmap character code */
  uint8_t  nchars;          /* Number of bitmap character codes */
  FAR const struct nx_fontbitmap_s *bitmap;
};

/* This structure describes the overall fontset */

struct nx_font_s
{
  uint8_t  mxheight;        /* Max height of one glyph in rows */
  uint8_t  mxwidth;         /* Max width of any glyph in pixels */
  uint8_t  mxbits;          /* Max number of bits per character code */
  uint8_t  spwidth;         /* The width of a space in pixels */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
# define EXTERN extern "C"
extern "C" {
#else
# define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: nxf_getfontset
 *
 * Description:
 *   Return information about the current font set
 *
 * Input Parameters:
 *   An instance of struct nx_font_s describing the font set.
 *
 ****************************************************************************/

EXTERN FAR const struct nx_font_s *nxf_getfontset(void);

/****************************************************************************
 * Name: nxf_getbitmap
 *
 * Description:
 *   Return font bitmap information for the selected character encoding.
 *
 * Input Parameters:
 *   ch - character code
 *
 * Returned Value:
 *   An instance of struct nx_fontbitmap_s describing the glyph.
 *
 ****************************************************************************/

EXTERN FAR const struct nx_fontbitmap_s *nxf_getbitmap(uint16_t ch);

/****************************************************************************
 * Name: nxf_convert_*bpp
 *
 * Description:
 *   Convert the 1BPP font to a new pixel depth
 *
 * Input Parameters:
 *   dest   - The destination buffer provided by the caller.
 *   height - The max height of the returned char in rows
 *   width  - The max width of the returned char in pixels
 *   stride - The width of the destination buffer in bytes
 *   bm     - Describes the character glyph to convert
 *   color  - The color to use for '1' bits in the font bitmap
 *            (0 bits are transparent)
 *
 * Returned Value:
 *  OK on Success, ERROR: on failure with errno set appropriately.
 *  (never fails)
 *
 ****************************************************************************/

EXTERN int nxf_convert_2bpp(FAR uint8_t *dest, uint16_t height,
                            uint16_t width, uint16_t stride,
                            FAR const struct nx_fontbitmap_s *bm,
                            nxgl_mxpixel_t color);
EXTERN int nxf_convert_4bpp(FAR uint8_t *dest, uint16_t height,
                            uint16_t width, uint16_t stride,
                            FAR const struct nx_fontbitmap_s *bm,
                            nxgl_mxpixel_t color);
EXTERN int nxf_convert_8bpp(FAR uint8_t *dest, uint16_t height,
                            uint16_t width, uint16_t stride,
                            FAR const struct nx_fontbitmap_s *bm,
                            nxgl_mxpixel_t color);
EXTERN int nxf_convert_16bpp(FAR uint16_t *dest, uint16_t height,
                             uint16_t width, uint16_t stride,
                             FAR const struct nx_fontbitmap_s *bm,
                             nxgl_mxpixel_t color);
EXTERN int nxf_convert_24bpp(FAR uint32_t *dest, uint16_t height,
                             uint16_t width, uint16_t stride,
                             FAR const struct nx_fontbitmap_s *bm,
                             nxgl_mxpixel_t color);
EXTERN int nxf_convert_32bpp(FAR uint32_t *dest, uint16_t height,
                             uint16_t width, uint16_t stride,
                             FAR const struct nx_fontbitmap_s *bm,
                             nxgl_mxpixel_t color);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_NUTTX_NXFONTS_H */
