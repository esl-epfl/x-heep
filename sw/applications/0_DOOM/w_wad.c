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
//      Handles WAD file header, directory, lump I/O.
//




#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef SEGGER
#include <strings.h>
#endif

#include "doomtype.h"

#include "i_timer.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_diskicon.h"
#include "z_zone.h"

#include "w_wad.h"

#include "x_buttons.h"
#include "x_spi.h"

typedef PACKED_STRUCT (
{
    // Should be "IWAD" or "PWAD".
    char                identification[4];
    int                 numlumps;
    int                 infotableofs;
}) wadinfo_t;


typedef PACKED_STRUCT (
{
    int                 filepos;
    int                 size;
    char                name[8];
}) filelump_t;


//#include "n_fs.h"
//#include "n_qspi.h"
//#include "n_mem.h"


extern int no_sdcard; //NRFD-NOTE: from main.c
//
// GLOBALS
//

// wad_file_t *wad_file = NULL; // NRFD

// Location of each lump on disk.
#define MAX_NUMLUMPS 1300
lumpinfo_t lumpinfo[MAX_NUMLUMPS];
unsigned short numlumps = 0;

filelump_t *filelumps;

// Hash table for fast lookups
static lumpindex_t *lumphash = NULL;

//X-HEEP COMMENT: N_FILE wad_file;
int first_lump_pos;

/*
int debugLumpCount = 0;
int debugLumpNums[12];
void *debugLumpCache[12];
*/

// Variables for the reload hack: filename of the PWAD to reload, and the
// lumps from WADs before the reload file, so we can resent numlumps and
// load the file again.
/* NRFD-EXCLUDE
static wad_file_t *reloadhandle = NULL;
static lumpinfo_t *reloadlumps = NULL;
static char *reloadname = NULL;
static int reloadlump = -1;
*/

