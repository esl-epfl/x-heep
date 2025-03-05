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
//      Preparation of data for rendering,
//      generation of lookups, caching, retrieval by name.
//

#include <stdio.h>
#include <string.h>
#ifndef SEGGER
#include <strings.h>
#endif

#include "deh_doomTop.h"
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"


#include "w_wad.h"

#include "doomdef.h"
#include "m_misc.h"
#include "r_local.h"
#include "p_local.h"

#include "doomstat.h"
#include "r_sky.h"

#include "r_data.h"

//#include "n_qspi.h"

#include "x_buttons.h"
#include "x_spi.h"

// NRFD-TODO: Check values for all supported games
#define MAX_TEXTURES 125
#define MAX_TEXTURE_PATCHES 350
#define MAX_FLATS 60

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
// 



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef PACKED_STRUCT (
{
    short       originx;
    short       originy;
    short       patch;
    short       stepdir;
    short       colormap;
}) mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef PACKED_STRUCT (
{
    char            name[8];
    int             masked;      
    short           width;
    short           height;
    int             obsolete;
    short           patchcount;
    mappatch_t      patches[1];
}) maptexture_t;


// A single patch from a texture definition,
//  basically a rectangular area within
//  the texture rectangle.
typedef struct  __attribute__((packed))
{
    // Block origin (allways UL),
    // which has allready accounted
    // for the internal origin of the patch.
    short       originx;    
    short       originy;
    short       patch;
} texpatch_t;


// A maptexturedef_t describes a rectangular texture,
//  which is composed of one or more mappatch_t structures
//  that arrange graphic patches.

typedef struct  __attribute__((packed)) texture_s texture_t;

struct  __attribute__((packed)) texture_s
{
    maptexture_t *wad_texture;
    // byte** columns;

    // Keep name for switch changing, etc.
    /*
    char        name[8];
    short        width;
    short        height;
    */

    /* NRFD-EXCLUDE: hash table
    // Index in textures list
    int         index;
    // Next in hash table chain
    texture_t  *next; 
    */

    // unsigned short        compositesize;
    // short*                columnlump;
    // unsigned short*       columnofs;
    byte*                 composite;
    
    // All the patches[patchcount]
    //  are drawn back to front into the cached texture.
    // byte       patchcount;
    // texpatch_t  *patches;
};



int                   firstflat;
int                   lastflat;
int                   numflats;

int                   firstpatch;
int                   lastpatch;
int                   numpatches;

int                   firstspritelump;
int                   lastspritelump;
int                   numspritelumps;

int                   numtextures;
texture_t             textures[MAX_TEXTURES];
char*                 textures_names;
texpatch_t*           texture_patches;
// texture_t**           textures_hashtable; // NRFD-EXCLUDE hashtable

// int                   texturewidthmask[MAX_TEXTURES];
// needed for texture pegging
// fixed_t               textureheight[MAX_TEXTURES];
// unsigned short        texturecompositesize[MAX_TEXTURES];
// short*                texturecolumnlump[MAX_TEXTURES];
// unsigned short*       texturecolumnofs[MAX_TEXTURES];
// byte*                 texturecomposite[MAX_TEXTURES];

// for global animation
short*                  flattranslation;
short*                  texturetranslation;

// needed for pre rendering
// fixed_t*              spritewidth;
// fixed_t*              spriteoffset;
// fixed_t*              spritetopoffset;

lighttable_t*         colormaps;


//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//

// void R_SetTextureHeight(texture_t *tex, int height)
// {
//     if (height >= 256*4) {
//         I_Error("R_SetTextureHeight");
//     }
//     tex->height = height/4;
// }

// void R_SetTextureWidth(texture_t *tex, int width)
// {
//     if (width >= 256*4) {
//         I_Error("R_SetTextureWidth");
//     }
//     tex->width = width/4;
// }

int R_TextureWidth(texture_t *tex)
{
    return SHORT(tex->wad_texture->width);
    // return tex->width*4;
}

int R_TextureHeight(texture_t *tex)
{
    return SHORT(tex->wad_texture->height);
    // return tex->height*4;
}

