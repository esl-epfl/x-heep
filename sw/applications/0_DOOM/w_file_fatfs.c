//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <stdio.h>

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

//#include "ff.h"

/* X-HEEP COMMENT
typedef struct
{
    wad_file_t wad;
    FIL fstream;
} fatfs_wad_file_t;
X-HEEP COMMENT END */


extern wad_file_class_t fatfs_wad_file;

static wad_file_t *W_FatFS_OpenFile(char *path)
{
    return NULL; /*
    PRINTF("FatFS: Opening: %s\n", path);
    fatfs_wad_file_t *result;
    FIL fstream;
    FRESULT ff_result;

    ff_result = f_open(&fstream, path, FA_READ);
    if (ff_result != FR_OK)
    {
        return NULL;
    }

    // Create a new fatfs_wad_file_t to hold the file handle.

    result = Z_Malloc(sizeof(fatfs_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &fatfs_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = f_size(&fstream);
    result->wad.path = M_StringDuplicate(path);
    result->fstream = fstream;

    PRINTF("FatFS: Open OK\n");
    return &result->wad;
    */
}

static void W_FatFS_CloseFile(wad_file_t *wad)
{
    /*
    fatfs_wad_file_t *fatfs_wad;

    fatfs_wad = (fatfs_wad_file_t *) wad;

    (void) f_close(&fatfs_wad->fstream);

    Z_Free(fatfs_wad);
    */
}

// Read data from the specified position in the file into the 
// provided buffer.  Returns the number of bytes read.

size_t W_FatFS_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    return 0;
    /*
    fatfs_wad_file_t *fatfs_wad;
    size_t result;
    FRESULT ff_result;

    fatfs_wad = (fatfs_wad_file_t *) wad;

    // Jump to the specified position in the file.

    ff_result = f_lseek(&fatfs_wad->fstream, offset);
    if (ff_result != FR_OK)
    {
        PRINTF("W_FatFS_Read: seek failed\n");
        return 0;
    }
    // Read into the buffer.

    ff_result = f_read(&fatfs_wad->fstream, buffer, buffer_len, &result);
    if (ff_result != FR_OK)
    {
        PRINTF("W_FatFS_Read: seek failed\n");
        return 0;
    }

    return result;
    */
}


wad_file_class_t fatfs_wad_file = 
{
    W_FatFS_OpenFile,
    W_FatFS_CloseFile,
    W_FatFS_Read,
};


