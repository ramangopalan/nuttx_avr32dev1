/****************************************************************************
 * fs/fat/fs_fat32.h
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

#ifndef __FS_FAT_FS_FAT32_H
#define __FS_FAT_FS_FAT32_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx_config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

/****************************************************************************
 * Definitions
 ****************************************************************************/

/****************************************************************************
 * These offsets describes the master boot record.
 *
 * The folowing fields are common to FAT12/16/32 (but all value descriptions
 * refer to the interpretation under FAT32.
 */

#define BS_JUMP             0 /*  3@0:  Jump instruction to boot code (ignored) */
#define BS_OEMNAME          3 /*  8@3:  Usually "MSWIN4.1" */
#define BS_BYTESPERSEC     11 /*  2@11: Bytes per sector: 512, 1024, 2048, 4096  */
#define BS_SECPERCLUS      13 /*  1@13: Sectors per allocation unit: 2**n, n=0..7 */
#define BS_RESVDSECCOUNT   14 /*  2@14: Reserved sector count: Usually 32 */
#define BS_NUMFATS         16 /*  1@16: Number of FAT data structures: always 2 */
#define BS_ROOTENTCNT      17 /*  2@17: FAT12/16: Must be 0 for FAT32 */
#define BS_TOTSEC16        19 /*  2@19: FAT12/16: Must be 0, see BS_TOTSEC32 */
#define BS_MEDIA           21 /*  1@21: Media code: f0, f8, f9-fa, fc-ff */ 
#define BS_FATSZ16         22 /*  2@22: FAT12/16: Must be 0, see BS_FATSZ32 */
#define BS_SECPERTRK       24 /*  2@24: Sectors per track geometry value */
#define BS_NUMHEADS        26 /*  2@26: Number of heads geometry value */
#define BS_HIDSEC          28 /*  4@28: Count of hidden sectors preceding FAT */
#define BS_TOTSEC32        32 /*  4@32: Total count of sectors on the volume */

/* The following fields are only valid for FAT12/16 */

#define BS16_DRVNUM        36 /*  1@36: Drive number for MSDOS bootstrap */
                              /*  1@37: Reserved (zero) */
#define BS16_BOOTSIG       38 /*  1@38: Extended boot signature: 0x29 if following valid */
#define BS16_VOLID         39 /*  4@39: Volume serial number */
#define BS16_VOLLAB        43 /* 11@43: Volume label */
#define BS16_FILESYSTYPE   54 /*  8@54: "FAT12  ", "FAT16  ", or "FAT    " */

#define BS16_BOOTCODE      62 /* Boot code may be placed in the remainder of the sector */
#define BS16_BOOTCODESIZE 448

/* The following fields are only valid for FAT32 */

#define BS32_FATSZ32       36 /*  4@36: Count of sectors occupied by one FAT */
#define BS32_EXTFLAGS      40 /*  2@40: 0-3:Active FAT, 7=0 both FATS, 7=1 one FAT */ 
#define BS32_FSVER         42 /*  2@42: MSB:Major LSB:Minor revision number (0.0) */
#define BS32_ROOTCLUS      44 /*  4@44: Cluster no. of 1st cluster of root dir */
#define BS32_FSINFO        48 /*  2@48: Sector number of fsinfo structure. Usually 1. */
#define BS32_BKBOOTSEC     50 /*  2@50: Sector number of boot record. Usually 6  */
                              /* 12@52: Reserved (zero) */
#define BS32_DRVNUM        64 /*  1@64: Drive number for MSDOS bootstrap */
                              /*  1@65: Reserved (zero) */
#define BS32_BOOTSIG       66 /*  1@66: Extended boot signature: 0x29 if following valid */
#define BS32_VOLID         67 /*  4@67: Volume serial number */
#define BS32_VOLLAB        71 /* 11@71: Volume label */
#define BS32_FILESYSTYPE   82 /*  8@82: "FAT12  ", "FAT16  ", or "FAT    " */

#define BS32_BOOTCODE      90 /* Boot code may be placed in the remainder of the sector */
#define BS32_BOOTCODESIZE 420

/* If the sector is not an MBR, then it could have a partition table at
 * this offset.
 */

#define MBR_TABLE         446

/* The magic bytes at the end of the MBR are common to FAT12/16/32 */

#define BS_SIGNATURE      510 /*  2@510: Valid MBRs have 0x55aa here */

#define BOOT_SIGNATURE16  0xaa55
#define BOOT_SIGNATURE32  0xaa550000

/* The extended boot signature (BS16/32_BOOTSIG) */

#define EXTBOOT_SIGNATURE 0x29

/****************************************************************************
 * These offsets describes the partition table.
 */
                              /* 446@0: Generally unused and zero; but may
                               * include IDM Boot Manager menu entry at 8@394 */
#define PART_ENTRY1       446 /* 16@446: Partition table, first entry */
#define PART_ENTRY2       462 /* 16@462: Partition table, second entry */
                              /* 32@478: Unused, should be zero */
#define PART_SIGNATURE        /* 2@510: Valid partitions have 0x55aa here */

/****************************************************************************
 * These offsets describes one partition table entry.  NOTE that ent entries
 * are aligned to 16-bit offsets so that the STARTSECTOR and SIZE values are
 * not properly aligned.
 */