//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
void
R_DrawColumnInCache
( column_t*     patch,
  byte*         cache,
  int           originy,
  int           cacheheight )
{
    int   count;
    int   position;
    byte* source;

    while (patch->topdelta != 0xff)
    {
        source = (byte *)patch + 3;
        count = patch->length;
        position = originy + patch->topdelta;

        if (position < 0)
        {
            count += position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
            memcpy (cache + position, source, count);
            
        patch = (column_t *)(  (byte *)patch + patch->length + 4); 
    }
}



//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
void R_GenerateComposite (int texnum)
{
    PRINTF("NRFD-TODO: R_GenerateComposite %d\n", texnum);
    
    /*
    byte*           block;
    texture_t*      texture;
    texpatch_t*     patch;
    patch_t*        realpatch;
    int             x;
    int             x1;
    int             x2;
    int             i;
    column_t*       patchcol;
    short*          collump;
    unsigned short* colofs;
        
    texture = &textures[texnum];

    block = Z_Malloc (texture->compositesize, PU_STATIC, texture->composite);    

    collump = texture->columnlump;
    colofs = texture->columnofs;
    
    // Composite the columns together.
    patch = texture->patches;
            
    for (i=0 , patch = texture->patches;
         i<texture->patchcount;
         i++, patch++)
    {
        realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);

        if (x1<0)
            x = 0;
        else
            x = x1;
        
        if (x2 > texture->width)
            x2 = texture->width;

        for ( ; x<x2 ; x++)
        {
            // Column does not have multiple patches?
            if (collump[x] >= 0)
            continue;
            
            patchcol = (column_t *)((byte *)realpatch
                        + LONG(realpatch->columnofs[x-x1]));
            R_DrawColumnInCache (patchcol,
                     block + colofs[x],
                     patch->originy,
                     texture->height);
        }
                                
    }
    */

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    // Z_ChangeTag (block, PU_CACHE); // NRFD-TODO?
}

boolean generate_to_flash;
byte *generate_buffer;
size_t store_loc;

void R_GenerateInit(int texture_storage_size)
{
    X_ReadButtons();
    I_Sleep(1);
    X_ReadButtons();
    generate_to_flash = X_ButtonState(1);

    generate_buffer = (byte*)I_VideoBuffers;
    //store_loc = N_qspi_alloc_block();
    PRINTF("R_GenerateInit: %d %d\n", store_loc, generate_to_flash);

    /* X-HEEP COMMENT
    for (int ofs=0; ofs<texture_storage_size; ofs+=N_QSPI_BLOCK_SIZE) {
        if (generate_to_flash) {
            N_qspi_erase_block(store_loc+ofs);
        }
        N_qspi_alloc_block();
    } X-HEEP COMMENT END */
}




void R_GenerateComposite_N (int num, texture_t *texture, char *patch_names)
{

    // PRINTF("R_GenerateComposite_N: %d\n", num);
    
    byte*           block;
    int             i;

    int width = R_TextureWidth(texture);
    int height = R_TextureHeight(texture);
    size_t texture_size = width*height;
    maptexture_t *mtex = texture->wad_texture;

    size_t texture_loc = store_loc;
    store_loc += texture_size;

    texture->composite = WAD_START_ADDRESS + texture_loc;//N_qspi_data_pointer(texture_loc);

    for (int i=0; i<texture_size; i++) {
        // NRFD-TODO: Verify that textures don't have 251 in them
        generate_buffer[i] = 251; // PINK, use as transparent is masked textures
    }

    int patchcount = SHORT(mtex->patchcount);
    for (i=0; i<patchcount; i++)
    {
        int          x;
        int          x1;
        int          x2;
        mappatch_t*  mpatch        = &mtex->patches[i];
        short        patch_num     = SHORT(mpatch->patch);
        int          originy       = SHORT(mpatch->originy);
        char*        patch_name    = patch_names + patch_num * 8;
        // int          patch_lump    = W_CheckNumForName(patch_name);
        patch_t*     realpatch     = W_CacheLumpName(patch_name, PU_CACHE); //W_CacheLumpNum (patch_lump, PU_CACHE);
        int          columnofs[256];

        // PRINTF("        %.8s(%d)\n", patch_name, patch_num);

        // int *patch_columnofs = realpatch->columnofs;
        // NOTE: Having some trouble with reliable reading of this data from QSPI
        memcpy(columnofs, realpatch->columnofs, 256*sizeof(int));

        x1 = SHORT(mpatch->originx);
        x2 = x1 + SHORT(realpatch->width);

        if (x1<0)
            x = 0;
        else
            x = x1;

        if (x2 > width)
            x2 = width;

        for ( ; x<x2 ; x++)
        {
            column_t*       firstcol;
            byte* dest_col = generate_buffer + (height*x);
            int col_num = x-x1;
            // int colofs = LONG(patch_columnofs[col_num]);
            if (col_num >= 256) {
                I_Error("R_GenerateComposite_N: col_num(%d) > 256\n", col_num);
            }
            int colofs = columnofs[col_num]; // LONG(...)
            firstcol = (column_t *)((byte *)realpatch + colofs);
            column_t* col_ptr = firstcol;

            while (col_ptr->topdelta != 0xff)
            {
                int col_length = col_ptr->length;
                byte* source = (byte *)col_ptr + 3;
                int count = col_length;
                int position = originy + col_ptr->topdelta;
                if (position < 0)
                {
                    count += position;
                    position = 0;
                }

                if (position + count > height)
                    count = height - position;


                if (count > 0) {
                    memcpy (dest_col + position, source, count);
                }
                col_ptr = (column_t *)((byte*)(col_ptr)  + col_length + 4);
            }
        }
    }

    if (generate_to_flash) {
        /* X-HEEP COMMENT
         * X-HEEP TODO: SPI WRITE
        N_qspi_write(texture_loc, generate_buffer, texture_size);
        */
    }
}


//
// R_GenerateLookup
//
void R_GenerateLookup (int texnum)
{
    PRINTF("NRFD-TODO: R_GenerateLookup\n");
     /*   
    
    texture_t*          texture;
    byte*           patchcount;   // patchcount[texture->width]
    texpatch_t*             patch;  
    patch_t*            realpatch;
    int                 x;
    int                 x1;
    int                 x2;
    int                 i;
    short*          collump;
    unsigned short*     colofs;
        
    texture = &textures[texnum];

    // Composited texture not created yet.
    texture->composite= 0;
    
    texture->compositesize = 0;
    collump = texture->columnlump;
    colofs = texture->columnofs;
    

    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.
    patchcount = (byte *) Z_Malloc(texture->width, PU_STATIC, &patchcount);
    memset (patchcount, 0, texture->width);
    patch = texture->patches;

    for (i=0 , patch = texture->patches;
         i<texture->patchcount;
         i++, patch++)
    {
        realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
        x1 = patch->originx;
        x2 = x1 + SHORT(realpatch->width);
        
        if (x1 < 0)
            x = 0;
        else
            x = x1;

        if (x2 > texture->width)
            x2 = texture->width;
        for ( ; x<x2 ; x++)
        {
            patchcount[x]++;
            collump[x] = patch->patch;
            colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
        }
    }
     
    for (x=0 ; x<texture->width ; x++)
    {
        if (!patchcount[x])
        {
            PRINTF ("R_GenerateLookup: column without a patch (%s)\n",
                R_TextureNameForNum(texnum));
            return;
        }
        // I_Error ("R_GenerateLookup: column without a patch");
        
        if (patchcount[x] > 1)
        {
            // Use the cached block.
            collump[x] = -1;    
            colofs[x] = texture->compositesize;
            
            if (texture->compositesize > 0x10000-texture->height)
            {
                I_Error ("R_GenerateLookup: texture %i is >64k", texnum);
            }
            
            texture->compositesize += texture->height;
        }
    }

    Z_Free(patchcount);
    */
}

//
// R_GetColumn
//
byte*
R_GetCachedColumn
( int           tex,
  int           col )
{
    // PRINTF("NRF-TODO: R_GetColumn\n");

    texture_t      *texture;
    short           height;
    short           width;

    texture = &textures[tex];
    col &= R_TextureWidthMask(tex);
    height = R_TextureHeight(texture);
    width = R_TextureWidth(texture);

    if (col < 0 || col >= width) {
        PRINTF("Assertion failed: %d < %d < %d\n", 0, col, width);
        I_Error("R_GetCachedColumn");
    }

    if (texture->composite != NULL) {
        return texture->composite + (col*height);
    }
    else {
        I_Error("R_GetCachedColumn: composite not generated");
    }
    return NULL;
}

byte*
R_GetColumn
( int           tex,
  int           col )
{
    // PRINTF("NRF-TODO: R_GetColumn\n");

    texture_t      *texture;
    short           height;
    short           width;

    texture = &textures[tex];
    col &= R_TextureWidthMask(tex);
    height = R_TextureHeight(texture);
    width = R_TextureWidth(texture);

    if (col < 0 || col >= width) {
        PRINTF("Assertion failed: %d < %d < %d\n", 0, col, width);
        I_Error("R_GetColumn");
    }
    I_Error("R_GetColumn");
    return NULL;
}



static void GenerateTextureHashTable(void)
{
    PRINTF("NRFD-EXCLUDE: GenerateTextureHashTable\n");
    /*
    texture_t **rover;
    int i;
    int key;

    textures_hashtable 
            = Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0);

    memset(textures_hashtable, 0, sizeof(texture_t *) * numtextures);

    // Add all textures to hash table

    for (i=0; i<numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = W_LumpNameHash(textures[i]->name) % numtextures;

        rover = &textures_hashtable[key];

        while (*rover != NULL)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = NULL;
        *rover = textures[i];
    }
    */
}


//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
void R_InitTextures (void)
{
    PRINTF("R_InitTextures\n");

    maptexture_t*       mtexture;
    texture_t*          texture;
    mappatch_t*             mpatch;
    texpatch_t*             patch;

    int                 i;
    int                 j;

    int*            maptex;
    int*            maptex2;
    int*            maptex1;
    
    char            name[9];
    char*           names;
    char*           patch_names;
    
    short*            patchlookup;
    
    int                 nummappatches;
    int                 offset;
    int                 maxoff;
    int                 maxoff1;
    int                 maxoff2;
    int                 numtextures1;
    int                 numtextures2;

    int*            directory;
    
    int                 temp1;
    int                 temp2;
    int                 temp3;

    int texture_patches_count = 0;

    
    // Load the patch names from pnames.lmp.
    name[8] = 0;
    names = W_CacheLumpName (DEH_String("PNAMES"), PU_STATIC);
    nummappatches = LONG ( *((int *)names) );
    patch_names = names + 4;

    // NRFD-TODO: Avoid malloc
    
    // patchlookup = Z_Malloc(nummappatches*sizeof(*patchlookup), PU_STATIC, NULL);

    // for (i = 0; i < nummappatches; i++)
    // {
    //     M_StringCopy(name, patch_names + i * 8, sizeof(name));
    //     patchlookup[i] = W_CheckNumForName(name);
    // }
    W_ReleaseLumpName(DEH_String("PNAMES"));

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName (DEH_String("TEXTURE1"), PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = maxoff1 = W_LumpLength (W_GetNumForName (DEH_String("TEXTURE1")));
    directory = maptex+1;
        
    if (W_CheckNumForName (DEH_String("TEXTURE2")) != -1)
    {
        maptex2 = W_CacheLumpName (DEH_String("TEXTURE2"), PU_STATIC);
        numtextures2 = LONG(*maptex2);
        maxoff2 = W_LumpLength (W_GetNumForName (DEH_String("TEXTURE2")));
    }
    else
    {
        maptex2 = NULL;
        numtextures2 = 0;
        maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    PRINTF("R_InitTextures: nummappatches = %d, numtextures = %d\n", nummappatches, numtextures);

    if (numtextures > MAX_TEXTURES) {
        I_Error("Too many textures in WAD\n");
    }

    
    //      Really complex printing shit...
    temp1 = W_GetNumForName (DEH_String("S_START"));  // P_???????
    temp2 = W_GetNumForName (DEH_String("S_END")) - 1;
    temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);

    // If stdout is a real console, use the classic vanilla "filling
    // up the box" effect, which uses backspace to "step back" inside
    // the box.  If stdout is a file, don't draw the box.

    /* NRFD-EXCLUDE
    if (I_ConsoleStdout())
    {
        PRINTF("[");
        for (i = 0; i < temp3 + 9; i++)
            PRINTF(" ");
        PRINTF("]");
        for (i = 0; i < temp3 + 10; i++)
            PRINTF("\b");
    }
    */

    // NRFD-TODO: Copy names to texture_names, use in name lookup functions, and possibly release names after level is loaded
        
    int texture_storage_size = 0;
    int texture_columns_size = 0;
    for (i=0 ; i<numtextures ; i++, directory++)
    {
        int patchcount;
        short width;
        short height;
        /* NRFD-EXCLUDE
        if (!(i&63))
            PRINTF (".");
        */

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex+1;
        }
            
        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error ("R_InitTextures: bad texture directory");
        
        mtexture = (maptexture_t *) ( (byte *)maptex + offset);

        texture = &textures[i];
        
        // memcpy (texture->name, mtexture->name, sizeof(texture->name));
        // R_SetTextureWidth(texture, SHORT(mtexture->width));
        // R_SetTextureHeight(texture, SHORT(mtexture->height));
        // texture->patchcount = SHORT(mtexture->patchcount);

        texture->wad_texture = mtexture;
        texture->composite = 0;
        patchcount = SHORT(mtexture->patchcount);
        width = SHORT(mtexture->width);
        height = SHORT(mtexture->height);

        texture_columns_size += R_TextureWidth(texture);
        // if (patchcount > 1) {
            texture_storage_size += R_TextureWidth(texture)*R_TextureHeight(texture);
        // }
        texture_patches_count += patchcount;

        PRINTF("Texture %d %.8s: width = %d, height = %d, patchcount = %d\n", 
            i, mtexture->name, width, height, patchcount);

        /* NRFD-NOTE: Moved to texture_t
        texturecolumnlump[i] = Z_Malloc (texture->width*sizeof(**texturecolumnlump), PU_STATIC,0);
        texturecolumnofs[i] = Z_Malloc (texture->width*sizeof(**texturecolumnofs), PU_STATIC,0);
        */
        /* NRFD-NOTE: Moved to R_TextureWidthMask
        j = 1;
        while (j*2 <= texture->width)
            j<<=1;

        texturewidthmask[i] = j-1;
        */

        // NRFD-NOTE: Moved to R_TextureHeight
        // textureheight[i] = texture->height<<FRACBITS;
    }

    PRINTF("Texture patches count: %d\n", texture_patches_count);
    PRINTF("Texture storage size: %d\n", texture_storage_size);


    // NRFD-NOTE: Do second pass after allocating texture patches 
    // texture_patches = Z_Malloc(sizeof(texpatch_t)*texture_patches_count, PU_STATIC, 0);

    // Start looking in first texture file.
    maptex = maptex1;
    maxoff = maxoff1;
    directory = maptex+1;

    patch = texture_patches;

    R_GenerateInit(texture_storage_size);

    for (i=0 ; i<numtextures ; i++, directory++) {

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex+1;
        }
        
        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error ("R_InitTextures: bad texture directory");
        
        mtexture = (maptexture_t *) ( (byte *)maptex + offset);

        texture = &textures[i];

        // mpatch = &mtexture->patches[0];

        R_GenerateComposite_N(i, texture, patch_names);

        /*
        texture->patches = patch;

        for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
        {
            short patch_num = SHORT(mpatch->patch);
            char *patch_name = patch_names + patch_num * 8;
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            // patch->patch = patchlookup[SHORT(mpatch->patch)];

            patch->patch = W_CheckNumForName(patch_name);
            if (patch->patch == -1)
            {
                I_Error ("R_InitTextures: Missing patch in texture %.8s", R_TextureNameForNum(i));
            }
        } 
        */
    }
    
    // Z_Free(patchlookup);
    W_ReleaseLumpName(DEH_String("PNAMES"));

    W_ReleaseLumpName(DEH_String("TEXTURE1"));
    if (maptex2)
        W_ReleaseLumpName(DEH_String("TEXTURE2"));
    
    // Precalculate whatever possible.      

    PRINTF("NRFD-TODO: R_InitTextures\n");
    /*
    for (i=0 ; i<numtextures ; i++)
        R_GenerateLookup (i);
    */
    
    // Create translation table for global animation.
    texturetranslation = Z_Malloc ((numtextures+1)*sizeof(*texturetranslation), PU_STATIC, 0);
    
    for (i=0 ; i<numtextures ; i++)
        texturetranslation[i] = i;

    GenerateTextureHashTable();
}



