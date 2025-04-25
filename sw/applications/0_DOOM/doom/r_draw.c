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
//      The actual span/column drawing functions.
//      Here find the main potential for optimization,
//       e.g. inline assembly, different algorithms.
//




#include "doomdef.h"
#include "deh_doomTop.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

// Needs access to LFB (guess what).
#include "v_video.h"

// State.
#include "doomstat.h"

#include"x_spi.h"


// ?
#define MAXWIDTH                        320
#define MAXHEIGHT                       200
// NRFD-TODO?
// #define MAXWIDTH                        1120
// #define MAXHEIGHT                       832

// status bar height at bottom of screen
#define SBARHEIGHT              32

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//


byte*           viewimage; 
int             viewwidth;
int             scaledviewwidth;
int             viewheight;
int             viewwindowx;
int             viewwindowy; 
// pixel_t*        ylookup[MAXHEIGHT];
// int             columnofs[MAXWIDTH]; 

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//
// byte            translations[3][256];   
 
// Backing buffer containing the bezel drawn around the screen and 
// surrounding background.

static pixel_t *background_buffer = NULL;


//
// R_DrawColumn
// Source is the top of the column to scale.
//
boolean                 dc_debug = false;
lighttable_t            *dc_colormap; 
int                     dc_x; 
int                     dc_yl; 
int                     dc_yh; 
fixed_t                 dc_iscale; 
fixed_t                 dc_texturemid;

// first pixel in a column (possibly virtual) 
byte*                   dc_source;              

// just for profiling 
int                     dccount;

pixel_t *ylookup(int y)
{
    return I_VideoBuffer + (y+viewwindowy)*SCREENWIDTH; 
}

int columnofs(int x)
{
    return viewwindowx + x;
}

byte column_buffer[SCREENHEIGHT+3];

// Draw column with transparent pixels
void R_DrawTransColumn (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    fixed_t             frac;
    fixed_t             fracstep;        

    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
        return; 
                                 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT) {
        // NRFD-TODO: I_Error 
        PRINTF ("R_DrawColumn: %i to %i at %i\n", dc_yl, dc_yh, dc_x);
        return;
    }
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup(dc_yl) + columnofs(dc_x);  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    byte tempval;
    uint32_t temp_val_data;
    X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
    memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes  
    do 
    {
        // Re-map color indices from wall texture column
        //  using a lighting/special effects LUT.
        pixel_t val = tempval;
        if (val != 251) { // Use pink as transparent color
            *dest = dc_colormap[val];
        }
        
        dest += SCREENWIDTH; 
        frac += fracstep;
        X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
        memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 
        
    } while (count--); 
} 

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// 
void R_DrawColumn (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    fixed_t             frac;
    fixed_t             fracstep;        

    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
        return; 
                                 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT) {
        // NRFD-TODO: I_Error 
        PRINTF ("R_DrawColumn: %i to %i at %i\n", dc_yl, dc_yh, dc_x);
        return;
    }
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup(dc_yl) + columnofs(dc_x);  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    byte tempval;
    uint32_t temp_val_data;
    X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
    memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 
    do 
    {
        // Re-map color indices from wall texture column
        //  using a lighting/special effects LUT.
        *dest = dc_colormap[temp_val_data];
        
        dest += SCREENWIDTH; 
        frac += fracstep;
        X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
        memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 
        
    } while (count--); 
} 