#define PART_BOOTINDICATOR  0  /* 1@0:  Boot indicator (0x80: active;0x00:otherwise) */
#define PART_STARTCHS       1  /* 3@1:  Starting Cylinder/Head/Sector values */
#define PART_TYPE           4  /* 1@4:  Partition type description */
#define PART_ENDCHS         5  /* 3@5:  Ending Cylinder/Head/Sector values */
#define PART_STARTSECTOR    8  /* 4@8:  Starting sector */
#define PART_SIZE          12  /* 4@12: Partition size (in sectors) */

/****************************************************************************
 * Each FAT directory entry is 32-bytes long.  The following define offsets
 * relative to the beginning of a directory entry.
 */

#define DIR_NAME            0 /* 11@ 0: NAME: 8 bytes + 3 byte extension */
#define DIR_ATTRIBUTES     11 /*  1@11: File attibutes (see below) */
#define DIR_NTRES          12 /*  1@12: Reserved for use by NT */
#define DIR_CRTTIMETENTH   13 /*  1@13: Tenth sec creation timestamp */
#define DIR_CRTIME         14 /*  2@14: Time file created */
#define DIR_CRDATE         16 /*  2@16: Date file created */
#define DIR_LASTACCDATE    18 /*  2@19: Last access date */
#define DIR_FSTCLUSTHI     20 /*  2@20: MS first cluster number */
#define DIR_WRTTIME        22 /*  2@22: Time of last write */
#define DIR_WRTDATE        24 /*  2@24: Date of last write */
#define DIR_FSTCLUSTLO     26 /*  2@26: LS first cluster number */
#define DIR_FILESIZE       28 /*  4@28: File size in bytes */
#define DIR_SIZE           32 /* The size of one directory entry */
#define DIR_SHIFT           5 /* log2 of DIR_SIZE */

/* First byte of the directory name has special meanings: */

#define DIR0_EMPTY        0xe5 /* The directory entry is empty */
#define DIR0_ALLEMPTY     0x00 /* This entry and all following are empty */
#define DIR0_E5           0x05 /* The actual value is 0xe5 */

/* NTRES flags in the FAT directory */

#define FATNTRES_LCNAME   0x08 /* Lower case in name */
#define FATNTRES_LCEXT    0x10 /* Lower case in extension */

/* Directory indexing helper.  Each directory entry is 32-bytes in length.
 * The number of directory entries in a sector then varies with the size
 * of the sector supported in hardware.
 */

#define DIRSEC_NDXMASK(f)   (((f)->fs_hwsectorsize - 1) >> 5)
#define DIRSEC_NDIRS(f)     (((f)->fs_hwsectorsize) >> 5)
#define DIRSEC_BYTENDX(f,i) (((i) & DIRSEC_NDXMASK(fs)) << 5)

#define SEC_NDXMASK(f)      ((f)->fs_hwsectorsize - 1)
#define SEC_NSECTORS(f,n)   ((n) / (f)->fs_hwsectorsize)

#define CLUS_NDXMASK(f)     ((f)->fs_fatsecperclus - 1)

/****************************************************************************
 * File system types */

#define FSTYPE_FAT12        0
#define FSTYPE_FAT16        1
#define FSTYPE_FAT32        2

/* File buffer flags */

#define FFBUFF_VALID        1
#define FFBUFF_DIRTY        2
#define FFBUFF_MODIFIED     4

/****************************************************************************
 * These offset describe the FSINFO sector
 */

#define FSI_LEADSIG         0 /*   4@0:   0x41615252  = "RRaA" */
                              /* 480@4:   Reserved (zero) */
#define FSI_STRUCTSIG     484 /*   4@484: 0x61417272 = "rrAa" */
#define FSI_FREECOUNT     488 /*   4@488: Last free cluster count on volume */
#define FSI_NXTFREE       492 /*   4@492: Cluster number of 1st free cluster */
                              /*  12@496: Reserved (zero) */
#define FSI_TRAILSIG      508 /*   4@508: 0xaa550000 */

/****************************************************************************
 * FAT values
 */

#define FAT_EOF      0x0ffffff8
#define FAT_BAD      0x0ffffff7

/****************************************************************************
 * Maximum cluster by FAT type.  This is the key value used to distinquish
 * between FAT12, 16, and 32.
 */

/* FAT12: For M$, the calculation is ((1 << 12) - 19).  But we will follow the
 * Linux tradition of allowing slightly more clusters for FAT12.
 */

#define FAT_MAXCLUST12 ((1 << 12) - 16)

/* FAT16: For M$, the calculation is ((1 << 16) - 19). */

#define FAT_MINCLUST16 (FAT_MAXCLUST12 + 1)
#define FAT_MAXCLUST16 ((1 << 16) - 16)

/* FAT32: M$ reserves the MS 4 bits of a FAT32 FAT entry so only 18 bits are
 * available.  For M$, the calculation is ((1 << 28) - 19).
 */

#define FAT_MINCLUST32  65524
/* #define FAT_MINCLUST32  (FAT_MAXCLUST16 + 1) */
#define FAT_MAXCLUST32  ((1 << 28) - 16)


/****************************************************************************
 * Access to data in raw sector data */

