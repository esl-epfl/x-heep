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

#include "doom_config.h"

#include "doomtype.h"
#include "m_argv.h"

#include "w_file.h"

extern wad_file_class_t fatfs_wad_file;


wad_file_t *W_OpenFile(char *path)
{
    wad_file_t *result;
    int i;

    return fatfs_wad_file.OpenFile(path);

    // Try all classes in order until we find one that works

    /* NRFD-TODO?
    result = NULL;

    for (i=0; i<arrlen(wad_file_classes); ++i)
    {
        result = wad_file_classes[i]->OpenFile(path);

        if (result != NULL)
        {
            break;
        }
    }

    return result;
    */
}

void W_CloseFile(wad_file_t *wad)
{
    // wad->file_class->CloseFile(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
    return 0;
    // return wad->file_class->Read(wad, offset, buffer, buffer_len);
}