//
// R_InitFlats
//
void R_InitFlats (void)
{
    int             i;

    firstflat = W_GetNumForName (DEH_String("F_START")) + 1;
    lastflat = W_GetNumForName (DEH_String("F_END")) - 1;
    numflats = lastflat - firstflat + 1;

    // NRFD-TODO: Assert that numflats < 65k?
    PRINTF("R_InitFlats: firstflat = %d, numflats = %d\n", firstflat, numflats);
        
    // Create translation table for global animation.
    // NRFD-TODO? flattranslation could be bytes if we used offset from firstflat 
    flattranslation = Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);
    
    for (i=0 ; i<numflats ; i++)
        flattranslation[i] = i;
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps (void)
{
    int             i;
    patch_t     *patch;

    PRINTF("R_InitSpriteLumps\n");


    firstspritelump = W_GetNumForName (DEH_String("S_START")) + 1;
    lastspritelump = W_GetNumForName (DEH_String("S_END")) - 1;
    
    numspritelumps = lastspritelump - firstspritelump + 1;
    // spritewidth = Z_Malloc (numspritelumps*sizeof(*spritewidth), PU_STATIC, 0);
    // spriteoffset = Z_Malloc (numspritelumps*sizeof(*spriteoffset), PU_STATIC, 0);
    // spritetopoffset = Z_Malloc (numspritelumps*sizeof(*spritetopoffset), PU_STATIC, 0);
        
    for (i=0 ; i< numspritelumps ; i++)
    {
        // if (!(i&63))
        //     PRINTF (".");

        // patch = W_CacheLumpNum (firstspritelump+i, PU_CACHE);
        // spritewidth[i] = SHORT(patch->width)<<FRACBITS;
        // spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
        // spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
    }
}