#define UBYTE_VAL(p,o)            (((uint8_t*)(p))[o])
#define UBYTE_PTR(p,o)            &UBYTE_VAL(p,o)
#define UBYTE_PUT(p,o,v)          (UBYTE_VAL(p,o)=(uint8_t)(v))

#define UINT16_PTR(p,o)           ((uint16_t*)UBYTE_PTR(p,o))
#define UINT16_VAL(p,o)           (*UINT16_PTR(p,o))
#define UINT16_PUT(p,o,v)         (UINT16_VAL(p,o)=(uint16_t)(v))

#define UINT32_PTR(p,o)           ((uint32_t*)UBYTE_PTR(p,o))
#define UINT32_VAL(p,o)           (*UINT32_PTR(p,o))
#define UINT32_PUT(p,o,v)         (UINT32_VAL(p,o)=(uint32_t)(v))

/* Regardless of the endian-ness of the target or alignment of the data, no
 * special operations are required for byte, string or byte array accesses.
 * The FAT data stream is little endian so multiple byte values must be
 * accessed byte-by-byte for big-endian targets.
 */

#define MBR_GETSECPERCLUS(p)      UBYTE_VAL(p,BS_SECPERCLUS)
#define MBR_GETNUMFATS(p)         UBYTE_VAL(p,BS_NUMFATS)
#define MBR_GETMEDIA(p)           UBYTE_VAL(p,BS_MEDIA)
#define MBR_GETDRVNUM16(p)        UBYTE_VAL(p,BS16_DRVNUM)
#define MBR_GETDRVNUM32(p)        UBYTE_VAL(p,BS32_DRVNUM)
#define MBR_GETBOOTSIG16(p)       UBYTE_VAL(p,BS16_BOOTSIG)
#define MBR_GETBOOTSIG32(p)       UBYTE_VAL(p,BS32_BOOTSIG)

#define PART1_GETTYPE(p)          UBYTE_VAL(p,PART_ENTRY1+PART_TYPE)
#define PART2_GETTYPE(p)          UBYTE_VAL(p,PART_ENTRY2+PART_TYPE)

#define DIR_GETATTRIBUTES(p)      UBYTE_VAL(p,DIR_ATTRIBUTES)
#define DIR_GETNTRES(p)           UBYTE_VAL(p,DIR_NTRES)
#define DIR_GETCRTTIMETENTH(p)    UBYTE_VAL(p,DIR_CRTTIMETENTH)

#define MBR_PUTSECPERCLUS(p,v)    UBYTE_PUT(p,BS_SECPERCLUS,v)
#define MBR_PUTNUMFATS(p,v)       UBYTE_PUT(p,BS_NUMFATS,v)
#define MBR_PUTMEDIA(p,v)         UBYTE_PUT(p,BS_MEDIA,v)
#define MBR_PUTDRVNUM16(p,v)      UBYTE_PUT(p,BS16_DRVNUM,v)
#define MBR_PUTDRVNUM32(p,v)      UBYTE_PUT(p,BS32_DRVNUM,v)
#define MBR_PUTBOOTSIG16(p,v)     UBYTE_PUT(p,BS16_BOOTSIG,v)
#define MBR_PUTBOOTSIG32(p,v)     UBYTE_PUT(p,BS32_BOOTSIG,v)

#define PART1_PUTTYPE(p,v)        UBYTE_PUT(p,PART_ENTRY1+PART_TYPE,v)
#define PART2_PUTTYPE(p,v)        UBYTE_PUT(p,PART_ENTRY2+PART_TYPE,v)

#define DIR_PUTATTRIBUTES(p,v)    UBYTE_PUT(p,DIR_ATTRIBUTES,v)
#define DIR_PUTNTRES(p,v)         UBYTE_PUT(p,DIR_NTRES,v)
#define DIR_PUTCRTTIMETENTH(p,v)  UBYTE_PUT(p,DIR_CRTTIMETENTH,v)

/* For the all targets, unaligned values need to be accessed byte-by-byte.
 * Some architectures may handle unaligned accesses with special interrupt
 * handlers.  But even in that case, it is more efficient to avoid the traps.
 */

/* Unaligned multi-byte access macros */

#define MBR_GETBYTESPERSEC(p)      fat_getuint16(UBYTE_PTR(p,BS_BYTESPERSEC))
#define MBR_GETROOTENTCNT(p)       fat_getuint16(UBYTE_PTR(p,BS_ROOTENTCNT))
#define MBR_GETTOTSEC16(p)         fat_getuint16(UBYTE_PTR(p,BS_TOTSEC16))
#define MBR_GETVOLID16(p)          fat_getuint32(UBYTE_PTR(p,BS16_VOLID))
#define MBR_GETVOLID32(p)          fat_getuint32(UBYTE_PTR(p,BS32_VOLID))

#define PART1_GETSTARTSECTOR(p)    fat_getuint32(UBYTE_PTR(p,PART_ENTRY1+PART_STARTSECTOR))
#define PART1_GETSIZE(p)           fat_getuint32(UBYTE_PTR(p,PART_ENTRY1+PART_SIZE))
#define PART2_GETSTARTSECTOR(p)    fat_getuint32(UBYTE_PTR(p,PART_ENTRY2+PART_STARTSECTOR))
#define PART2_GETSIZE(p)           fat_getuint32(UBYTE_PTR(p,PART_ENTRY2+PART_SIZE))

