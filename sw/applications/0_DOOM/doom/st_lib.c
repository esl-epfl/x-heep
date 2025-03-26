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
//      The status bar widget code.
//


#include <stdio.h>
#include <ctype.h>

#include "deh_doomTop.h"
#include "doomdef.h"

#include "z_zone.h"
#include "v_video.h"

#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"
#include "x_spi.h"


// in AM_map.c
extern boolean          automapactive; 




//
// Hack display negative frags.
//  Loads and store the stminus lump.
//
patch_t*                sttminus; // X-HEEP comment : sttminus is an adress in flash it must be read using X_spi_read

void STlib_init(void)
{
    sttminus = (patch_t *) W_CacheLumpName(DEH_String("STTMINUS"), PU_STATIC);
}


// ?
void
STlib_initNum
( st_number_t*          n,
  short                 x,
  short                 y,
  patch_t**             pl,
  short*                        num,
  boolean*              on,
  short                 width )
{
    n->x        = x;
    n->y        = y;
    n->oldnum   = 0;
    n->width    = width;
    n->num      = num;
    n->on       = on;
    n->p        = pl;
}


// 
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
// NRFD-NOTE: int changed to short
void
STlib_drawNum
( st_number_t*  n,
  boolean       refresh )
{

    short               numdigits = n->width;
    short               num = *n->num;
    
    patch_t n_temp; 
    X_spi_read(n->p[0], &n_temp, sizeof(n_temp)/4); 

    short               w = SHORT(n_temp.width);
    short               h = SHORT(n_temp.height);
    short               x = n->x;
    
    int         neg;

    n->oldnum = *n->num;

    neg = num < 0;

    if (neg)
    {
        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;
        
        num = -num;
    }

    // clear the area
    x = n->x - numdigits*w;

    if (n->y - ST_Y < 0)
        I_Error("drawNum: n->y - ST_Y < 0");

    V_CopyRect(x, n->y - ST_Y, st_backing_screen, w*numdigits, h, x, n->y);

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - w, n->y, n->p[ 0 ]);

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        V_DrawPatch(x, n->y, n->p[ num % 10 ]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawPatch(x - 8, n->y, sttminus);
}


//
void
STlib_updateNum
( st_number_t*          n,
  boolean               refresh )
{
    if (*n->on) STlib_drawNum(n, refresh);
}


// NRFD-NOTE: int changed to short
void
STlib_initPercent
( st_percent_t*         p,
  short                 x,
  short                 y,
  patch_t**             pl,
  short*                 num,
  boolean*              on,
  patch_t*              percent )
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}




void
STlib_updatePercent
( st_percent_t*         per,
  int                   refresh )
{
    if (refresh && *per->n.on)
        V_DrawPatch(per->n.x, per->n.y, per->p);
    
    STlib_updateNum(&per->n, refresh);
}


// NRFD-NOTE: int changed to short
void
STlib_initMultIcon
( st_multicon_t*        i,
  short                 x,
  short                 y,
  patch_t**             il,
  short*                inum,
  boolean*              on )
{
    i->x        = x;
    i->y        = y;
    i->oldinum  = -1;
    i->inum     = inum;
    i->on       = on;
    i->p        = il;
}


// NRFD-NOTE: int changed to short
void
STlib_updateMultIcon
( st_multicon_t*        mi,
  boolean               refresh )
{
    short                 w;
    short                 h;
    short                 x;
    short                 y;

    patch_t p_temp; 
    X_spi_read(mi->p[mi->oldinum], &p_temp, sizeof(p_temp)/4); 
    if (*mi->on
        && (mi->oldinum != *mi->inum || refresh)
        && (*mi->inum!=-1))
    {
        if (mi->oldinum != -1)
        {
            x = mi->x - SHORT(p_temp.leftoffset);
            y = mi->y - SHORT(p_temp.topoffset);
            w = SHORT(p_temp.width);
            h = SHORT(p_temp.height);

            if (y - ST_Y < 0)
                I_Error("updateMultIcon: y - ST_Y < 0");

            V_CopyRect(x, y-ST_Y, st_backing_screen, w, h, x, y);
        }
        V_DrawPatch(mi->x, mi->y, mi->p[*mi->inum]);
        mi->oldinum = *mi->inum;
    }
}


// NRFD-NOTE: int changed to short
void
STlib_initBinIcon
( st_binicon_t*         b,
  short                 x,
  short                 y,
  patch_t*              i,
  boolean*              val,
  boolean*              on )
{
    b->x        = x;
    b->y        = y;
    b->oldval   = false;
    b->val      = val;
    b->on       = on;
    b->p        = i;
}


// NRFD-NOTE: int changed to short
void
STlib_updateBinIcon
( st_binicon_t*         bi,
  boolean               refresh )
{
    short                 x;
    short                 y;
    short                 w;
    short                 h;

    if (*bi->on
     && (bi->oldval != *bi->val || refresh))
    {
        patch_t p_temp; 
        X_spi_read(bi->p, &p_temp, sizeof(p_temp)/4); 
        x = bi->x - SHORT(p_temp.leftoffset);
        y = bi->y - SHORT(p_temp.topoffset);
        w = SHORT(p_temp.width);
        h = SHORT(p_temp.height);

        if (y - ST_Y < 0)
            I_Error("updateBinIcon: y - ST_Y < 0");

        if (*bi->val)
            V_DrawPatch(bi->x, bi->y, bi->p);
        else
            V_CopyRect(x, y-ST_Y, st_backing_screen, w, h, x, y);

        bi->oldval = *bi->val;
    }

}