// NRFD-TODO: Optimize these

fixed_t R_SpriteWidth(int num)
{
    patch_t     *patch;
    patch = W_CacheLumpNum (firstspritelump+num, PU_CACHE);
    return SHORT(patch->width)<<FRACBITS;
}
fixed_t R_SpriteOffset(int num)
{
    patch_t     *patch;
    patch = W_CacheLumpNum (firstspritelump+num, PU_CACHE);
    return SHORT(patch->leftoffset)<<FRACBITS;
}
fixed_t R_SpriteTopOffset(int num)
{
    patch_t     *patch;
    patch = W_CacheLumpNum (firstspritelump+num, PU_CACHE);
    return SHORT(patch->topoffset)<<FRACBITS;
}


//
// R_InitColormaps
//
void R_InitColormaps (void)
{
    int     lump;

    // Load in the light tables, 
    //  256 byte align tables.
    lump = W_GetNumForName(DEH_String("COLORMAP"));

    // NRFD-TODO?
    // colormaps = W_CacheLumpNum(lump, PU_STATIC);

    int length = W_LumpLength(lump);
    PRINTF("R_InitColormaps: length = %d\n", length);

    colormaps = Z_Malloc(length, PU_STATIC, 0);
    W_ReadLump(lump, colormaps);

}