#define MBR_PUTBYTESPERSEC(p,v)    fat_putuint16(UBYTE_PTR(p,BS_BYTESPERSEC),v)
#define MBR_PUTROOTENTCNT(p,v)     fat_putuint16(UBYTE_PTR(p,BS_ROOTENTCNT),v)
#define MBR_PUTTOTSEC16(p,v)       fat_putuint16(UBYTE_PTR(p,BS_TOTSEC16),v)
#define MBR_PUTVOLID16(p,v)        fat_putuint32(UBYTE_PTR(p,BS16_VOLID),v)
#define MBR_PUTVOLID32(p,v)        fat_putuint32(UBYTE_PTR(p,BS32_VOLID),v)

#define PART1_PUTSTARTSECTOR(p,v) fat_putuint32(UBYTE_PTR(p,PART_ENTRY1+PART_STARTSECTOR),v)
#define PART1_PUTSIZE(p,v)        fat_putuint32(UBYTE_PTR(p,PART_ENTRY1+PART_SIZE),v)
#define PART2_PUTSTARTSECTOR(p,v) fat_putuint32(UBYTE_PTR(p,PART_ENTRY2+PART_STARTSECTOR),v)
#define PART2_PUTSIZE(p,v)        fat_putuint32(UBYTE_PTR(p,PART_ENTRY2+PART_SIZE),v)

/* But for multi-byte values, the endian-ness of the target vs. the little
 * endian order of the byte stream or alignment of the data within the byte
 * stream can force special, byte-by-byte accesses.
 */

#ifdef CONFIG_ENDIAN_BIG

/* If the target is big-endian, then even aligned multi-byte values must be
 * accessed byte-by-byte.
 */

# define MBR_GETRESVDSECCOUNT(p)   fat_getuint16(UBYTE_PTR(p,BS_RESVDSECCOUNT))
# define MBR_GETFATSZ16(p)         fat_getuint16(UBYTE_PTR(p,BS_FATSZ16))
# define MBR_GETSECPERTRK(p)       fat_getuint16(UBYTE_PTR(p,BS_SECPERTRK))
# define MBR_GETNUMHEADS(p)        fat_getuint16(UBYTE_PTR(p,BS_NUMHEADS))
# define MBR_GETHIDSEC(p)          fat_getuint32(UBYTE_PTR(p,BS_HIDSEC))
# define MBR_GETTOTSEC32(p)        fat_getuint32(UBYTE_PTR(p,BS_TOTSEC32))
# define MBR_GETFATSZ32(p)         fat_getuint32(UBYTE_PTR(p,BS32_FATSZ32))
# define MBR_GETEXTFLAGS(p)        fat_getuint16(UBYTE_PTR(p,BS32_EXTFLAGS))
# define MBR_GETFSVER(p)           fat_getuint16(UBYTE_PTR(p,BS32_FSVER))
# define MBR_GETROOTCLUS(p)        fat_getuint32(UBYTE_PTR(p,BS32_ROOTCLUS))
# define MBR_GETFSINFO(p)          fat_getuint16(UBYTE_PTR(p,BS32_FSINFO))
# define MBR_GETBKBOOTSEC(p)       fat_getuint16(UBYTE_PTR(p,BS32_BKBOOTSEC))
# define MBR_GETSIGNATURE(p)       fat_getuint16(UBYTE_PTR(p,BS_SIGNATURE))

# define FSI_GETLEADSIG(p)         fat_getuint32(UBYTE_PTR(p,FSI_LEADSIG))
# define FSI_GETSTRUCTSIG(p)       fat_getuint32(UBYTE_PTR(p,FSI_STRUCTSIG))
# define FSI_GETFREECOUNT(p)       fat_getuint32(UBYTE_PTR(p,FSI_FREECOUNT))
# define FSI_GETNXTFREE(p)         fat_getuint32(UBYTE_PTR(p,FSI_NXTFREE))
# define FSI_GETTRAILSIG(p)        fat_getuint32(UBYTE_PTR(p,FSI_TRAILSIG))

# define DIR_GETCRTIME(p)          fat_getuint16(UBYTE_PTR(p,DIR_CRTIME))
# define DIR_GETCRDATE(p)          fat_getuint16(UBYTE_PTR(p,DIR_CRDATE))
# define DIR_GETLASTACCDATE(p)     fat_getuint16(UBYTE_PTR(p,DIR_LASTACCDATE))
# define DIR_GETFSTCLUSTHI(p)      fat_getuint16(UBYTE_PTR(p,DIR_FSTCLUSTHI))
# define DIR_GETWRTTIME(p)         fat_getuint16(UBYTE_PTR(p,DIR_WRTTIME))
# define DIR_GETWRTDATE(p)         fat_getuint16(UBYTE_PTR(p,DIR_WRTDATE))
# define DIR_GETFSTCLUSTLO(p)      fat_getuint16(UBYTE_PTR(p,DIR_FSTCLUSTLO))
# define DIR_GETFILESIZE(p)        fat_getuint32(UBYTE_PTR(p,DIR_FILESIZE))

