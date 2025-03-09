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
//      Here is a core component: drawing the floors and ceilings,
//       while maintaining a per column clipping list only.
//      Moreover, the sky areas have to be determined.
//


#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"



planefunction_t         floorfunc;
planefunction_t         ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
// NRFD-TODO: Find max for Doom 1 (and/or Doom 2?)
#define MAXVISPLANES    48 // 128
visplane_t              visplanes[MAXVISPLANES];
visplane_t*             lastvisplane;
visplane_t*             floorplane;
visplane_t*             ceilingplane;


// NRFD-TODO:  Find max for Doom 1 (and/or Doom 2?)
#define MAXOPENINGS     SCREENWIDTH*16 // *64
short                   openings[MAXOPENINGS];
short*                  lastopening;

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short                   floorclip[SCREENWIDTH];
short                   ceilingclip[SCREENWIDTH];

//
// spanstart holds the start of a plane span
// initialized to 0 at start
// NRFD-NOTE: Was int
short                   spanstart[SCREENHEIGHT];
short                   spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t**          planezlight;
fixed_t                 planeheight;

// NRFD-TODO: CHange view size
const fixed_t                 yslope[SCREENHEIGHT] = {
    0x1EA89,0x1F07C,0x1F693,0x1FCD1,0x20338,0x209C8,0x21084,0x2176C,0x21E84,0x225CC,0x22D47,0x234F7,0x23CDD,0x244FE,0x24D5A,0x255F4,0x25ED0,
    0x267F0,0x27157,0x27B09,0x2850A,0x28F5C,0x29A04,0x2A506,0x2B067,0x2BC2B,0x2C859,0x2D4F4,0x2E204,0x2EF8F,0x2FD9B,0x30C30,0x31B56,0x32B16,
    0x33B79,0x34C89,0x35E50,0x370DC,0x38438,0x39873,0x3AD9B,0x3C3C3,0x3DAFC,0x3F35B,0x40CF6,0x427E5,0x44444,0x46231,0x481CD,0x4A33F,0x4C6AF,
    0x4EC4E,0x51451,0x53EF3,0x56C79,0x59D31,0x5D174,0x609A9,0x64646,0x687D6,0x6CEFA,0x71C71,0x7711D,0x7CE0C,0x83483,0x8A60D,0x92492,0x9B26C,
    0xA5294,0xB08D3,0xBDA12,0xCCCCC,0xDE9BD,0xF3CF3,0x10D794,0x12D2D2,0x155555,0x189D89,0x1D1745,0x238E38,0x2DB6DB,0x400000,0x6AAAAA,
    0x1400000,0x1400000,0x6AAAAA,0x400000,0x2DB6DB,0x238E38,0x1D1745,0x189D89,0x155555,0x12D2D2,0x10D794,0xF3CF3,0xDE9BD,0xCCCCC,
    0xBDA12,0xB08D3,0xA5294,0x9B26C,0x92492,0x8A60D,0x83483,0x7CE0C,0x7711D,0x71C71,0x6CEFA,0x687D6,0x64646,0x609A9,0x5D174,
    0x59D31,0x56C79,0x53EF3,0x51451,0x4EC4E,0x4C6AF,0x4A33F,0x481CD,0x46231,0x44444,0x427E5,0x40CF6,0x3F35B,0x3DAFC,0x3C3C3,
    0x3AD9B,0x39873,0x38438,0x370DC,0x35E50,0x34C89,0x33B79,0x32B16,0x31B56,0x30C30,0x2FD9B,0x2EF8F,0x2E204,0x2D4F4,0x2C859,
    0x2BC2B,0x2B067,0x2A506,0x29A04,0x28F5C,0x2850A,0x27B09,0x27157,0x267F0,0x25ED0,0x255F4,0x24D5A,0x244FE,0x23CDD,0x234F7,
    0x22D47,0x225CC,0x21E84,0x2176C,0x21084,0x209C8,0x20338,0x1FCD1,0x1F693,0x1F07C,0x1EA89};