// Hash function used for lump names.
unsigned int W_LumpNameHash(const char *s)
{
    // This is the djb2 string hash function, modded to work on strings
    // that have a maximum length of 8.

    unsigned int result = 5381;
    unsigned int i;

    for (i=0; i < 8 && s[i] != '\0'; ++i)
    {
        result = ((result << 5) ^ result ) ^ toupper(s[i]);
    }

    return result;
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.

wad_file_t *W_AddFile (char *filename)
{
    wad_file_t *wad_file_data;
    wad_file_data->path = "doom.wad";
    wad_file_data->length = 4196366*2;
    return wad_file_data;

    /* X_HEEP COMMENT
    wadinfo_t header;
    lumpindex_t i;
    // wad_file_t *wad_file;
    wad_file_t *wad_file_data;
    int length;
    int startlump;
    filelump_t *fileinfo;
    filelump_t *filerover;
    // lumpinfo_t *filelumps;

    boolean do_wad_transfer = false;

    X_ReadButtons();
    I_Sleep(1);
    X_ReadButtons();

    if (X_ButtonState(0)) {
        do_wad_transfer = true;
    }

    /* NRFD-EXCLUDE:
    // If the filename begins with a ~, it indicates that we should use the
    // reload hack.
    if (filename[0] == '~')
    {
        if (reloadname != NULL)
        {
            I_Error("Prefixing a WAD filename with '~' indicates that the "
                    "WAD should be reloaded\n"
                    "on each level restart, for use by level authors for "
                    "rapid development. You\n"
                    "can only reload one WAD file, and it must be the last "
                    "file in the -file list.");
        }

        reloadname = strdup(filename);
        reloadlump = numlumps;
        ++filename;
    }
    */
   /*X_HEEP COMMENT

    if (numlumps != 0) {
        I_Error("Only one wad file supported\n");
    }

    PRINTF("W_AddFile: Reading %s\n", filename);
    // Open the file and add to directory
    // wad_file = W_OpenFile(filename);
    if (!no_sdcard) {
        wad_file = N_fs_open(filename);
        if (wad_file == NULL)
        {
            PRINTF (" couldn't open %s\n", filename);
            return NULL;
        }
    }
    wad_file_data = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);

    if (strcasecmp(filename+strlen(filename)-3 , "wad" ) )
    {
        I_Error("NRFD-TODO: W_AddFile\n");
        /*
        // single lump file

        // fraggle: Swap the filepos and size here.  The WAD directory
        // parsing code expects a little-endian directory, so will swap
        // them back.  Effectively we're constructing a "fake WAD directory"
        // here, as it would appear on disk.

        fileinfo = Z_N_Malloc(sizeof(filelump_t), PU_STATIC, 0);
        fileinfo->filepos = LONG(0);
        fileinfo->size = LONG(wad_file->length);

        // Name the lump after the base of the filename (without the
        // extension).

        M_ExtractFileBase (filename, fileinfo->name);
        numfilelumps = 1;
        */
       /*X-HEEP COMMENT
    }
    else
    {
        // Copy entire WAD file to Flash memory

        long file_size = 4196366;
        if (!no_sdcard) {
            file_size = N_fs_size(wad_file);
        }
        PRINTF("File size: %ld\n", file_size);

        int num_blocks = (file_size + N_QSPI_BLOCK_SIZE - 1) / N_QSPI_BLOCK_SIZE;
        N_qspi_reserve_blocks(num_blocks);
        if (!no_sdcard) {
            uint8_t *block_data = N_malloc(N_QSPI_BLOCK_SIZE);
            int block_loc = 0;
            uint8_t *qspi_data = N_qspi_data_pointer(0);
            boolean data_mismatch = do_wad_transfer;

            /*
            for (i = 0; i<num_blocks; i++) {
                PRINTF("Verifying block %d of %d\n", i, num_blocks);
                int block_next = block_loc + N_QSPI_BLOCK_SIZE;
                int block_size = block_next > file_size ? (file_size%N_QSPI_BLOCK_SIZE) : N_QSPI_BLOCK_SIZE;
                PRINTF("N_fs_read\n");
                N_fs_read(wad_file, block_loc, block_data, block_size);
                PRINTF("Comparing...\n");
                for (int j = 0; j<block_size; j++) {
                    if (block_data[j] != qspi_data[block_loc+j]) {
                        data_mismatch = true;
                        PRINTF("Found mismatch in byte %d\n", block_loc+j);
                        break;
                    }
                }
                if (data_mismatch) break;
                block_loc = block_next;
            }
            */
           /*X-HEEP COMMENT
            
            if (data_mismatch) {
                PRINTF("Uploading WAD data to QSPI flash memory..");
                block_loc = 0;
                for (i = 0; i<num_blocks; i++) {
                    PRINTF("Copying block %d of %d\n", i, num_blocks);
                    int block_next = block_loc + N_QSPI_BLOCK_SIZE;
                    int block_size = block_next > file_size ? (file_size%N_QSPI_BLOCK_SIZE) : N_QSPI_BLOCK_SIZE;
                    PRINTF("N_fs_read\n");
                    N_fs_read(wad_file, block_loc, block_data, block_size);
                    PRINTF("N_qspi_write_block\n");
                    N_qspi_write_block(block_loc, block_data, block_size);
                    block_loc = block_next;
                }
            }
            N_free(block_data);
        }
        

        wadinfo_t *header_ptr = N_qspi_data_pointer(0);

        // WAD file
        // W_Read(wad_file, 0, &header, sizeof(header));

        if (strncmp(header_ptr->identification,"IWAD",4))
        {
            // Homebrew levels?
            if (strncmp(header_ptr->identification,"PWAD",4))
            {
                // W_CloseFile(wad_file);
                I_Error ("Wad file %s doesn't have IWAD "
                    "or PWAD id\n", filename);
            }

            // ???modifiedgame = true;
        }

        header_ptr->numlumps = LONG(header_ptr->numlumps);

         // Vanilla Doom doesn't like WADs with more than 4046 lumps
         // https://www.doomworld.com/vb/post/1010985
         if (!strncmp(header_ptr->identification,"PWAD",4) && header_ptr->numlumps > 4046)
         {
                 // W_CloseFile(wad_file);
                 I_Error ("Error: Vanilla limit for lumps in a WAD is 4046, "
                          "PWAD %s has %d", filename, header_ptr->numlumps);
         }

        header_ptr->infotableofs = LONG(header_ptr->infotableofs);
        // length = header_ptr->numlumps*sizeof(filelump_t);
        // fileinfo = Z_Malloc(length, PU_STATIC, 0);

        PRINTF("WAD header_ptr\n");
        PRINTF("ID: %.4s\n", header_ptr->identification);
        PRINTF("Num lumps: %d\n", header_ptr->numlumps);
        PRINTF("Info table: %d\n", header_ptr->infotableofs);

        
        if (numlumps != 0) { 
            I_Error("NRFD-TODO: Multiple WADs not supported yet\n");
        }

        if ((numlumps+header_ptr->numlumps) > MAX_NUMLUMPS) {
            I_Error("W_AddFile: MAX_NUMLUMPS reached\n");
        } 

        first_lump_pos = header_ptr->infotableofs;
        filelumps = (filelump_t*)N_qspi_data_pointer(first_lump_pos);
        numlumps +=  header_ptr->numlumps;
        /*
        for (i = 0; i < header_ptr->numlumps; i++)
        {
            filelump_t filelump;
            int lump_pos = header_ptr->infotableofs+sizeof(filelump_t)*i;
            // W_Read(wad_file, lump_pos, &filelump, sizeof(filelump_t));
            filelump = *((filelump_t*)(N_qspi_data_pointer(lump_pos)));
            lumpinfo_t *lump_p = &lumpinfo[numlumps];
            // lump_p->wad_file = wad_file; // NRFD-TODO: Support multiple files
            // lump_p->position = LONG(filelump.filepos);
            unsigned int lump_filepos = LONG(filelump.filepos);
            lump_p->size = LONG(filelump.size);
            // lump_p->cache = NULL;
            lump_p->cache = N_qspi_data_pointer(lump_filepos);
            strncpy(lump_p->name, filelump.name, 8);

            // PRINTF("Lump %.8s: num: %d size: %d location: %X\n", lump_p->name, numlumps, lump_p->size, (unsigned int)lump_p->cache);

            if (0) //!strncasecmp(filelump.name, "ENDOOM", 8))
            {
                PRINTF("Found ENDOOM at %X\n", lump_filepos);
                char *endoom = lump_p->cache;
                PRINTF("%X\n", (unsigned int)(endoom));
                for (int i=0; i<80*4; i++) {
                    char c = endoom[i];
                    if (i%2==1) continue;
                    if ((i/2)%80==0)
                        PRINTF("\n");
                    if (c < 32 || c > 127)
                        PRINTF("X");
                    else 
                        PRINTF("%c", c);
                }
                PRINTF("\n");
            }

            numlumps += 1;
        }
        */
/* X-HEEP COMMENT
        wad_file_data->path = filename;
        wad_file_data->length = file_size;
    }

    if (lumphash != NULL)
    {
        Z_Free(lumphash);
        lumphash = NULL;
    }

    /* NRFD-EXCLUDE
    // If this is the reload file, we need to save some details about the
    // file so that we can close it later on when we do a reload.
    if (reloadname)
    {
        reloadhandle = wad_file;
        reloadlumps = filelumps;
    }
    */

    return wad_file_data;
}


void *W_LumpDataPointer(lumpindex_t lump)
{
    //return N_qspi_data_pointer(LONG(filelumps[lump].filepos));
    return WAD_START_ADDRESS + filelumps[lump].filepos;
}

//
// W_NumLumps
//
int W_NumLumps (void)
{
    return numlumps;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//

lumpindex_t W_CheckNumForName(const char* name)
{
    lumpindex_t i;

    // Do we have a hash table yet?

    /* NRFD-TODO: lump hash table */
    /*
    if (lumphash != NULL)
    {
        int hash;

        // We do! Excellent.

        hash = W_LumpNameHash(name) % numlumps;

        for (i = lumphash[hash]; i != -1; i = lumpinfo[i]->next)
        {
            if (!strncasecmp(lumpinfo[i]->name, name, 8))
            {
                return i;
            }
        }
    }
    else*/
    {
        // We don't have a hash table generate yet. Linear search :-(
        //
        // scan backwards so patch lump files take precedence

        for (i = numlumps - 1; i >= 0; --i)
        {
            if (!strncasecmp(filelumps[i].name, name, 8))
            // if (!strncasecmp(lumpinfo[i].name, name, 8))
            {
                return i;
            }
        }
    }

    // TFB. Not found.

    return -1;
}

//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
lumpindex_t W_GetNumForName(const char* name)
{
    lumpindex_t i;

    i = W_CheckNumForName (name);

    if (i < 0)
    {
        I_Error ("W_GetNumForName: %s not found!", name);
    }
 
    return i;
}

char *W_LumpName(lumpindex_t lump)
{
    return filelumps[lump].name;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(lumpindex_t lump)
{
    if (lump >= numlumps)
    {
        I_Error ("W_LumpLength: %i >= numlumps", lump);
    }

    // return lumpinfo[lump].size;
    return LONG(filelumps[lump].size);
}



//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(lumpindex_t lump, void *dest)
{
    if (lump >= numlumps)
    {
        I_Error ("W_ReadLump: %i >= numlumps", lump);
    }

    X_spi_read(WAD_START_ADDRESS + lumpinfo[lump].position, dest, lumpinfo[lump].size);


    // lumpinfo_t *l;
    // l = &lumpinfo[lump];
    // PRINTF("W_ReadLump(dummy): %.8s\n", l->name);

    // V_BeginRead(l->size);
    /*X_HEEP Comment
    filelump_t *filelump = &filelumps[lump];
    void *ptr = N_qspi_data_pointer(LONG(filelump->filepos));
    memcpy(dest, ptr, LONG(filelump->size));
    X-HEEP Finished comment*/

    // memcpy(dest, l->cache, l->size);

    /* NRFD-EXCLUDE

    // int c;
    // PRINTF("Read lump at %d with size %d to %X\n", l->position, l->size, (unsigned int)(dest));
    // c = W_Read(l->wad_file, l->position, dest, l->size);
    // c = W_Read(wad_file, l->position, dest, l->size);

    if (c < l->size)
    {
        I_Error("W_ReadLump: only read %i of %i on lump %i",
                c, l->size, lump);
    }*/
}




//
// W_CacheLumpNum
//
// Load a lump into memory and return a pointer to a buffer containing
// the lump data.
//
// 'tag' is the type of zone memory buffer to allocate for the lump
// (usually PU_STATIC or PU_CACHE).  If the lump is loaded as 
// PU_STATIC, it should be released back using W_ReleaseLumpNum
// when no longer needed (do not use Z_ChangeTag).
//

void *W_CacheLumpNum(lumpindex_t lumpnum, int tag)
{

    byte *result;
    // lumpinfo_t *lump;

    if ((unsigned)lumpnum >= numlumps)
    {
        I_Error ("W_CacheLumpNum: %i >= numlumps", lumpnum);
    }

    // lump = &lumpinfo[lumpnum];

    // Get the pointer to return.  If the lump is in a Memory-mapped
    // file, we can just return a pointer to within the memory-mapped
    // region.  If the lump is in an ordinary file, we may already
    // have it cached; otherwise, load it into memory.

    // result = lump->cache;
    /*
    for (int i=0;i<debugLumpCount;i++) {
        if (debugLumpNums[i]==lumpnum) {
            byte *cache = debugLumpCache[i];
            byte *qspi_data = W_LumpDataPointer(lumpnum);
            for (int j=0;j<filelumps[lumpnum].size;j++) {
                if (cache[j] != qspi_data[j]) {
                    PRINTF("X");
                }
            }
            return cache;
        }
    }
    */
    result = W_LumpDataPointer(lumpnum);
    // N_ldbg("W_CacheLumpNum: %.8s\n", lump->name);

    /* NRFD-EXCLUDE:
    if (lump->wad_file->mapped != NULL)
    {
        // Memory mapped file, return from the mmapped region.

        result = lump->wad_file->mapped + lump->position;
    }
    else if (lump->cache != NULL)
    {
        // Already cached, so just switch the zone tag.

        result = lump->cache;
        Z_ChangeTag(lump->cache, tag);
    }
    else
    {
        // Not yet loaded, so load it now
        lump->cache = Z_Malloc(W_LumpLength(lumpnum), tag, &lump->cache);
        W_ReadLump (lumpnum, lump->cache);
        result = lump->cache;
    }
    */
   
    return result;
}



//
// W_CacheLumpName
//
void *W_CacheLumpName(char *name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

// 
// Release a lump back to the cache, so that it can be reused later 
// without having to read from disk again, or alternatively, discarded
// if we run out of memory.
//
// Back in Vanilla Doom, this was just done using Z_ChangeTag 
// directly, but now that we have WAD mmap, things are a bit more
// complicated ...
//

void W_ReleaseLumpNum(lumpindex_t lumpnum)
{

    if ((unsigned)lumpnum >= numlumps)
    {
        I_Error ("W_ReleaseLumpNum: %i >= numlumps", lumpnum);
    }

    /* NRFD-EXCLUDE

    lumpinfo_t *lump;
    lump = &lumpinfo[lumpnum];

    if (lump->wad_file->mapped != NULL)
    {
        // Memory-mapped file, so nothing needs to be done here.
    }
    else
    {
        Z_ChangeTag(lump->cache, PU_CACHE);
    }
    */
}

void W_ReleaseLumpName(char *name)
{
    W_ReleaseLumpNum(W_GetNumForName(name));
}

#if 0

//
// W_Profile
//
int             info[2500][10];
int             profilecount;

void W_Profile (void)
{
    int         i;
    memblock_t* block;
    void*       ptr;
    char        ch;
    FILE*       f;
    int         j;
    char        name[9];
        
        
    for (i=0 ; i<numlumps ; i++)
    {   
        ptr = lumpinfo[i].cache;
        if (!ptr)
        {
            ch = ' ';
            continue;
        }
        else
        {
            block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
            if (block->tag < PU_PURGELEVEL)
                ch = 'S';
            else
                ch = 'P';
        }
        info[i][profilecount] = ch;
    }
    profilecount++;
        
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
        memcpy (name,lumpinfo[i].name,8);

        for (j=0 ; j<8 ; j++)
            if (!name[j])
                break;

        for ( ; j<8 ; j++)
            name[j] = ' ';

        fprintf (f,"%s ",name);

        for (j=0 ; j<profilecount ; j++)
            fprintf (f,"    %c",info[i][j]);

        fprintf (f,"\n");
    }
    fclose (f);
}


#endif

// Generate a hash table for fast lookups

void W_GenerateHashTable(void)
{
    lumpindex_t i;
    PRINTF("NRDF-TODO? W_GenerateHashTable\n");

    /*
    // Free the old hash table, if there is one:
    if (lumphash != NULL)
    {
        Z_Free(lumphash);
    }

    // Generate hash table
    if (numlumps > 0)
    {
        lumphash = Z_Malloc(sizeof(lumpindex_t) * numlumps, PU_STATIC, NULL);

        for (i = 0; i < numlumps; ++i)
        {
            lumphash[i] = -1;
        }

        for (i = 0; i < numlumps; ++i)
        {
            unsigned int hash;

            hash = W_LumpNameHash(lumpinfo[i]->name) % numlumps;

            // Hook into the hash table

            lumpinfo[i]->next = lumphash[hash];
            lumphash[hash] = i;
        }
    }
    */

    // All done!
}

// The Doom reload hack. The idea here is that if you give a WAD file to -file
// prefixed with the ~ hack, that WAD file will be reloaded each time a new
// level is loaded. This lets you use a level editor in parallel and make
// incremental changes to the level you're working on without having to restart
// the game after every change.
// But: the reload feature is a fragile hack...
void W_Reload(void)
{
    /* NRFD-EXCLUDE
    char *filename;
    lumpindex_t i;

    if (reloadname == NULL)
    {
        return;
    }

    // We must free any lumps being cached from the PWAD we're about to reload:
    for (i = reloadlump; i < numlumps; ++i)
    {
        if (lumpinfo[i]->cache != NULL)
        {
            Z_Free(lumpinfo[i]->cache);
        }
    }

    // Reset numlumps to remove the reload WAD file:
    numlumps = reloadlump;

    // Now reload the WAD file.
    filename = reloadname;

    W_CloseFile(reloadhandle);
    free(reloadlumps);

    reloadname = NULL;
    reloadlump = -1;
    reloadhandle = NULL;
    W_AddFile(filename);
    free(filename);

    // The WAD directory has changed, so we have to regenerate the
    // fast lookup hashtable:
    W_GenerateHashTable();
    */
}

/*
void W_DebugLump(int lump)
{
    return;
    if (lump != 561) return;

    filelump_t *filelump = &filelumps[lump];

    PRINTF("W_DebugLump: %d size: %d\n", lump, filelump->size);

    byte *cache = N_malloc(filelump->size);
    
    // N_fs_read(wad_file, filelump->filepos, cache, filelump->size);
    byte *qspi_data = W_LumpDataPointer(lump);
    for (int i=0;i<filelump->size;i++) {
        cache[i] = qspi_data[i];
    }

    debugLumpNums[debugLumpCount] = lump;
    debugLumpCache[debugLumpCount] = cache;
    debugLumpCount++;
}
*/