# define FSI_GETLEADSIG(p)         fat_getuint32(UBYTE_PTR(p,FSI_LEADSIG))
# define FSI_GETSTRUCTSIG(p)       fat_getuint32(UBYTE_PTR(p,FSI_STRUCTSIG))
# define FSI_GETFREECOUNT(p)       fat_getuint32(UBYTE_PTR(p,FSI_FREECOUNT))
# define FSI_GETNXTFREE(p)         fat_getuint32(UBYTE_PTR(p,FSI_NXTFREE))
# define FSI_GETTRAILSIG(p)        fat_getuint32(UBYTE_PTR(p,FSI_TRAILSIG))

# define FAT_GETFAT16(p,i)         fat_getuint16(UBYTE_PTR(p,i))
# define FAT_GETFAT32(p,i)         fat_getuint32(UBYTE_PTR(p,i))

# define MBR_PUTRESVDSECCOUNT(p,v) fat_putuint16(UBYTE_PTR(p,BS_RESVDSECCOUNT),v)
# define MBR_PUTFATSZ16(p,v)       fat_putuint16(UBYTE_PTR(p,BS_FATSZ16),v)
# define MBR_PUTSECPERTRK(p,v)     fat_putuint16(UBYTE_PTR(p,BS_SECPERTRK),v)
# define MBR_PUTNUMHEADS(p,v)      fat_putuint16(UBYTE_PTR(p,BS_NUMHEADS),v)
# define MBR_PUTHIDSEC(p,v)        fat_putuint32(UBYTE_PTR(p,BS_HIDSEC),v)
# define MBR_PUTTOTSEC32(p,v)      fat_putuint32(UBYTE_PTR(p,BS_TOTSEC32),v)
# define MBR_PUTFATSZ32(p,v)       fat_putuint32(UBYTE_PTR(p,BS32_FATSZ32),v)
# define MBR_PUTEXTFLAGS(p,v)      fat_putuint16(UBYTE_PTR(p,BS32_EXTFLAGS),v)
# define MBR_PUTFSVER(p,v)         fat_putuint16(UBYTE_PTR(p,BS32_FSVER),v)
# define MBR_PUTROOTCLUS(p,v)      fat_putuint32(UBYTE_PTR(p,BS32_ROOTCLUS),v)
# define MBR_PUTFSINFO(p,v)        fat_putuint16(UBYTE_PTR(p,BS32_FSINFO),v)
# define MBR_PUTBKBOOTSEC(p,v)     fat_putuint16(UBYTE_PTR(p,BS32_BKBOOTSEC),v)
# define MBR_PUTSIGNATURE(p,v)     fat_putuint16(UBYTE_PTR(p,BS_SIGNATURE),v)

# define FSI_PUTLEADSIG(p,v)       fat_putuint32(UBYTE_PTR(p,FSI_LEADSIG),v)
# define FSI_PUTSTRUCTSIG(p,v)     fat_putuint32(UBYTE_PTR(p,FSI_STRUCTSIG),v)
# define FSI_PUTFREECOUNT(p,v)     fat_putuint32(UBYTE_PTR(p,FSI_FREECOUNT),v)
# define FSI_PUTNXTFREE(p,v)       fat_putuint32(UBYTE_PTR(p,FSI_NXTFREE),v)
# define FSI_PUTTRAILSIG(p,v)      fat_putuint32(UBYTE_PTR(p,FSI_TRAILSIG),v)

# define DIR_PUTCRTIME(p,v)        fat_putuint16(UBYTE_PTR(p,DIR_CRTIME),v)
# define DIR_PUTCRDATE(p,v)        fat_putuint16(UBYTE_PTR(p,DIR_CRDATE),v)
# define DIR_PUTLASTACCDATE(p,v)   fat_putuint16(UBYTE_PTR(p,DIR_LASTACCDATE),v)
# define DIR_PUTFSTCLUSTHI(p,v)    fat_putuint16(UBYTE_PTR(p,DIR_FSTCLUSTHI),v)
# define DIR_PUTWRTTIME(p,v)       fat_putuint16(UBYTE_PTR(p,DIR_WRTTIME),v)
# define DIR_PUTWRTDATE(p,v)       fat_putuint16(UBYTE_PTR(p,DIR_WRTDATE),v)
# define DIR_PUTFSTCLUSTLO(p,v)    fat_putuint16(UBYTE_PTR(p,DIR_FSTCLUSTLO),v)
# define DIR_PUTFILESIZE(p,v)      fat_putuint32(UBYTE_PTR(p,DIR_FILESIZE),v)

# define FSI_PUTLEADSIG(p,v)       fat_putuint32(UBYTE_PTR(p,FSI_LEADSIG),v)
# define FSI_PUTSTRUCTSIG(p,v)     fat_putuint32(UBYTE_PTR(p,FSI_STRUCTSIG),v)
# define FSI_PUTFREECOUNT(p,v)     fat_putuint32(UBYTE_PTR(p,FSI_FREECOUNT),v)
# define FSI_PUTNXTFREE(p,v)       fat_putuint32(UBYTE_PTR(p,FSI_NXTFREE),v)
# define FSI_PUTTRAILSIG(p,v)      fat_putuint32(UBYTE_PTR(p,FSI_TRAILSIG),v)

