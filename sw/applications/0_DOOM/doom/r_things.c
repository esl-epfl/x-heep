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
//      Refresh of things, i.e. objects represented by sprites.
//




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef SEGGER
#include <strings.h>
#endif

#include "deh_doomTop.h"
#include "doomdef.h"

#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

#include "doomstat.h"

#define MAX_SPRITEFRAMES 261
#define MAX_SPRITES 138

#define MINZ                            (FRACUNIT*4)
#define BASEYCENTER                     (SCREENHEIGHT/2)

//void R_DrawColumn (void);
//void R_DrawFuzzColumn (void);



typedef struct
{
    int         x1;
    int         x2;
        
    int         column;
    int         topclip;
    int         bottomclip;

} maskdraw_t;


vissprite_t     vissprites[MAXVISSPRITES];
vissprite_t*    vissprite_p;
int             newvissprite;

vissprite_t*    vsprsortedhead;


//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t         pspritescale;
fixed_t         pspriteiscale;

lighttable_t**  spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
const short           negonearray[SCREENWIDTH] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
const short           screenheightarray[SCREENWIDTH] = {
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168,
    168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168, 168
    };

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t     sprites[MAX_SPRITES];
int             numsprites;

spriteframe_t   spriteframes[MAX_SPRITEFRAMES];
spriteframe_t   *sprtemp;
int             maxframe;
int             spriteframe_count = 0;
char*           spritename;




//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void
R_InstallSpriteLump
( int           lump,
  unsigned      frame,
  unsigned      rotation,
  boolean       flipped )
{
    int         r;
        
    if (frame >= 29 || rotation > 8)
        I_Error("R_InstallSpriteLump: "
                "Bad frame characters in lump %i", lump);
        
    if ((int)frame > maxframe)
        maxframe = frame;
                
    if (rotation == 0)
    {
        // the lump should be used for all rotations
        if (sprtemp[frame].rotate == false)
            I_Error ("R_InitSprites: Sprite %s frame %c has "
                     "multip rot=0 lump", spritename, 'A'+frame);

        if (sprtemp[frame].rotate == true)
            I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
                     "and a rot=0 lump", spritename, 'A'+frame);
                        
        sprtemp[frame].rotate = false;
        sprtemp[frame].flip = 0;
        for (r=0 ; r<8 ; r++)
        {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip |= (byte)((flipped?1:0)<<r);
            // sprtemp[frame].flip[r] = (byte)flipped;
        }
        return;
    }
        
    // the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
        I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
                 "and a rot=0 lump", spritename, 'A'+frame);
                
    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;         
    if (sprtemp[frame].lump[rotation] != -1)
        I_Error ("R_InitSprites: Sprite %s : %c : %c "
                 "has two lumps mapped to it",
                 spritename, 'A'+frame, '1'+rotation);
                
    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip &= ~(1<<rotation);
    sprtemp[frame].flip |= (byte)(flipped<<rotation);
    // sprtemp[frame].flip[rotation] = (byte)flipped;
}