// UNUSED.
// Loop unrolled.
#if 0
void R_DrawColumn (void) 
{ 
    int                 count; 
    byte*               source;
    byte*               dest;
    byte*               colormap;
    
    unsigned            frac;
    unsigned            fracstep;
    unsigned            fracstep2;
    unsigned            fracstep3;
    unsigned            fracstep4;       
 
    count = dc_yh - dc_yl + 1; 

    source = dc_source;
    colormap = dc_colormap;              
    dest = ylookup(dc_yl) + columnofs()c_x];  
         
    fracstep = dc_iscale<<9; 
    frac = (dc_texturemid + (dc_yl-centery)*dc_iscale)<<9; 
 
    fracstep2 = fracstep+fracstep;
    fracstep3 = fracstep2+fracstep;
    fracstep4 = fracstep3+fracstep;
        
    while (count >= 8) 
    { 
        dest[0] = colormap[source[frac>>25]]; 
        dest[SCREENWIDTH] = colormap[source[(frac+fracstep)>>25]]; 
        dest[SCREENWIDTH*2] = colormap[source[(frac+fracstep2)>>25]]; 
        dest[SCREENWIDTH*3] = colormap[source[(frac+fracstep3)>>25]];
        
        frac += fracstep4; 

        dest[SCREENWIDTH*4] = colormap[source[frac>>25]]; 
        dest[SCREENWIDTH*5] = colormap[source[(frac+fracstep)>>25]]; 
        dest[SCREENWIDTH*6] = colormap[source[(frac+fracstep2)>>25]]; 
        dest[SCREENWIDTH*7] = colormap[source[(frac+fracstep3)>>25]]; 

        frac += fracstep4; 
        dest += SCREENWIDTH*8; 
        count -= 8;
    } 
        
    while (count > 0)
    { 
        *dest = colormap[source[frac>>25]]; 
        dest += SCREENWIDTH; 
        frac += fracstep; 
        count--;
    } 
}
#endif


void R_DrawColumnLow (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    pixel_t*            dest2;
    fixed_t             frac;
    fixed_t             fracstep;        
    int                 x;
 
    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
        return; 
                                 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT)
    {
        
        I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
    //  dccount++; 
#endif 
    // Blocky mode, need to multiply by 2.
    x = dc_x << 1;
    
    dest = ylookup(dc_yl) + columnofs(x);
    dest2 = ylookup(dc_yl) + columnofs(x+1);
    
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    
    byte tempval;
    uint32_t temp_val_data;
    X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
    memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 
    do 
    {
        // Hack. Does not work corretly.
        *dest2 = *dest = dc_colormap[temp_val_data];
        dest += SCREENWIDTH;
        dest2 += SCREENWIDTH;
        frac += fracstep; 
        X_spi_read(dc_source + ((frac>>FRACBITS)&127), &temp_val_data, 1);  
        memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 

    } while (count--);
}


//
// Spectre/Invisibility.
//
#define FUZZTABLE               50 
#define FUZZOFF (SCREENWIDTH)


const int     fuzzoffset[FUZZTABLE] =
{
    FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF 
}; 

uint8_t     fuzzpos = 0; 


//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
void R_DrawFuzzColumn (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    fixed_t             frac;
    fixed_t             fracstep;        

    // Adjust borders. Low... 
    if (!dc_yl) 
        dc_yl = 1;

    // .. and high.
    if (dc_yh == viewheight-1) 
        dc_yh = viewheight - 2; 
                 
    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
        return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawFuzzColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif
    
    dest = ylookup(dc_yl) + columnofs(dc_x);

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Looks like an attempt at dithering,
    //  using the colormap #6 (of 0-31, a bit
    //  brighter than average).
    do 
    {
        // Lookup framebuffer, and retrieve
        //  a pixel that is either one column
        //  left or right of the current one.
        // Add index from colormap to index.
        *dest = colormaps[6*256+dest[fuzzoffset[fuzzpos]]]; 

        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE) 
            fuzzpos = 0;
        
        dest += SCREENWIDTH;

        frac += fracstep; 
    } while (count--); 
} 

// low detail mode version
 