const fixed_t                 distscale[SCREENWIDTH] = {
    0x16A75,0x16912,0x167FA,0x166E4,0x165D0,0x164BF,0x163B0,0x16262,0x16158,0x16052,0x15F0B,0x15E0B,0x15D0B,0x15BCE,0x15AD6,
    0x1599F,0x158AB,0x1577C,0x1568B,0x15561,0x15475,0x15353,0x15232,0x1514E,0x15034,0x14F1B,0x14E08,0x14D2D,0x14C1D,0x14B13,
    0x14A0A,0x14902,0x14800,0x146FF,0x14601,0x14506,0x1440D,0x14319,0x14226,0x14136,0x14018,0x13F2D,0x13E46,0x13D60,0x13C50,
    0x13B70,0x13A91,0x1398B,0x138B2,0x137B1,0x136DE,0x135E4,0x13516,0x13420,0x1332F,0x13267,0x1317C,0x13093,0x12FAE,0x12EF1,
    0x12E10,0x12D33,0x12C58,0x12B80,0x12AAB,0x129D9,0x1290A,0x1283E,0x12773,0x126AC,0x125C7,0x12506,0x12446,0x1238B,0x122B2,
    0x121FB,0x12146,0x12077,0x11FC7,0x11EFD,0x11E54,0x11D90,0x11CEB,0x11C2D,0x11B8D,0x11AD5,0x11A20,0x1196E,0x118D7,0x1182A,
    0x11780,0x116DA,0x11635,0x11594,0x114F6,0x1145A,0x113C1,0x1132A,0x11297,0x11206,0x11177,0x110EB,0x11062,0x10FDB,0x10F45,
    0x10EC3,0x10E44,0x10DB6,0x10D3C,0x10CC5,0x10C40,0x10BCD,0x10B4E,0x10AE0,0x10A66,0x109FE,0x10989,0x10926,0x108B7,0x1084B,
    0x107F0,0x10789,0x10725,0x106D0,0x10671,0x10616,0x105BD,0x10567,0x1051D,0x104CC,0x1047E,0x10433,0x103EA,0x103A4,0x10360,
    0x1031E,0x102E0,0x102A4,0x1026B,0x10234,0x10200,0x101CF,0x101A0,0x10174,0x1014A,0x10123,0x100FD,0x100DB,0x100BB,0x1009E,
    0x10080,0x10069,0x10053,0x10040,0x10030,0x10022,0x10016,0x1000C,0x10006,0x10002,0x10001,0x10002,0x10005,0x1000B,0x10015,
    0x10020,0x1002E,0x1003E,0x10051,0x10066,0x1007D,0x1009B,0x100B8,0x100D7,0x100F9,0x1011E,0x10145,0x1016F,0x1019A,0x101C9,
    0x101FA,0x1022E,0x10264,0x1029D,0x102D9,0x10317,0x10358,0x1039A,0x103E0,0x1042A,0x10475,0x104C3,0x10514,0x1055C,0x105B2,
    0x1060A,0x10665,0x106C4,0x10719,0x1077C,0x107E2,0x1083E,0x108AA,0x10918,0x1097B,0x109F0,0x10A57,0x10AD1,0x10B3E,0x10BBD,
    0x10C2F,0x10CB4,0x10D2C,0x10DA4,0x10E33,0x10EB1,0x10F31,0x10FC8,0x1104F,0x110D8,0x11164,0x111F1,0x11282,0x11315,0x113AC,
    0x11444,0x114DF,0x1157E,0x1161F,0x116C2,0x11769,0x11812,0x118BF,0x11954,0x11A06,0x11ABA,0x11B72,0x11C13,0x11CD0,0x11D75,
    0x11E37,0x11EE2,0x11FAA,0x1205A,0x12128,0x121DC,0x12293,0x1236C,0x12427,0x124E5,0x125A6,0x1268C,0x12752,0x1281B,0x128E7,
    0x129B7,0x12A88,0x12B5C,0x12C34,0x12D0E,0x12DEA,0x12ECB,0x12F87,0x1306D,0x13155,0x13241,0x13307,0x133F8,0x134EB,0x135BB,
    0x136B4,0x13788,0x13887,0x13960,0x13A65,0x13B44,0x13C23,0x13D32,0x13E17,0x13EFE,0x13FE9,0x14105,0x141F5,0x142E7,0x143DD,
    0x144D4,0x145CF,0x146CC,0x147CB,0x148CF,0x149D4,0x14ADD,0x14BE8,0x14CF7,0x14DD0,0x14EE5,0x14FFB,0x15116,0x151FA,0x15319,
    0x1543B,0x15527,0x1564E,0x1573F,0x1586E,0x15963,0x15A97,0x15B90,0x15CCC,0x15DCB,0x15ECC,0x16010,0x16115,0x1621F,0x1636D,
    0x1647B,0x1658C,0x1669E,0x167B3,0x168CC};
fixed_t                 basexscale;
fixed_t                 baseyscale;