//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs () 
{ 
    char**      check;
    int         i;
    int         l;
    int         frame;
    int         rotation;
    int         start;
    int         end;
    int         patched;
                
    // count the number of sprite names
    numsprites = 0;
    check = (char**)sprnames;
    while (*check != NULL) {
        check++;
        numsprites++;
    }
        
    if (!numsprites)
        return;
    PRINTF("R_InitSpriteDefs");
    PRINTF("  numsprites = %d\n", numsprites);

    if (numsprites > MAX_SPRITES) {
        I_Error("R_InitSpriteDefs: %d > MAX_SPRITES", numsprites);
    }
    // sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);
        
    start = firstspritelump-1;
    end = lastspritelump+1;

    memset (spriteframes, -1, sizeof(spriteframes));

        
    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<numsprites ; i++)
    {
        spritename = DEH_String((char*)sprnames[i]);
        sprtemp = &spriteframes[spriteframe_count];

        // NRFD-TODO: Should we do something like this?
        // memset (sprtemp,-1, sizeof(sprtemp));
                
        maxframe = -1;
        
        // scan the lumps,
        //  filling in the frames for whatever is found
        for (l=start+1 ; l<end ; l++)
        {
            char *lumpName = W_LumpName(l);
            if (!strncasecmp(lumpName, spritename, 4))
            {
                frame = lumpName[4] - 'A';
                rotation = lumpName[5] - '0';

                if (modifiedgame)
                    patched = W_GetNumForName (lumpName);
                else
                    patched = l;

                // PRINTF("  Frame %d rot %d: %.8s - %d\n", frame, rotation, lumpName, patched);
                R_InstallSpriteLump (patched, frame, rotation, false);

                if (lumpName[6])
                {
                    frame = lumpName[6] - 'A';
                    rotation = lumpName[7] - '0';
                    // PRINTF("  Frame %d rot %d: %.8s - %d\n", frame, rotation, lumpName, l);
                    R_InstallSpriteLump (l, frame, rotation, true);
                }
            }
        }
        
        // check the frames that were found for completeness
        if (maxframe == -1)
        {
            sprites[i].numframes = 0;
            continue;
        }
                
        maxframe++;
        
        for (frame = 0 ; frame < maxframe ; frame++)
        {
            switch ((int)sprtemp[frame].rotate)
            {
              case -1:
                // no rotations were found for that frame at all
                I_Error ("R_InitSprites: No patches found "
                         "for %s frame %c", spritename, frame+'A');
                break;
                
              case 0:
                // only the first rotation is needed
                break;
                        
              case 1:
                // must have all 8 frames
                for (rotation=0 ; rotation<8 ; rotation++)
                    if (sprtemp[frame].lump[rotation] == -1)
                        I_Error ("R_InitSprites: Sprite %s frame %c "
                                 "is missing rotations",
                                 spritename, frame+'A');
                break;
            }
        }
        
        // allocate space for the frames present and copy sprtemp to it
        if (maxframe > 256) {
            I_Error("R_InitSpriteDefs: maxframe overflow");
        }

        spriteframe_count += maxframe;
        if (spriteframe_count > MAX_SPRITEFRAMES) {
            I_Error("R_InitSpriteDefs: %d > MAX_SPRITEFRAMES", spriteframe_count);
        }

        sprites[i].numframes = maxframe;
        sprites[i].spriteframes = sprtemp;
        //     Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        // memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }
    PRINTF("  spriteframe_count = %d\n", spriteframe_count);
}




//
// GAME FUNCTIONS
//


//
// R_InitSprites
// Called at program start.
//
void R_InitSprites ()
{
    /* NRFD-EXCLUDE
    int         i;
    for (i=0 ; i<SCREENWIDTH ; i++)
    {
        negonearray[i] = -1;
    }
    */
        
    R_InitSpriteDefs ();
}



//
// R_ClearSprites
// Called at frame start.
//
int vissprite_count = 0;

void R_ClearSprites (void)
{
    if (vissprite_count != 0) {
        // PRINTF("VSC: %d\n", vissprite_count);
    }
    vissprite_count = 0;
    vissprite_p = vissprites;
}


//
// R_NewVisSprite
//
vissprite_t     overflowsprite;

vissprite_t* R_NewVisSprite (void)
{
    if (vissprite_p == &vissprites[MAXVISSPRITES]) {
        // PRINTF("R_NewVisSprite: overflow %d\n", vissprite_count);
        vissprite_count++;
        return &overflowsprite;
    }
    vissprite_count++;
    vissprite_p++;
    return vissprite_p-1;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
short*          mfloorclip;
short*          mceilingclip;

fixed_t         spryscale;
fixed_t         sprtopscreen;

void R_DrawMaskedColumn (column_t* column)
{
    int         topscreen;
    int         bottomscreen;
    fixed_t     basetexturemid;
        
    basetexturemid = dc_texturemid;

    for ( ; column->topdelta != 0xff ; ) 
    {
        // calculate unclipped screen coordinates
        //  for post
        topscreen = sprtopscreen + spryscale*column->topdelta;
        bottomscreen = topscreen + spryscale*column->length;

        dc_yl = (topscreen+FRACUNIT-1)>>FRACBITS;
        dc_yh = (bottomscreen-1)>>FRACBITS;
                
        if (dc_yh >= mfloorclip[dc_x])
            dc_yh = mfloorclip[dc_x]-1;
        if (dc_yl <= mceilingclip[dc_x])
            dc_yl = mceilingclip[dc_x]+1;

        if (dc_yl <= dc_yh)
        {
            dc_source = (byte *)column + 3;
            dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);
            // dc_source = (byte *)column + 3 - column->topdelta;

            // Drawn by either R_DrawColumn
            //  or (SHADOW) R_DrawFuzzColumn.
            colfunc (); 
        }
        column = (column_t *)(  (byte *)column + column->length + 4);
    }
        
    dc_texturemid = basetexturemid;
}