void R_DrawFuzzColumnLow (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    pixel_t*            dest2;
    fixed_t             frac;
    fixed_t             fracstep;        
    int x;

    // Adjust borders. Low... 
    if (!dc_yl) 
        dc_yl = 1;

    // .. and high.
    if (dc_yh == viewheight-1) 
        dc_yh = viewheight - 2; 
                 
    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
        return; 

    // low detail mode, need to multiply by 2
    
    x = dc_x << 1;
    
#ifdef RANGECHECK 
    if ((unsigned)x >= SCREENWIDTH
        || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawFuzzColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif
    
    dest = ylookup(dc_yl) + columnofs(x);
    dest2 = ylookup(dc_yl) + columnofs(x+1);

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Looks like an attempt at dithering,
    //  using the colormap #6 (of 0-31, a bit
    //  brighter than average).
    do 
    {
        // Lookup framebuffer, and retrieve
        //  a pixel that is either one column
        //  left or right of the current one.
        // Add index from colormap to index.
        *dest = colormaps[6*256+dest[fuzzoffset[fuzzpos]]]; 
        *dest2 = colormaps[6*256+dest2[fuzzoffset[fuzzpos]]]; 

        // Clamp table lookup index.
        if (++fuzzpos == FUZZTABLE) 
            fuzzpos = 0;
        
        dest += SCREENWIDTH;
        dest2 += SCREENWIDTH;

        frac += fracstep; 
    } while (count--); 
} 
 
  
  
 

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//
byte*   dc_translation;
byte*   translationtables;
const byte   translationtables_const[256*3] = {
                      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, 
                     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 
                     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
                     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 
                     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 
                     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 
                     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
                     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
                    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
                    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
                    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
                    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
                    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
                    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 
                    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
                    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 
                      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, 
                     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 
                     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
                     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 
                     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 
                     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 
                     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
                     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 
                    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
                    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
                    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
                    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
                    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
                    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 
                    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
                    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 
                      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, 
                     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 
                     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
                     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 
                     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79, 
                     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 
                     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
                     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
                    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
                    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
                    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
                    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
                    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
                    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 
                    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
                    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
                    };

void R_DrawTranslatedColumn (void) 
{ 
    int                 count; 
    pixel_t*            dest;
    fixed_t             frac;
    fixed_t             fracstep;        
 
    count = dc_yh - dc_yl; 
    if (count <  0) 
        return; 
                                 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                  dc_yl, dc_yh, dc_x);
    }
    
#endif 


    dest = ylookup(dc_yl) + columnofs(dc_x); 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    byte tempval;
    uint32_t temp_val_data;
    X_spi_read(dc_source + (frac>>FRACBITS), &temp_val_data, 1);  
    memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 

    // Here we do an additional index re-mapping.
    do 
    {
        // Translation tables are used
        //  to map certain colorramps to other ones,
        //  used with PLAY sprites.
        // Thus the "green" ramp of the player 0 sprite
        //  is mapped to gray, red, black/indigo. 
        *dest = dc_colormap[dc_translation[temp_val_data]];
        dest += SCREENWIDTH;
        
        frac += fracstep; 
        X_spi_read(dc_source + (frac>>FRACBITS), &temp_val_data, 1);  
        memcpy(&tempval, &temp_val_data, sizeof(column_t));  // Copy only 1 bytes 
    } while (count--); 
} 

void R_DrawTranslatedColumnLow (void) 
{ 
    PRINTF("NRFD-TODO: R_DrawTranslatedColumnLow\n"); /*

    int                 count; 
    pixel_t*            dest;
    pixel_t*            dest2;
    fixed_t             frac;
    fixed_t             fracstep;        
    int                 x;
 
    count = dc_yh - dc_yl; 
    if (count < 0) 
        return; 

    // low detail, need to scale by 2
    x = dc_x << 1;
                                 
#ifdef RANGECHECK 
    if ((unsigned)x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                  dc_yl, dc_yh, x);
    }
    
#endif 


    dest = ylookup(dc_yl) + columnofs(x); 
    dest2 = ylookup(dc_yl) + columnofs(x+1); 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
        // Translation tables are used
        //  to map certain colorramps to other ones,
        //  used with PLAY sprites.
        // Thus the "green" ramp of the player 0 sprite
        //  is mapped to gray, red, black/indigo. 
        *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        *dest2 = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        dest += SCREENWIDTH;
        dest2 += SCREENWIDTH;
        
        frac += fracstep; 
    } while (count--); 
    */
} 