# define FAT_PUTFAT16(p,i,v)       fat_putuint16(UBYTE_PTR(p,i),v)
# define FAT_PUTFAT32(p,i,v)       fat_putuint32(UBYTE_PTR(p,i),v)

#else

/* But nothing special has to be done for the little endian-case for access
 * to aligned mulitbyte values.
 */

# define MBR_GETRESVDSECCOUNT(p)   UINT16_VAL(p,BS_RESVDSECCOUNT)
# define MBR_GETFATSZ16(p)         UINT16_VAL(p,BS_FATSZ16)
# define MBR_GETSECPERTRK(p)       UINT16_VAL(p,BS_SECPERTRK)
# define MBR_GETNUMHEADS(p)        UINT16_VAL(p,BS_NUMHEADS)
# define MBR_GETHIDSEC(p)          UINT32_VAL(p,BS_HIDSEC)
# define MBR_GETTOTSEC32(p)        UINT32_VAL(p,BS_TOTSEC32)
# define MBR_GETFATSZ32(p)         UINT32_VAL(p,BS32_FATSZ32)
# define MBR_GETEXTFLAGS(p)        UINT16_VAL(p,BS32_EXTFLAGS)
# define MBR_GETFSVER(p)           UINT16_VAL(p,BS32_FSVER)
# define MBR_GETROOTCLUS(p)        UINT32_VAL(p,BS32_ROOTCLUS)
# define MBR_GETFSINFO(p)          UINT16_VAL(p,BS32_FSINFO)
# define MBR_GETBKBOOTSEC(p)       UINT16_VAL(p,BS32_BKBOOTSEC)
# define MBR_GETSIGNATURE(p)       UINT16_VAL(p,BS_SIGNATURE)

# define FSI_GETLEADSIG(p)         UINT32_VAL(p,FSI_LEADSIG)
# define FSI_GETSTRUCTSIG(p)       UINT32_VAL(p,FSI_STRUCTSIG)
# define FSI_GETFREECOUNT(p)       UINT32_VAL(p,FSI_FREECOUNT)
# define FSI_GETNXTFREE(p)         UINT32_VAL(p,FSI_NXTFREE)
# define FSI_GETTRAILSIG(p)        UINT32_VAL(p,FSI_TRAILSIG)

# define DIR_GETCRTIME(p)          UINT16_VAL(p,DIR_CRTIME)
# define DIR_GETCRDATE(p)          UINT16_VAL(p,DIR_CRDATE)
# define DIR_GETLASTACCDATE(p)     UINT16_VAL(p,DIR_LASTACCDATE)
# define DIR_GETFSTCLUSTHI(p)      UINT16_VAL(p,DIR_FSTCLUSTHI)
# define DIR_GETWRTTIME(p)         UINT16_VAL(p,DIR_WRTTIME)
# define DIR_GETWRTDATE(p)         UINT16_VAL(p,DIR_WRTDATE)
# define DIR_GETFSTCLUSTLO(p)      UINT16_VAL(p,DIR_FSTCLUSTLO)
# define DIR_GETFILESIZE(p)        UINT32_VAL(p,DIR_FILESIZE)

# define FSI_GETLEADSIG(p)         UINT32_VAL(p,FSI_LEADSIG)
# define FSI_GETSTRUCTSIG(p)       UINT32_VAL(p,FSI_STRUCTSIG)
# define FSI_GETFREECOUNT(p)       UINT32_VAL(p,FSI_FREECOUNT)
# define FSI_GETNXTFREE(p)         UINT32_VAL(p,FSI_NXTFREE)
# define FSI_GETTRAILSIG(p)        UINT32_VAL(p,FSI_TRAILSIG)

# define FAT_GETFAT16(p,i)         UINT16_VAL(p,i)
# define FAT_GETFAT32(p,i)         UINT32_VAL(p,i)

# define MBR_PUTRESVDSECCOUNT(p,v) UINT16_PUT(p,BS_RESVDSECCOUNT,v)
# define MBR_PUTFATSZ16(p,v)       UINT16_PUT(p,BS_FATSZ16,v)
# define MBR_PUTSECPERTRK(p,v)     UINT16_PUT(p,BS_SECPERTRK,v)
# define MBR_PUTNUMHEADS(p,v)      UINT16_PUT(p,BS_NUMHEADS,v)
# define MBR_PUTHIDSEC(p,v)        UINT32_PUT(p,BS_HIDSEC,v)
# define MBR_PUTTOTSEC32(p,v)      UINT32_PUT(p,BS_TOTSEC32,v)
# define MBR_PUTFATSZ32(p,v)       UINT32_PUT(p,BS32_FATSZ32,v)
# define MBR_PUTEXTFLAGS(p,v)      UINT16_PUT(p,BS32_EXTFLAGS,v)
# define MBR_PUTFSVER(p,v)         UINT16_PUT(p,BS32_FSVER,v)
# define MBR_PUTROOTCLUS(p,v)      UINT32_PUT(p,BS32_ROOTCLUS,v)
# define MBR_PUTFSINFO(p,v)        UINT16_PUT(p,BS32_FSINFO,v)
# define MBR_PUTBKBOOTSEC(p,v)     UINT16_PUT(p,BS32_BKBOOTSEC,v)
# define MBR_PUTSIGNATURE(p,v)     UINT16_PUT(p,BS_SIGNATURE,v)