//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void
R_DrawVisSprite
( vissprite_t*          vis,
  int                   x1,
  int                   x2 )
{
    // N_ldbg("R_DrawVisSprite\n");

    column_t*           column;
    int                 texturecolumn;
    fixed_t             frac;
    patch_t*            patch;
        
    int lumpnum = vis->patch+firstspritelump;

    if (lumpnum == 561) {
        dc_debug = true;
        // PRINTF("R_DrawVisSprite: Pistol\n");
    }

    patch = W_CacheLumpNum (lumpnum, PU_CACHE);

    dc_colormap = vis->colormap;
    
    if (!dc_colormap)
    {
        // NULL colormap = shadow draw
        colfunc = fuzzcolfunc;
    }
    /* NRFD_TODO: translation
    else if (vis->thing->flags & MF_TRANSLATION)
    {
        colfunc = transcolfunc;
        dc_translation = translationtables - 256 +
            ( (vis->thing->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
    }
    */

    dc_iscale = abs(vis->xiscale)>>detailshift;
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);
        
    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
        texturecolumn = frac>>FRACBITS;
#ifdef RANGECHECK
        if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
            I_Error ("R_DrawSpriteRange: bad texturecolumn");
#endif
        column = (column_t *) ((byte *)patch +
                               LONG(patch->columnofs[texturecolumn]));
        R_DrawMaskedColumn (column);
    }
    dc_debug = false;
    colfunc = basecolfunc;
}



//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite (mobj_t* thing)
{
    fixed_t             tr_x;
    fixed_t             tr_y;
    
    fixed_t             gxt;
    fixed_t             gyt;
    
    fixed_t             tx;
    fixed_t             tz;

    fixed_t             xscale;
    
    int                 x1;
    int                 x2;

    spritedef_t*        sprdef;
    spriteframe_t*      sprframe;
    int                 lump;
    
    unsigned            rot;
    boolean             flip;
    
    int                 index;

    vissprite_t*        vis;
    
    angle_t             ang;
    fixed_t             iscale;
    
    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;
        
    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);
    
    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
        return;
    
    xscale = FixedDiv(projection, tz);
        
    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
        return;
    
    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned int) thing->sprite >= (unsigned int) numsprites)
        I_Error ("R_ProjectSprite: invalid sprite number %i ",
                 thing->sprite);
#endif

    sprdef = &sprites[thing->sprite];

    int frame = thing->state->frame;

#ifdef RANGECHECK
    if ( (frame&FF_FRAMEMASK) >= sprdef->numframes )
        I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ",
                 thing->sprite, frame);
#endif

    sprframe = &sprdef->spriteframes[frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle (thing->x, thing->y);
        rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
        lump = sprframe->lump[rot];
        flip = R_SpriteGetFlip(sprframe, rot);
    }
    else
    {
        // use single rotation for all views
        lump = sprframe->lump[0];
        flip = R_SpriteGetFlip(sprframe, 0);
    }
    
    // calculate edges of the shape
    tx -= R_SpriteOffset(lump);   
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
        return;
    
    tx +=  R_SpriteWidth(lump);
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;
    
    // store information in a vissprite
    vis = R_NewVisSprite ();
    // vis->mobjflags = thing->flags; // NRFD-TODO?
    vis->scale = xscale<<detailshift;

    vis->thing = thing;
    /*
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = thing->z + R_SpriteTopOffset(lump);
    */
    fixed_t gzt = thing->z + R_SpriteTopOffset(lump);
    vis->texturemid = gzt - viewz;

    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;       
    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
        vis->startfrac = R_SpriteWidth(lump)-1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);
    vis->patch = lump;
    
    // get light level
    if (thing->flags & MF_SHADOW)
    {
        // shadow draw
        vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
        // fixed map
        vis->colormap = fixedcolormap;
    }
    else if (frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = colormaps;
    }
    
    else
    {
        // diminished light
        index = xscale>>(LIGHTSCALESHIFT-detailshift);

        if (index >= MAXLIGHTSCALE) 
            index = MAXLIGHTSCALE-1;

        vis->colormap = spritelights[index];
    }   
}




//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites (sector_t* sec)
{
    mobj_t*             thing;
    int                 lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;         

    // Well, now it will be done.
    sec->validcount = validcount;
        
    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (lightnum < 0)           
        spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight[LIGHTLEVELS-1];
    else
        spritelights = scalelight[lightnum];

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
        R_ProjectSprite (thing);
}