//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//
void R_InitTranslationTables (void)
{
    translationtables = (byte*)translationtables_const;
    int i;
    /* NRFD-EXCLUDED: moved to constant table
    translationtables = Z_Malloc (256*3, PU_STATIC, 0);
    
    // translate just the 16 green colors
    for (i=0 ; i<256 ; i++)
    {
        if (i >= 0x70 && i<= 0x7f)
        {
            // map green ramp to gray, brown, red
            translationtables[i] = 0x60 + (i&0xf);
            translationtables [i+256] = 0x40 + (i&0xf);
            translationtables [i+512] = 0x20 + (i&0xf);
        }
        else
        {
            // Keep all other colors as is.
            translationtables[i] = translationtables[i+256] 
                = translationtables[i+512] = i;
        }
    }
    */
    /*
    PRINTF("translationtables_const[256*3] = {\n");
    for (i=0 ; i<256*3 ; i++)
    {
        PRINTF("%3d, ", translationtables[i]);
        if (i%16==15) { PRINTF("\n"); }
    }
    PRINTF("}\n");
    */
}




//
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int                     ds_y; 
int                     ds_x1; 
int                     ds_x2;

lighttable_t*           ds_colormap; 

fixed_t                 ds_xfrac; 
fixed_t                 ds_yfrac; 
fixed_t                 ds_xstep; 
fixed_t                 ds_ystep;

// start of a 64*64 tile image 
byte*                   ds_source;      // X-HEEP comment : ds_source is an adress in flash it must be read using X_spi_read

// just for profiling
int                     dscount;


//
// Draws the actual span.
void R_DrawSpan (void) 
{ 
    unsigned int position, step;
    pixel_t *dest;
    int count;
    int spot;
    unsigned int xtemp, ytemp;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1
        || ds_x1<0
        || ds_x2>=SCREENWIDTH
        || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error( "R_DrawSpan: %i to %i at %i",
                 ds_x1,ds_x2,ds_y);
    }
//      dscount++;
#endif

    // Pack position and step variables into a single 32-bit integer,
    // with x in the top 16 bits and y in the bottom 16 bits.  For
    // each 16-bit part, the top 6 bits are the integer part and the
    // bottom 10 bits are the fractional part of the pixel position.

    position = ((ds_xfrac << 10) & 0xffff0000)
             | ((ds_yfrac >> 6)  & 0x0000ffff);
    step = ((ds_xstep << 10) & 0xffff0000)
         | ((ds_ystep >> 6)  & 0x0000ffff);

    dest = ylookup(ds_y) + columnofs(ds_x1);

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    uint32_t temp_data;
    X_spi_read(ds_source[spot], &temp_data, 1); 

    do
    {
        // Calculate current texture index in u,v.
        ytemp = (position >> 4) & 0x0fc0;
        xtemp = (position >> 26);
        spot = xtemp | ytemp;

        // Lookup pixel from flat texture tile,
        //  re-index using light/colormap.
        *dest++ = ds_colormap[((temp_data >> 0)  & 0xFF)];

        position += step;

    } while (count--);
}



// UNUSED.
// Loop unrolled by 4.
#if 0
void R_DrawSpan (void) 
{ 
    unsigned    position, step;

    byte*       source;
    byte*       colormap;
    pixel_t*    dest;
    
    unsigned    count;
    usingned    spot; 
    unsigned    value;
    unsigned    temp;
    unsigned    xtemp;
    unsigned    ytemp;
                
    position = ((ds_xfrac<<10)&0xffff0000) | ((ds_yfrac>>6)&0xffff);
    step = ((ds_xstep<<10)&0xffff0000) | ((ds_ystep>>6)&0xffff);
                
    source = ds_source;
    colormap = ds_colormap;
    dest = ylookup(ds_y) + columnofs()s_x1];     
    count = ds_x2 - ds_x1 + 1; 
        
    while (count >= 4) 
    { 
        ytemp = position>>4;
        ytemp = ytemp & 4032;
        xtemp = position>>26;
        spot = xtemp | ytemp;
        position += step;
        dest[0] = colormap[source[spot]]; 

        ytemp = position>>4;
        ytemp = ytemp & 4032;
        xtemp = position>>26;
        spot = xtemp | ytemp;
        position += step;
        dest[1] = colormap[source[spot]];
        
        ytemp = position>>4;
        ytemp = ytemp & 4032;
        xtemp = position>>26;
        spot = xtemp | ytemp;
        position += step;
        dest[2] = colormap[source[spot]];
        
        ytemp = position>>4;
        ytemp = ytemp & 4032;
        xtemp = position>>26;
        spot = xtemp | ytemp;
        position += step;
        dest[3] = colormap[source[spot]]; 
                
        count -= 4;
        dest += 4;
    } 
    while (count > 0) 
    { 
        ytemp = position>>4;
        ytemp = ytemp & 4032;
        xtemp = position>>26;
        spot = xtemp | ytemp;
        position += step;
        *dest++ = colormap[source[spot]]; 
        count--;
    } 
} 
#endif