//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    R_InitTextures ();
    R_InitFlats ();
    R_InitSpriteLumps ();
    R_InitColormaps ();
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName (const char* name)
{
    int             i;
    char        namet[9];

    i = W_CheckNumForName (name);

    if (i == -1)
    {
        namet[8] = 0;
        memcpy (namet, name,8);
        I_Error ("R_FlatNumForName: %s not found",namet);
    }
    // PRINTF("R_FlatNumForName: %d\n", i);
    return i - firstflat;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int     R_CheckTextureNumForName (const char *name)
{
    texture_t *texture;
    int key;
    // "NoTexture" marker.
    if (name[0] == '-')             
        return 0;

    // NRFD-NOTE: Slower but requires less memory. Potential for optimization
    // PRINTF("Checking %.7s\n", name);
    for (int i=0; i<numtextures; i++) {
        texture = &textures[i];
        // PRINTF("  %d: %.8s\n", i, texture->name);
        if (!strncasecmp (texture->wad_texture->name, name, 8) ) {
            return i;
        }
    }
    
    /* NRFD-EXCLUDE: hashtable
    key = W_LumpNameHash(name) % numtextures;

    texture=textures_hashtable[key]; 
    
    while (texture != NULL)
    {
        if (!strncasecmp (texture->name, name, 8) )
            return texture->index;

        texture = texture->next;
    }
    */
    
    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int     R_TextureNumForName (const char* name)
{
    int             i;
        
    i = R_CheckTextureNumForName (name);

    if (i==-1)
    {
        I_Error ("R_TextureNumForName: %s not found",
             name);
    }
    if (i>256) {
        I_Error("R_TextureNumForName: Only 256 textures supported");
    }
    return i;
}


char *R_TextureNameForNum(int num) {
    // PRINTF("NRFD-TODO: R_TextureNameForNum\n");
    // return "TODO";
    return textures[num].wad_texture->name;
}


//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int             flatmemory;
int             texturememory;
int             spritememory;

void R_PrecacheLevel (void)
{
    PRINTF("NRF-TODO: R_PrecacheLevel\n");
    /*
    char*           flatpresent;
    char*           texturepresent;
    char*           spritepresent;

    int                 i;
    int                 j;
    int                 k;
    int                 lump;
    
    texture_t*          texture;
    thinker_t*          th;
    spriteframe_t*      sf;

    if (demoplayback)
        return;
    
    // Precache flats.
    flatpresent = Z_Malloc(numflats, PU_STATIC, NULL);
    memset (flatpresent,0,numflats);        

    for (i=0 ; i<numsectors ; i++)
    {
        flatpresent[sectors[i].floorpic] = 1;
        flatpresent[sectors[i].ceilingpic] = 1;
    }
        
    flatmemory = 0;

    for (i=0 ; i<numflats ; i++)
    {
        if (flatpresent[i])
        {
            lump = firstflat + i;
            flatmemory += lumpinfo[lump].size;
            W_CacheLumpNum(lump, PU_CACHE);
        }
    }

    Z_Free(flatpresent);
    
    // Precache textures.
    texturepresent = Z_Malloc(numtextures, PU_STATIC, NULL);
    memset (texturepresent,0, numtextures);
        
    for (i=0 ; i<numsides ; i++)
    {
        texturepresent[sides[i].toptexture] = 1;
        texturepresent[sides[i].midtexture] = 1;
        texturepresent[sides[i].bottomtexture] = 1;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.
    texturepresent[skytexture] = 1;
        
    texturememory = 0;
    for (i=0 ; i<numtextures ; i++)
    {
        if (!texturepresent[i])
            continue;

        texture = &textures[i];
        
        for (j=0 ; j<texture->patchcount ; j++)
        {
            lump = texture->patches[j].patch;
            texturememory += lumpinfo[lump].size;
            W_CacheLumpNum(lump , PU_CACHE);
        }
    }

    Z_Free(texturepresent);
    
    // Precache sprites.
    spritepresent = Z_Malloc(numsprites, PU_STATIC, NULL);
    memset (spritepresent,0, numsprites);
        
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
            spritepresent[((mobj_t *)th)->sprite] = 1;
    }
        
    spritememory = 0;
    for (i=0 ; i<numsprites ; i++)
    {
        if (!spritepresent[i])
            continue;

        for (j=0 ; j<sprites[i].numframes ; j++)
        {
            sf = &sprites[i].spriteframes[j];
            for (k=0 ; k<8 ; k++)
            {
            lump = firstspritelump + sf->lump[k];
            spritememory += lumpinfo[lump].size;
            W_CacheLumpNum(lump , PU_CACHE);
            }
        }
    }

    Z_Free(spritepresent);
    */
}

// NRFD-TODO: Optimize/inline?
fixed_t R_TextureHeightFixed(int num)
{
    return R_TextureHeight(&textures[num]) << FRACBITS;
}

// NRFD-TODO: Optimize/inline?
int R_TextureWidthMask(int num)
{
    int texture_width = R_TextureWidth(&textures[num]);
    int j = 1;
    while (j*2 <= texture_width)
        j<<=1;
    return j-1;
}