//
// R_DrawPSprite
//
void R_DrawPSprite (pspdef_t* psp)
{
    // PRINTF("R_DrawPSprite\n");
    fixed_t             tx;
    fixed_t             width;
    int                 x1;
    int                 x2;
    spritedef_t*        sprdef;
    spriteframe_t*      sprframe;
    int                 lump;
    boolean             flip;
    vissprite_t*        vis;
    vissprite_t         avis;
    
    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= (unsigned int) numsprites)
        I_Error ("R_ProjectSprite: invalid sprite number %i ",
                 psp->state->sprite);
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
        I_Error ("R_ProjectSprite: invalid sprite frame %i : %i ",
                 psp->state->sprite, psp->state->frame);
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];

    // PRINTF("Frame: %d\n", psp->state->frame);
    // PRINTF("Lump: %d %.8s\n", lump, lumpinfo[firstspritelump+lump].name);
    // patch_t     *patch;
    // patch = W_CacheLumpNum (lump, PU_CACHE);

    flip = R_SpriteGetFlip(sprframe, 0);
    
    // calculate edges of the shape
    tx = psp->sx-(SCREENWIDTH/2)*FRACUNIT;
        
    tx -= R_SpriteOffset(lump);   
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
        return;         

    width = R_SpriteWidth(lump);
    tx += width;
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
        return;
    
    // store information in a vissprite
    vis = &avis;
    // vis->mobjflags = 0; // NRFD-TODO?
    vis->texturemid = (BASEYCENTER<<FRACBITS)+FRACUNIT/2-(psp->sy-R_SpriteTopOffset(lump));
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;       
    vis->scale = pspritescale<<detailshift;
    
    if (flip)
    {
        vis->xiscale = -pspriteiscale;
        vis->startfrac = R_SpriteWidth(lump)-1;
    }
    else
    {
        vis->xiscale = pspriteiscale;
        vis->startfrac = 0;
    }
    
    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch = lump;

    if (viewplayer->powers[pw_invisibility] > 4*32
        || viewplayer->powers[pw_invisibility] & 8)
    {
        // shadow draw
        vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
        // fixed color
        vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
        // full bright
        vis->colormap = colormaps;
    }
    else
    {
        // local light
        vis->colormap = spritelights[MAXLIGHTSCALE-1];
    }
        
    R_DrawVisSprite (vis, vis->x1, vis->x2);
}



//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites (void)
{
    // N_ldbg("R_DrawPlayerSprites\n");
    int         i;
    int         lightnum;
    pspdef_t*   psp;
    
    // get light level
    lightnum =
        (viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) 
        +extralight;

    if (lightnum < 0)           
        spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight[LIGHTLEVELS-1];
    else
        spritelights = scalelight[lightnum];
    
    // clip to screen bounds
    mfloorclip = (short*)screenheightarray;
    mceilingclip = (short*)negonearray;
    
    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
         i<NUMPSPRITES;
         i++,psp++)
    {
        if (psp->state)
            R_DrawPSprite (psp);
    }
}




//
// R_SortVisSprites
//


void R_SortVisSprites (void)
{
    int count = vissprite_p - vissprites;
    // PRINTF("R_SortVisSprites: %d\n", count);
    // NRFD-NOTE: Completely rewritten for singly-linked list
    // PRINTF("R_SortVisSprites: %d\n", count);

    // Initialize sorted list
    vsprsortedhead = &vissprites[0];
    vsprsortedhead->next = NULL;

    if (count < 2) return;

    // Insert items into list in semi-sorted order
    for (int i=1 ; i<count ; i++)
    {
        vissprite_t*   ds = &vissprites[i];
        vissprite_t*   comp = vsprsortedhead;

        while (comp != NULL) {
            vissprite_t*   next = comp->next;
            if ((ds->scale > comp->scale) || (next == NULL)) {
                // Insert after comp
                ds->next = comp->next;
                comp->next = ds;
                break;
            }
            comp = next;
        }
    }

    if (count < 3) return;

    // Bubble sort
    boolean sorted = false;
    while (!sorted) {
        sorted = true;

        vissprite_t    dummy_head;
        vissprite_t*   prev = &dummy_head;
        vissprite_t*   ds = vsprsortedhead;

        dummy_head.next = vsprsortedhead;

        while (ds->next != NULL) {
            vissprite_t*   next = ds->next;
            if (ds->scale > next->scale) {
                // Swap
                prev->next = next;
                ds->next = next->next;
                next->next = ds;
                sorted = false;
            }
            else {
                ds = next;
            }
            prev = prev->next;
        }
        vsprsortedhead = dummy_head.next;
    }

    // PRINTF("  ");
    vissprite_t*        spr;
    int count2 = 0;
    for (spr = vsprsortedhead ;
         spr != NULL ;
         spr=spr->next)
    {
        count2++;
        // PRINTF("%d ", spr->scale);
        if (spr->next && (spr->next->scale < spr->scale)) {
            PRINTF("\n%d %d\n", spr->scale, spr->next->scale);
            I_Error("R_SortVisSprites: sort failed order");
        }
    }
    // PRINTF("\n");
    if (count != count2) {
        I_Error("R_SortVisSprites: sort failed count");
    }

}