//
// Again..
//
void R_DrawSpanLow (void)
{
    unsigned int position, step;
    unsigned int xtemp, ytemp;
    pixel_t *dest;
    int count;
    int spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1
        || ds_x1<0
        || ds_x2>=SCREENWIDTH
        || (unsigned)ds_y>SCREENHEIGHT)
    {
        I_Error( "R_DrawSpan: %i to %i at %i",
                 ds_x1,ds_x2,ds_y);
    }
//      dscount++; 
#endif

    position = ((ds_xfrac << 10) & 0xffff0000)
             | ((ds_yfrac >> 6)  & 0x0000ffff);
    step = ((ds_xstep << 10) & 0xffff0000)
         | ((ds_ystep >> 6)  & 0x0000ffff);

    count = (ds_x2 - ds_x1);

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    dest = ylookup(ds_y) + columnofs(ds_x1);
    
    uint32_t temp_data;
    X_spi_read(ds_source[spot], &temp_data, 1); 

    do
    {
        // Calculate current texture index in u,v.
        ytemp = (position >> 4) & 0x0fc0;
        xtemp = (position >> 26);
        spot = xtemp | ytemp;

        // Lowres/blocky mode does it twice,
        //  while scale is adjusted appropriately.
        *dest++ = ds_colormap[((temp_data >> 0)  & 0xFF)];
        *dest++ = ds_colormap[((temp_data >> 0)  & 0xFF)];

        position += step;

    } while (count--);
}

//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//
void
R_InitBuffer
( int           width,
  int           height ) 
{ 
    int         i; 

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (SCREENWIDTH-width) >> 1; 

    // Column offset. For windows.
    // NRFD-TODO: columnofs LUT?
    // for (i=0 ; i<width ; i++) 
    //     columnofs[i] = viewwindowx + i;

    // Samw with base row offset.
    if (width == SCREENWIDTH) 
        viewwindowy = 0; 
    else 
        viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1; 

    // Preclaculate all row offsets.

    // NRFD-TODO?
    // for (i=0 ; i<height ; i++) 
    //     ylookup[i] = I_VideoBuffer + (i+viewwindowy)*SCREENWIDTH; 
} 
 
 