# define FSI_PUTLEADSIG(p,v)       UINT32_PUT(p,FSI_LEADSIG,v)
# define FSI_PUTSTRUCTSIG(p,v)     UINT32_PUT(p,FSI_STRUCTSIG,v)
# define FSI_PUTFREECOUNT(p,v)     UINT32_PUT(p,FSI_FREECOUNT,v)
# define FSI_PUTNXTFREE(p,v)       UINT32_PUT(p,FSI_NXTFREE,v)
# define FSI_PUTTRAILSIG(p,v)      UINT32_PUT(p,FSI_TRAILSIG,v)

# define DIR_PUTCRTIME(p,v)        UINT16_PUT(p,DIR_CRTIME,v)
# define DIR_PUTCRDATE(p,v)        UINT16_PUT(p,DIR_CRDATE,v)
# define DIR_PUTLASTACCDATE(p,v)   UINT16_PUT(p,DIR_LASTACCDATE,v)
# define DIR_PUTFSTCLUSTHI(p,v)    UINT16_PUT(p,DIR_FSTCLUSTHI,v)
# define DIR_PUTWRTTIME(p,v)       UINT16_PUT(p,DIR_WRTTIME,v)
# define DIR_PUTWRTDATE(p,v)       UINT16_PUT(p,DIR_WRTDATE,v)
# define DIR_PUTFSTCLUSTLO(p,v)    UINT16_PUT(p,DIR_FSTCLUSTLO,v)
# define DIR_PUTFILESIZE(p,v)      UINT32_PUT(p,DIR_FILESIZE,v)

# define FSI_PUTLEADSIG(p,v)       UINT32_PUT(p,FSI_LEADSIG,v)
# define FSI_PUTSTRUCTSIG(p,v)     UINT32_PUT(p,FSI_STRUCTSIG,v)
# define FSI_PUTFREECOUNT(p,v)     UINT32_PUT(p,FSI_FREECOUNT,v)
# define FSI_PUTNXTFREE(p,v)       UINT32_PUT(p,FSI_NXTFREE,v)
# define FSI_PUTTRAILSIG(p,v)      UINT32_PUT(p,FSI_TRAILSIG,v)

# define FAT_PUTFAT16(p,i,v)       UINT16_PUT(p,i,v)
# define FAT_PUTFAT32(p,i,v)       UINT32_PUT(p,i,v)

#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* This structure represents the overall mountpoint state.  An instance of this
 * structure is retained as inode private data on each mountpoint that is
 * mounted with a fat32 filesystem.
 */

struct fat_file_s;
struct fat_mountpt_s
{
  struct inode      *fs_blkdriver; /* The block driver inode that hosts the FAT32 fs */
  struct fat_file_s *fs_head;      /* A list to all files opened on this mountpoint */

  sem_t    fs_sem;                 /* Used to assume thread-safe access */
  size_t   fs_hwsectorsize;        /* HW: Sector size reported by block driver*/
  size_t   fs_hwnsectors;          /* HW: The number of sectors reported by the hardware */
  size_t   fs_fatbase;             /* Logical block of start of filesystem (past resd sectors) */
  size_t   fs_rootbase;            /* MBR: Cluster no. of 1st cluster of root dir */
  size_t   fs_database;            /* Logical block of start data sectors */
  size_t   fs_fsinfo;              /* MBR: Sector number of FSINFO sector */
  size_t   fs_currentsector;       /* The sector number buffered in fs_buffer */
  uint32_t fs_nclusters;           /* Maximum number of data clusters */
  uint32_t fs_nfatsects;           /* MBR: Count of sectors occupied by one fat */
  uint32_t fs_fattotsec;           /* MBR: Total count of sectors on the volume */
  uint32_t fs_fsifreecount;        /* FSI: Last free cluster count on volume */
  uint32_t fs_fsinextfree;         /* FSI: Cluster number of 1st free cluster */
  uint16_t fs_fatresvdseccount;    /* MBR: The total number of reserved sectors */
  uint16_t fs_rootentcnt;          /* MBR: Count of 32-bit root directory entries */
  bool     fs_mounted;             /* true: The file system is ready */
  bool     fs_dirty;               /* true: fs_buffer is dirty */
  bool     fs_fsidirty;            /* true: FSINFO sector must be written to disk */
  uint8_t  fs_type;                /* FSTYPE_FAT12, FSTYPE_FAT16, or FSTYPE_FAT32 */
  uint8_t  fs_fatnumfats;          /* MBR: Number of FATs (probably 2) */
  uint8_t  fs_fatsecperclus;       /* MBR: Sectors per allocation unit: 2**n, n=0..7 */
  uint8_t *fs_buffer;              /* This is an allocated buffer to hold one sector
                                    * from the device */
};

/* This structure represents on open file under the mountpoint.  An instance
 * of this structure is retained as struct file specific information on each
 * opened file.
 */