//
// R_DrawSprite
//
void R_DrawSprite (vissprite_t* spr)
{
    drawseg_t*          ds;
    short               clipbot[SCREENWIDTH];
    short               cliptop[SCREENWIDTH];
    int                 x;
    int                 r1;
    int                 r2;
    fixed_t             scale;
    fixed_t             lowscale;
    int                 silhouette;

    fixed_t gx  = spr->thing->x;
    fixed_t gy  = spr->thing->y;
    fixed_t gz  = spr->thing->z;
    fixed_t gzt = spr->thing->z + R_SpriteTopOffset(spr->patch);
                
    for (x = spr->x1 ; x<=spr->x2 ; x++)
        clipbot[x] = cliptop[x] = -2;
    
    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--)
    {
        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2
            || ds->x2 < spr->x1
            || (!ds->silhouette
                && !ds->maskedtexturecol) )
        {
            // does not cover sprite
            continue;
        }
                        
        r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
        r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale = ds->scale2;
        }
                
        if (scale < spr->scale
            || ( lowscale < spr->scale
                 && !R_PointOnSegSide (gx, gy, ds->curline) ) )
        {
            // masked mid texture?
            if (ds->maskedtexturecol)   
                R_RenderMaskedSegRange (ds, r1, r2);
            // seg is behind sprite
            continue;                   
        }

        
        // clip this piece of the sprite
        silhouette = ds->silhouette;
        
        if (gz >= ds->bsilheight)
            silhouette &= ~SIL_BOTTOM;

        if (gzt <= ds->tsilheight)
            silhouette &= ~SIL_TOP;
                        

        if (silhouette == 1)
        {
            // bottom sil
            for (x=r1 ; x<=r2 ; x++)
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];
        }
        else if (silhouette == 2)
        {
            // top sil
            for (x=r1 ; x<=r2 ; x++)
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
        }
        else if (silhouette == 3)
        {
            // both
            for (x=r1 ; x<=r2 ; x++)
            {
                if (clipbot[x] == -2)
                    clipbot[x] = ds->sprbottomclip[x];
                if (cliptop[x] == -2)
                    cliptop[x] = ds->sprtopclip[x];
            }
        }
                
    }
    
    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
        if (clipbot[x] == -2)           
            clipbot[x] = viewheight;

        if (cliptop[x] == -2)
            cliptop[x] = -1;
    }
                
    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}




//
// R_DrawMasked
//
void R_DrawMasked (void)
{
    vissprite_t*        spr;
    drawseg_t*          ds;
        
    R_SortVisSprites ();

    if (vissprite_p > vissprites)
    {
        // draw all vissprites back to front
        N_ldbg("R_DrawMasked\n");

        // for (spr=vissprites ; spr<vissprite_p ; spr++)
        // {
        //     R_DrawSprite (spr);
        // }
        for (spr = vsprsortedhead ;
             spr != NULL ;
             spr=spr->next)
        {
            
            R_DrawSprite (spr);
        }
    }

    // render any remaining masked mid textures
    for (ds=ds_p-1 ; ds >= drawsegs ; ds--) {
        if (ds->maskedtexturecol) {
            R_RenderMaskedSegRange (ds, ds->x1, ds->x2);
        }
    }

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset)               
        R_DrawPlayerSprites ();
}


// NRFD-NOTE: Function added to aid memory-optimization
boolean R_SpriteGetFlip(spriteframe_t *sprite, int num) 
{
    return (sprite->flip >> num)&1;
    // NRFD-NOTE: Was:
      // return sprite->flip[num];
}