//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen (void) 
{ 
    byte*       src; // X-HEEP comment : src is an adress in flash it must be read using X_spi_read
    pixel_t*    dest;
    int         x;
    int         y; 
    patch_t*    patch; // X-HEEP comment : patch is an adress in flash it must be read using X_spi_read

    // DOOM border patch.
    char       *name1 = DEH_String("FLOOR7_2");

    // DOOM II border patch.
    char *name2 = DEH_String("GRNROCK");

    char *name;

    // If we are running full screen, there is no need to do any of this,
    // and the background buffer can be freed if it was previously in use.

    if (scaledviewwidth == SCREENWIDTH)
    {
        if (background_buffer != NULL)
        {
            Z_Free(background_buffer);
            background_buffer = NULL;
        }

        return;
    }

    // Allocate the background buffer if necessary
        
    if (background_buffer == NULL)
    {
        background_buffer = Z_Malloc(SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT) * sizeof(*background_buffer),
                                     PU_STATIC, NULL);
    }

    if (gamemode == commercial)
        name = name2;
    else
        name = name1;
    
    src = W_CacheLumpName(name, PU_CACHE); 
    dest = background_buffer;
         
    for (y=0 ; y<SCREENHEIGHT-SBARHEIGHT ; y++) 
    { 
        for (x=0 ; x<SCREENWIDTH/64 ; x++) 
        { 
            X_spi_read(src+((y&63)<<6), dest, 64/4); 
            //memcpy (dest, src+((y&63)<<6), 64); 
            dest += 64; 
        } 

        if (SCREENWIDTH&63) 
        { 
            X_spi_read(src+((y&63)<<6), dest, (SCREENWIDTH&63)/4); //X-HEEP comment : for now SCREENWIDTH&63 = 0 if this value changes check that it is a multiple of 4 
            //memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63); 
            dest += (SCREENWIDTH&63); 
        } 
    } 
     
    // Draw screen and bezel; this is done to a separate screen buffer.

    V_UseBuffer(background_buffer);

    patch = W_CacheLumpName(DEH_String("brdr_t"),PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
        V_DrawPatch(viewwindowx+x, viewwindowy-8, patch);
    patch = W_CacheLumpName(DEH_String("brdr_b"),PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
        V_DrawPatch(viewwindowx+x, viewwindowy+viewheight, patch);
    patch = W_CacheLumpName(DEH_String("brdr_l"),PU_CACHE);

    for (y=0 ; y<viewheight ; y+=8)
        V_DrawPatch(viewwindowx-8, viewwindowy+y, patch);
    patch = W_CacheLumpName(DEH_String("brdr_r"),PU_CACHE);

    for (y=0 ; y<viewheight ; y+=8)
        V_DrawPatch(viewwindowx+scaledviewwidth, viewwindowy+y, patch);

    // Draw beveled edge. 
    V_DrawPatch(viewwindowx-8,
                viewwindowy-8,
                W_CacheLumpName(DEH_String("brdr_tl"),PU_CACHE));
    
    V_DrawPatch(viewwindowx+scaledviewwidth,
                viewwindowy-8,
                W_CacheLumpName(DEH_String("brdr_tr"),PU_CACHE));
    
    V_DrawPatch(viewwindowx-8,
                viewwindowy+viewheight,
                W_CacheLumpName(DEH_String("brdr_bl"),PU_CACHE));
    
    V_DrawPatch(viewwindowx+scaledviewwidth,
                viewwindowy+viewheight,
                W_CacheLumpName(DEH_String("brdr_br"),PU_CACHE));

    V_RestoreBuffer();
} 
 

//
// Copy a screen buffer.
//
void
R_VideoErase
( unsigned      ofs,
  int           count ) 
{ 
  // LFB copy.
  // This might not be a good idea if memcpy
  //  is not optiomal, e.g. byte by byte on
  //  a 32bit CPU, as GNU GCC/Linux libc did
  //  at one point.

    if (background_buffer != NULL)
    {
        memcpy(I_VideoBuffer + ofs, background_buffer + ofs, count * sizeof(*I_VideoBuffer));
    }
} 


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder (void) 
{ 
    int         top;
    int         side;
    int         ofs;
    int         i; 
 
    if (scaledviewwidth == SCREENWIDTH) 
        return; 
  
    top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2; 
    side = (SCREENWIDTH-scaledviewwidth)/2; 
 
    // copy top and one line of left side 
    R_VideoErase (0, top*SCREENWIDTH+side); 
 
    // copy one line of right side and bottom 
    ofs = (viewheight+top)*SCREENWIDTH-side; 
    R_VideoErase (ofs, top*SCREENWIDTH+side); 
 
    // copy sides using wraparound 
    ofs = top*SCREENWIDTH + SCREENWIDTH-side; 
    side <<= 1;
    
    for (i=1 ; i<viewheight ; i++) 
    { 
        R_VideoErase (ofs, side); 
        ofs += SCREENWIDTH; 
    } 

    // ? 
    V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT); 
} 
 
 