// fixed_t                 cachedheight[SCREENHEIGHT];
// fixed_t                 cacheddistance[SCREENHEIGHT];
// fixed_t                 cachedxstep[SCREENHEIGHT];
// fixed_t                 cachedystep[SCREENHEIGHT];

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
  // Doh!
}


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
void
R_MapPlane
( int           y,
  int           x1,
  int           x2 )
{
    angle_t     angle;
    fixed_t     distance;
    fixed_t     length;
    unsigned    index;
        
#ifdef RANGECHECK
    if (x2 < x1
     || x1 < 0
     || x2 >= viewwidth
     || y > viewheight)
    {
        I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
    }
#endif

    // NRFD-TODO: optimize
    /*
    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul (distance,basexscale);
        ds_ystep = cachedystep[y] = FixedMul (distance,baseyscale);
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }
    */

    distance  = FixedMul (planeheight, yslope[y]);
    ds_xstep = FixedMul (distance,basexscale);
    ds_ystep = FixedMul (distance,baseyscale);
    
    length = FixedMul (distance,distscale[x1]);
    angle = (viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length);
    ds_yfrac = -viewy - FixedMul(finesine[angle], length);

    if (fixedcolormap)
        ds_colormap = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;
        
        if (index >= MAXLIGHTZ )
            index = MAXLIGHTZ-1;

        ds_colormap = planezlight[index];
    }
        
    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc ();
}


//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes (void)
{
    int         i;
    angle_t     angle;
    
    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;

    lastopening = openings;
    
    // texture calculation
    // NRFD-TODO: Optimizations
    // memset (cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;
        
    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}




//
// R_FindPlane
//
visplane_t*
R_FindPlane
( fixed_t       height,
  int           picnum,
  int           lightlevel )
{
    // PRINTF("R_FindPlane\n");
    visplane_t* check;
        
    if (picnum == skyflatnum)
    {
        height = 0;                     // all skys map together
        lightlevel = 0;
    }
        
    for (check=visplanes; check<lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel)
        {
            break;
        }
    }
    
                        
    if (check < lastvisplane)
        return check;
                
    if (lastvisplane - visplanes == MAXVISPLANES)
        I_Error ("R_FindPlane: no more visplanes");
                
    lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;

    memset (check->top,0xff,sizeof(check->top));
                
    return check;
}


//
// R_CheckPlane
//
visplane_t*
R_CheckPlane
( visplane_t*   pl,
  int           start,
  int           stop )
{
    // PRINTF("R_CheckPlane\n");
    int         intrl;
    int         intrh;
    int         unionl;
    int         unionh;
    int         x;
        
    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }
        
    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x=intrl ; x<= intrh ; x++)
        if (pl->top[x] != 0xff)
            break;

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;

        // use the same one
        return pl;              
    }
        
    // make a new visplane
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    
    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    memset (pl->top,0xff,sizeof(pl->top));
                
    return pl;
}


//
// R_MakeSpans
//
void
R_MakeSpans
( int           x,
  int           t1,
  int           b1,
  int           t2,
  int           b2 )
{
    while (t1 < t2 && t1<=b1)
    {
        R_MapPlane (t1,spanstart[t1],x-1);
        t1++;
    }
    while (b1 > b2 && b1>=t1)
    {
        R_MapPlane (b1,spanstart[b1],x-1);
        b1--;
    }
        
    while (t2 < t1 && t2<=b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2>=t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes (void)
{
    visplane_t*         pl;
    int                 light;
    int                 x;
    int                 stop;
    int                 angle;
    int                 lumpnum;
                                
#ifdef RANGECHECK
    if (ds_p - drawsegs > MAXDRAWSEGS)
        I_Error ("R_DrawPlanes: drawsegs overflow (%i)",
                 ds_p - drawsegs);
    
    if (lastvisplane - visplanes > MAXVISPLANES)
        I_Error ("R_DrawPlanes: visplane overflow (%i)",
                 lastvisplane - visplanes);
    
    if (lastopening - openings > MAXOPENINGS)
        I_Error ("R_DrawPlanes: opening overflow (%i)",
                 lastopening - openings);
#endif

    for (pl = visplanes ; pl < lastvisplane ; pl++)
    {
        if (pl->minx > pl->maxx)
            continue;

        
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = pspriteiscale>>detailshift;
            
            // Sky is allways drawn full bright,
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.
            dc_colormap = colormaps; 

            dc_texturemid = skytexturemid;
            for (x=pl->minx ; x <= pl->maxx ; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];

                if (dc_yl <= dc_yh)
                {
                    angle = (viewangle + xtoviewangle[x])>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetCachedColumn(skytexture, angle);
                    colfunc ();
                }
            }
            continue;
        }
        
        // regular flat
        lumpnum = firstflat + flattranslation[pl->picnum];
        ds_source = W_CacheLumpNum(lumpnum, PU_STATIC);
        
        planeheight = abs(pl->height-viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;

        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS-1;

        if (light < 0)
            light = 0;

        planezlight = zlight[light];

        pl->top[pl->maxx+1] = 0xff;
        pl->top[pl->minx-1] = 0xff;
                
        stop = pl->maxx + 1;

        for (x=pl->minx ; x<= stop ; x++)
        {
            R_MakeSpans(x,pl->top[x-1],
                        pl->bottom[x-1],
                        pl->top[x],
                        pl->bottom[x]);
        }
        
        W_ReleaseLumpNum(lumpnum);
    }
}