struct fat_file_s
{
  struct fat_file_s *ff_next;      /* Retained in a singly linked list */
  bool     ff_open;                /* true: The file is (still) open */
  uint8_t  ff_bflags;              /* The file buffer flags */
  uint8_t  ff_oflags;              /* Flags provided when file was opened */
  uint8_t  ff_sectorsincluster;    /* Sectors remaining in cluster */
  uint16_t ff_dirindex;            /* Index into ff_dirsector to directory entry */
  uint32_t ff_currentcluster;      /* Current cluster being accessed */
  size_t   ff_dirsector;           /* Sector containing the directory entry */
  off_t    ff_size;                /* Size of the file in bytes */
  size_t   ff_startcluster;        /* Start cluster of file on media */
  size_t   ff_currentsector;       /* Current sector being operated on */
  size_t   ff_cachesector;         /* Current sector in the file buffer */
  uint8_t *ff_buffer;              /* File buffer (for partial sector accesses) */
};

/* This structure is used internally for describing directory entries */

struct fat_dirinfo_s
{
  uint8_t  fd_name[8+3];           /* Filename -- directory format*/
#ifdef CONFIG_FAT_LCNAMES
  uint8_t  fd_ntflags;             /* NTRes lower case flags */
#endif
  struct fs_fatdir_s dir;          /* Used with opendir, readdir, etc. */
  uint8_t *fd_entry;               /* A pointer to the raw 32-byte entry */
};

/****************************************************************************
 * Global Variables
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

/* Utitilies to handle unaligned or byte swapped accesses */

EXTERN uint16_t fat_getuint16(uint8_t *ptr);
EXTERN uint32_t fat_getuint32(uint8_t *ptr);
EXTERN void   fat_putuint16(uint8_t *ptr, uint16_t value16);
EXTERN void   fat_putuint32(uint8_t *ptr, uint32_t value32);

/* Manage the per-mount semaphore that protects access to shared resources */

EXTERN void   fat_semtake(struct fat_mountpt_s *fs);
EXTERN void   fat_semgive(struct fat_mountpt_s *fs);

/* Get the current time for FAT creation and write times */

EXTERN uint32_t fat_systime2fattime(void);
EXTERN time_t fat_fattime2systime(uint16_t time, uint16_t date);

/* Handle hardware interactions for mounting */

EXTERN int    fat_mount(struct fat_mountpt_s *fs, bool writeable);
EXTERN int    fat_checkmount(struct fat_mountpt_s *fs);

/* low-level hardware access */

EXTERN int    fat_hwread(struct fat_mountpt_s *fs, uint8_t *buffer,
                         size_t sector, unsigned int nsectors);
EXTERN int    fat_hwwrite(struct fat_mountpt_s *fs, uint8_t *buffer,
                          size_t sector, unsigned int nsectors);

/* Cluster / cluster chain access helpers */

EXTERN ssize_t fat_cluster2sector(struct fat_mountpt_s *fs,  uint32_t cluster);
EXTERN ssize_t fat_getcluster(struct fat_mountpt_s *fs, uint32_t clusterno);
EXTERN int    fat_putcluster(struct fat_mountpt_s *fs, uint32_t clusterno,
                             size_t startsector);
EXTERN int    fat_removechain(struct fat_mountpt_s *fs, uint32_t cluster);
EXTERN int32_t fat_extendchain(struct fat_mountpt_s *fs, uint32_t cluster);

#define fat_createchain(fs) fat_extendchain(fs, 0)

/* Help for traversing directory trees */

EXTERN int    fat_nextdirentry(struct fat_mountpt_s *fs, struct fs_fatdir_s *dir);
EXTERN int    fat_finddirentry(struct fat_mountpt_s *fs, struct fat_dirinfo_s *dirinfo,
                               const char *path);
EXTERN int    fat_allocatedirentry(struct fat_mountpt_s *fs, struct fat_dirinfo_s *dirinfo);

EXTERN int    fat_dirname2path(char *path, uint8_t *direntry);

/* File creation and removal helpers */

EXTERN int    fat_dirtruncate(struct fat_mountpt_s *fs, struct fat_dirinfo_s *dirinfo);
EXTERN int    fat_dircreate(struct fat_mountpt_s *fs, struct fat_dirinfo_s *dirinfo);
EXTERN int    fat_remove(struct fat_mountpt_s *fs, const char *relpath, bool directory);

/* Mountpoint and file buffer cache (for partial sector accesses) */

EXTERN int    fat_fscacheflush(struct fat_mountpt_s *fs);
EXTERN int    fat_fscacheread(struct fat_mountpt_s *fs, size_t sector);
EXTERN int    fat_ffcacheflush(struct fat_mountpt_s *fs, struct fat_file_s *ff);
EXTERN int    fat_ffcacheread(struct fat_mountpt_s *fs, struct fat_file_s *ff, size_t sector);
EXTERN int    fat_ffcacheinvalidate(struct fat_mountpt_s *fs, struct fat_file_s *ff);

/* FSINFO sector support */

EXTERN int    fat_updatefsinfo(struct fat_mountpt_s *fs);
EXTERN int    fat_nfreeclusters(struct fat_mountpt_s *fs, size_t *pfreeclusters);
EXTERN int    fat_currentsector(struct fat_mountpt_s *fs, struct fat_file_s *ff, off_t position);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __FS_FAT_FS_FAT32_H */
