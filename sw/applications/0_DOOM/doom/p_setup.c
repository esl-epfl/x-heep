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
//      Do all the WAD I/O, get map description,
//      set up initial state and misc. LUTs.
//



#include <math.h>

#include "z_zone.h"

#include "deh_doomTop.h"
#include "i_swap.h"
#include "m_argv.h"
#include "m_bbox.h"

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

//#include "s_sound.h"

#include "doomstat.h"
#include "x_spi.h"
#include "n_mem.h"


void    P_SpawnMapThing (mapthing_t*    mthing);

// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int             numvertexes;
//vertex_t*       vertexes;
mapvertex_t*    mapvertexes; // X-HEEP comment : mapvertexes is an adress in flash it must be read using X_spi_read


int             numsegs;
// seg_t*          segs;
mapseg_t*          mapsegs; // X-HEEP comment : mapsegs is an adress in flash it must be read using X_spi_read

int             numsectors;
sector_t*       sectors;

int             numsubsectors;
subsector_t*    subsectors;

int             numnodes;
// node_t*         nodes;
mapnode_t*         mapnodes; // X-HEEP comment : mapnodes is an adress in flash it must be read using X_spi_read

int             numlines;
line_t*         lines; //X-HEEP comment: lines->mld is an adress in flash it must be read using X_spi_read

int             numsides;
side_t*         sides;
mapsidedef_t*   mapsides; // X-HEEP comment : mapsides is an adress in flash it must be read using X_spi_read

static int      totallines;

line_t**            linebuffer;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int             bmapwidth;
int             bmapheight;     // size in mapblocks
short*          blockmap;       // int for larger maps // X-HEEP comment : blockmap is an adress in flash it must be read using X_spi_read 
// offsets in blockmap are from here
short*          blockmaplump;  // X-HEEP comment : blockmaplump is an adress in flash it must be read using X_spi_read 
// origin of block map
fixed_t         bmaporgx;
fixed_t         bmaporgy;
// for thing chains
mobj_t*         blocklinks[BLOCKLINKS_SIZE];             


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*           rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS   10

mapthing_t      deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t*     deathmatch_p;
mapthing_t      playerstarts[MAXPLAYERS];





//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{

    /*
    byte*               data; // X-HEEP comment : data is an adress in flash it must be read using X_spi_read
    int                 i;
    mapvertex_t*        ml;// X-HEEP comment : ml is an adress in flash it must be read using X_spi_read
    vertex_t*           li;
    */

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);
    
    mapvertexes = (mapvertex_t*)W_CacheLumpNum(lump, PU_LEVEL); 

    // NRFD-TODO: Access mapvertexes directly?
    PRINTF("P_LoadVertexes: lump = %d, lumplength = %d, numvertexes = %d\n", lump,  W_LumpLength (lump), numvertexes);

    /* X-HEEP comment : avoid Z_Malloc of vertexes 

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0); 
    printf ("vertexes : %d\n", vertexes);      

    // Load data into cache.
    data = W_CacheLumpNum (lump, PU_STATIC);
        
    ml = (mapvertex_t *)data;
    li = vertexes;

    mapvertex_t temp_ml; 
    X_spi_read(ml, &temp_ml, sizeof(temp_ml)/4); 
    printf ("before loop \n"); 
    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
        li->x = SHORT(temp_ml.x)<<FRACBITS;
        li->y = SHORT(temp_ml.y)<<FRACBITS;
    }

    // Free buffer memory.
    //W_ReleaseLumpNum(lump); //useless 

    */
}

//
// GetSectorAtNullAddress
//
sector_t* GetSectorAtNullAddress(void)
{
    static boolean null_sector_is_initialized = false;
    static sector_t null_sector;

    if (!null_sector_is_initialized)
    {
        memset(&null_sector, 0, sizeof(null_sector));
        I_GetMemoryValue(0, &null_sector.floorheight, 4);
        I_GetMemoryValue(4, &null_sector.ceilingheight, 4);
        null_sector_is_initialized = true;
    }

    return &null_sector;
}

//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    PRINTF("P_LoadSegs\n");

    /*
    byte*               data;
    int                 i;
    mapseg_t*           ml;
    seg_t*              li;
    line_t*             ldef;
    int                 linedef;
    int                 side;
    int                 sidenum;
    */

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    mapsegs = (mapseg_t*)W_CacheLumpNum(lump, PU_LEVEL);

/*
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0); 
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
        li->ms = ml;
        
        // li->v1 = &vertexes[SHORT(ml->v1)];
        // li->v2 = &vertexes[SHORT(ml->v2)];

        // li->angle = (SHORT(ml->angle))<<FRACBITS;
        // li->offset = (SHORT(ml->offset))<<FRACBITS;
        linedef = SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);

        // e6y: check for wrong indexes
        if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
        {
            I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d",
                    linedef, i, (unsigned)ldef->sidenum[side]);
        }

        li->sidedef = &sides[ldef->sidenum[side]];

        li->frontsector = sides[ldef->sidenum[side]].sector;

        if (ldef-> flags & ML_TWOSIDED)
        {
            sidenum = ldef->sidenum[side ^ 1];

            // If the sidenum is out of range, this may be a "glass hack"
            // impassible window.  Point at side #0 (this may not be
            // the correct Vanilla behavior; however, it seems to work for
            // OTTAWAU.WAD, which is the one place I've seen this trick
            // used).

            if (sidenum < 0 || sidenum >= numsides)
            {
                I_Error("TODO?");
                // li->backsector = GetSectorAtNullAddress();
            }
            else
            {
                li->backsector = sides[sidenum].sector;
            }
        }
        else
        {
            li->backsector = 0;
        }
    }
        
    W_ReleaseLumpNum(lump);
        */
}

// X-HEEP comment : This function returns an adress in flash that must be read using X_spi_read
seg_t *GetSeg(int num)
{
    return (seg_t*)&mapsegs[num];
}

sector_t *SegFrontSector(seg_t *seg) {
    // NRFD-TODO: Optimize: Use numbers insteadof pointers?
    side_t *sidedef = SegSideDef(seg);
    return SideSector(sidedef);
}

sector_t *SegBackSector(seg_t *seg) {
    mapseg_t *ms = (mapseg_t*)seg;
    line_t *linedef = SegLineDef(seg);
    mapseg_t temp_ms; 
    X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4);
    if (LineFlags(linedef) & ML_TWOSIDED) {
        int side = SHORT(temp_ms.side);
        side = side ^ 1;
            // NRFD-TODO: Optimize: Use numbers insteadof pointers?
        side_t *line_side = LineSide(linedef, side);
        return SideSector(line_side);
    }
    else {
        return NULL;
    }
}

vertex_t SegV1(seg_t *seg) {
    mapseg_t *ms = (mapseg_t*)seg;
    mapseg_t temp_ms; 
    X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4);

    int v1_num = SHORT(temp_ms.v1);

    mapvertex_t temp_ml; 

    X_spi_read(mapvertexes + v1_num, &temp_ml, sizeof(temp_ml)/4);
    
    vertex_t linev1;
    linev1.x = (SHORT(temp_ml.x)<<FRACBITS); 
    linev1.y = (SHORT(temp_ml.y)<<FRACBITS); 
    
    return linev1;
}
vertex_t SegV2(seg_t *seg) {
    mapseg_t *ms = (mapseg_t*)seg;
    mapseg_t temp_ms; 
    X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4);

    int v2_num = SHORT(temp_ms.v2);

    mapvertex_t temp_ml; 

    X_spi_read(mapvertexes + v2_num, &temp_ml, sizeof(temp_ml)/4);
    
    vertex_t linev2;
    linev2.x = (SHORT(temp_ml.x)<<FRACBITS); 
    linev2.y = (SHORT(temp_ml.y)<<FRACBITS);

    return linev2; 
}
angle_t SegAngle(seg_t *seg) {
    mapseg_t *ms = (mapseg_t*)seg;

    return (SHORT(ms->angle))<<FRACBITS;
}
fixed_t SegOffset(seg_t *seg) {
    mapseg_t *ms = (mapseg_t*)seg;

    return (SHORT(ms->offset))<<FRACBITS;
}
line_t *SegLineDef(seg_t *seg)
{
    mapseg_t *ms = (mapseg_t*)seg;
    mapseg_t temp_ms; 
    X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4);
    int num = temp_ms.linedef;
    return &lines[num];
}
side_t *SegSideDef(seg_t *seg)
{
    mapseg_t *ms = (mapseg_t*)seg;

    mapseg_t temp_ms; 
    X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4); 

    short side = SHORT(temp_ms.side);
    line_t *ldef = SegLineDef(seg);
    return LineSide(ldef, side);
}
//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    PRINTF("P_LoadSubsectors\n");
    
    byte*               data; // X-HEEP comment : data is an adress in flash it must be read using X_spi_read
    int                 i;
    mapsubsector_t*     ms; // X-HEEP comment : ms is an adress in flash it must be read using X_spi_read
    subsector_t*        ss;
        
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = N_malloc(numsubsectors*sizeof(subsector_t));
    //subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);       
    data = W_CacheLumpNum (lump,PU_STATIC);
        
    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;
    
    mapsubsector_t temp_ms;
    
    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
        X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4); 
        ss->numlines = SHORT(temp_ms.numsegs);
        ss->firstline = SHORT(temp_ms.firstseg);

    }
        
    W_ReleaseLumpNum(lump);
}



//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    PRINTF("P_LoadSectors\n");

    byte*               data; //X-HEEP comment: data is an adress in flash it must be read using X_spi_read
    int                 i;
    mapsector_t*        ms;//X-HEEP comment: ms is an adress in flash it must be read using X_spi_read
    sector_t*           ss;
        
    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = N_malloc(numsectors*sizeof(sector_t)); 
    //sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);        
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsector_t *)data;
    ss = sectors;

    mapsector_t temp_ms;  

    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
        X_spi_read(ms, &temp_ms, sizeof(temp_ms)/4); 
        ss->floorheight = SHORT(temp_ms.floorheight)<<FRACBITS;
        ss->ceilingheight = SHORT(temp_ms.ceilingheight)<<FRACBITS;
        ss->floorpic = R_FlatNumForName(temp_ms.floorpic);
        ss->ceilingpic = R_FlatNumForName(temp_ms.ceilingpic);
        ss->lightlevel = SHORT(temp_ms.lightlevel);
        ss->special = SHORT(temp_ms.special);
        ss->tag = SHORT(temp_ms.tag);
        ss->thinglist = NULL;
    }
        
    W_ReleaseLumpNum(lump);
}

degenmobj_t *SectorSoundOrg(sector_t *sec)
{
    return NULL;
}

//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    PRINTF("P_LoadNodes\n");

    // byte*       data;
    // int         i;
    // int         j;
    // int         k;
    // mapnode_t*  mn;
    // node_t*     no;
      
    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    mapnodes = (mapnode_t*)W_CacheLumpNum(lump, PU_LEVEL);
      /*
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);      
    data = W_CacheLumpNum (lump,PU_STATIC);
        
    mn = (mapnode_t *)data;
    no = nodes;
    
    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
        no.x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;
        for (j=0 ; j<2 ; j++)
        {
            no->children[j] = SHORT(mn->children[j]);
            for (k=0 ; k<4 ; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }
        
    W_ReleaseLumpNum(lump);*/

}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    PRINTF("P_LoadThings\n");
    
    byte               *data;  //X-HEEP comment: data is an adress in flash it must be read using X_spi_read
    int                 i;
    mapthing_t         *mt;  //X-HEEP comment: mt is an adress in flash it must be read using X_spi_read
    mapthing_t          spawnthing;
    int                 numthings;
    boolean             spawn;

    data = W_CacheLumpNum (lump,PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);

    P_InitMobjs(numthings);

        
    mt = (mapthing_t *)data;
    mapthing_t temp_mt; 
    uint32_t temp_data[(sizeof(temp_mt)+3)/4];

    for (i=0 ; i<numthings ; i++, mt++)
    {
        X_spi_read(mt, &temp_data, (sizeof(temp_mt)+3)/4); 
        memcpy(&temp_mt, &temp_data, sizeof(temp_mt));  
        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if (gamemode != commercial)
        {
            switch (SHORT(temp_mt.type))
            {
              case 68:  // Arachnotron
              case 64:  // Archvile
              case 88:  // Boss Brain
              case 89:  // Boss Shooter
              case 69:  // Hell Knight
              case 67:  // Mancubus
              case 71:  // Pain Elemental
              case 65:  // Former Human Commando
              case 66:  // Revenant
              case 84:  // Wolf SS
                spawn = false;
                break;
            }
        }
        if (spawn == false)
            break;

        // Do spawn all other stuff. 
        spawnthing.x = SHORT(temp_mt.x);
        spawnthing.y = SHORT(temp_mt.y);
        spawnthing.angle = SHORT(temp_mt.angle);
        spawnthing.type = SHORT(temp_mt.type);
        spawnthing.options = SHORT(temp_mt.options);
        P_SpawnMapThing(&spawnthing);
    }

    //W_ReleaseLumpNum(lump); //useless
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    PRINTF("P_LoadLineDefs\n");

    byte*               data;  //X-HEEP comment: data is an adress in flash it must be read using X_spi_read
    int                 i;
    maplinedef_t*       mld;  //X-HEEP comment: mld is an adress in flash it must be read using X_spi_read
    line_t*             ld;
    vertex_t*           v1;
    vertex_t*           v2;
    fixed_t             dx, dy;
        
    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = N_malloc(numlines*sizeof(line_t));
    //lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);      
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
        
    mld = (maplinedef_t *)data;
    maplinedef_t temp_mld; 
    uint32_t temp_data[(sizeof(temp_mld)+3)/4];
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
        X_spi_read(mld, &temp_data, (sizeof(temp_mld)+3)/4); 
        memcpy(&temp_mld, &temp_data, sizeof(temp_mld));    
        ld->mld = mld;
        // ld->flags_x = SHORT(mld->flags);
        short special = SHORT(temp_mld.special);
        if (special > 256 || special < 0) {
            I_Error("P_LoadLineDefs: special");
        }
        ld->special = special;
        // ld->tag_x = SHORT(mld->tag);

        short v1_num = SHORT(temp_mld.v1);
        short v2_num = SHORT(temp_mld.v2);

        if (v1_num > numvertexes) I_Error("P_LoadLineDefs v1 %d > %d", v1_num, numvertexes);
        if (v2_num > numvertexes) I_Error("P_LoadLineDefs v2 %d > %d", v2_num, numvertexes);

        // v1 = &vertexes[v1_num];
        // v2 = &vertexes[v2_num];
        // dx = v2->x - v1->x;
        // dy = v2->y - v1->y;

        // // ld->dx = v2->x - v1->x;
        // // ld->dy = v2->y - v1->y;
        
        // if (!dx)
        //     ld->slopetype = ST_VERTICAL;
        // else if (!dy)
        //     ld->slopetype = ST_HORIZONTAL;
        // else
        // {
        //     if (FixedDiv (dy , dx) > 0)
        //         ld->slopetype = ST_POSITIVE;
        //     else
        //         ld->slopetype = ST_NEGATIVE;
        // }
                
        /*
        // NRFD-NOTE: Moved to LineBBox
        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }

        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }
        */

        // ld->sidenum[0] = SHORT(mld->sidenum[0]);
        // ld->sidenum[1] = SHORT(mld->sidenum[1]);

        // if (ld->sidenum[0] != -1)
        //     ld->frontsector = sides[ld->sidenum[0]].sector;
        // else
        //     ld->frontsector = 0;

        // if (ld->sidenum[1] != -1)
        //     ld->backsector = sides[ld->sidenum[1]].sector;
        // else
        //     ld->backsector = 0;
    }

    // W_ReleaseLumpNum(lump);
}

vertex_t LineV1(line_t *line)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld)); 
    short v1_num = SHORT(temp_mld.v1);

    mapvertex_t temp_ml;
    X_spi_read(mapvertexes + v1_num, &temp_ml, sizeof(temp_ml)/4);
    
    vertex_t linev1;
    linev1.x = (SHORT(temp_ml.x)<<FRACBITS); 
    linev1.y = (SHORT(temp_ml.y)<<FRACBITS); 
    
    return linev1;
}
vertex_t LineV2(line_t *line)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld)); 
    short v2_num = SHORT(temp_mld.v2);
    
    mapvertex_t temp_ml; 
    X_spi_read(mapvertexes + v2_num, &temp_ml, sizeof(temp_ml)/4);
    
    vertex_t linev2;
    linev2.x = (SHORT(temp_ml.x)<<FRACBITS); 
    linev2.y = (SHORT(temp_ml.y)<<FRACBITS); 
    
    return linev2;
}
short LineFlags(line_t *line)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld));  
    return SHORT(temp_mld.flags);
}
void LineSetMapped(line_t *line)
{
    line->validcount |= 0x80;
}
boolean LineIsMapped(line_t *line)
{
    return (line->validcount & 0x80);
}
short LineTag(line_t *line)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld));  
    return SHORT(temp_mld.tag);
}
void LineTagSet666(line_t *line)
{
    I_Error("NRFD-TODO: LineTagSet666");
}
short   LineSideNum(line_t *line, int num)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld));
    return  SHORT(temp_mld.sidenum[num]);
}
side_t*   LineSide(line_t *line, int num)
{
    maplinedef_t temp_mld; 
    uint32_t temp_mld_data[(sizeof(temp_mld)+3)/4];
    X_spi_read(line->mld, &temp_mld_data, (sizeof(temp_mld)+3)/4);
    memcpy(&temp_mld, &temp_mld_data, sizeof(temp_mld));  
    short sn = SHORT(temp_mld.sidenum[num]);
    if (sn == -1) return NULL;
    return &sides[sn];
}

sector_t *LineFrontSector(line_t *ld) 
{
    // NRFD-TODO: Optimize: Use numbers insteadof pointers?
    side_t *side = LineSide(ld,0);
    if (side)
        return SideSector(side);
    else
        return NULL;
}
sector_t *LineBackSector(line_t *ld)
{
    // NRFD-TODO: Optimize: Use numbers insteadof pointers?
    side_t *side = LineSide(ld,1);
    if (side)
        return SideSector(side);
    else
        return NULL;
}

slopetype_t LineSlopeType(line_t *line)
{
    vector_t vec = LineVector(line);
    if      (!vec.dx) return ST_VERTICAL;
    else if (!vec.dy) return ST_HORIZONTAL;
    else
    {
        if (FixedDiv (vec.dy , vec.dx) > 0)
            return ST_POSITIVE;
        else
            return ST_NEGATIVE;
    }
}

vector_t LineVector(line_t* ld)
{
    // NRFD-TODO: Optimize?
    vector_t result;
    vertex_t  v1 = LineV1(ld);
    vertex_t  v2 = LineV2(ld);
    result.dx = v2.x - v1.x;
    result.dy = v2.y - v1.y;
    return result;
}

static fixed_t shared_bbox[4];

fixed_t* LineBBox(line_t* ld)
{
    // NRFD-TODO: Optimize?

    vertex_t           v1;
    vertex_t           v2;

    v1 = LineV1(ld);
    v2 = LineV2(ld);

    // if (ld->v1 >= &vertexes[numvertexes]) I_Error("LineBBox v1 overflow %X vs %X", (unsigned int)ld->v1, (unsigned int)vertexes);
    // if (ld->v1 <  &vertexes[0])           I_Error("LineBBox v1 underflow %X vs %X", (unsigned int)ld->v1, (unsigned int)vertexes);
    // if (ld->v2 >= &vertexes[numvertexes]) I_Error("LineBBox v2 overflow %X vs %X", (unsigned int)ld->v2, (unsigned int)vertexes);
    // if (ld->v2 <  &vertexes[0])           I_Error("LineBBox v2 underflow %X vs %X", (unsigned int)ld->v2, (unsigned int)vertexes);

    if (v1.x < v2.x)
    {
        shared_bbox[BOXLEFT] = v1.x;
        shared_bbox[BOXRIGHT] = v2.x;
    }
    else
    {
        shared_bbox[BOXLEFT] = v2.x;
        shared_bbox[BOXRIGHT] = v1.x;
    }

    if (v1.y < v2.y)
    {
        shared_bbox[BOXBOTTOM] = v1.y;
        shared_bbox[BOXTOP] = v2.y;
    }
    else
    {
        shared_bbox[BOXBOTTOM] = v2.y;
        shared_bbox[BOXTOP] = v1.y;
    }
    return shared_bbox;
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    PRINTF("P_LoadSideDefs\n");

    byte*               data; //X-HEEP comment: data is an adress in flash it must be read using X_spi_read
    int                 i;
    mapsidedef_t*       msd; //X-HEEP comment: msd is an adress in flash it must be read using X_spi_read
    side_t*             sd;
        
    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = N_malloc(numsides*sizeof(side_t));
    //sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);      
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
    
    msd = (mapsidedef_t *)data;
    mapsides = msd; 
    sd = sides;

    mapsidedef_t temp_msd; 
    uint32_t temp_data[(sizeof(temp_msd)+3)/4];
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
        X_spi_read(msd, &temp_data, (sizeof(temp_msd)+3)/4); 
        memcpy(&temp_msd, &temp_data, sizeof(temp_msd));   
        // sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
        // sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
        sd->textureoffset_short = SHORT(temp_msd.textureoffset);
        sd->rowoffset_short = SHORT(temp_msd.rowoffset);
        sd->toptexture = R_TextureNumForName(temp_msd.toptexture);
        sd->bottomtexture = R_TextureNumForName(temp_msd.bottomtexture);
        sd->midtexture = R_TextureNumForName(temp_msd.midtexture);
        // sd->sector = &sectors[SHORT(msd->sector)];
        sd->sector_num = SHORT(temp_msd.sector);
        if (sd->sector_num < 0 || sd->sector_num > 256) {
            I_Error("P_LoadSideDefs: sector_num");
        }
    }

    // W_ReleaseLumpNum(lump);
}

fixed_t R_SideTextureOffset(side_t *side)
{
    return side->textureoffset_short << FRACBITS;
}
fixed_t R_SideRowOffset(side_t *side)
{
    return side->rowoffset_short << FRACBITS;
}
sector_t* SideSector(side_t *side)
{
    return &sectors[side->sector_num];
}
sector_t* SideNumSector(int sidenum)
{
    return &sectors[sides[sidenum].sector_num];
    // mapsidedef_t *msd = &mapsides[sidenum];
    // return &sectors[SHORT(msd->sector)];
}



//
// P_LoadBlockMap
//
void P_LoadBlockMap (int lump)
{
    PRINTF("P_LoadBlockMap\n");

    int i;
    int count;
    int lumplen;

    lumplen = W_LumpLength(lump);
    count = lumplen / 2;
    
    // NRFD-NOTE: Replace W_ReadLump with cache
    // blockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    // W_ReadLump(lump, blockmaplump);
    // NRFD-TODO: Optimizations
    
    blockmaplump = W_CacheLumpNum(lump, PU_LEVEL);
    blockmap = blockmaplump + 4;

    // Swap all short integers to native byte ordering.
  
    /* NRFD-EXCLUDE: Don't need to do endian shift here
    for (i=0; i<count; i++)
    {
        blockmaplump[i] = SHORT(blockmaplump[i]);
    }
    */

    // Read the header
    uint32_t temp_blockmaplump[2]; 
    X_spi_read(blockmaplump, &temp_blockmaplump, 2); 
    
    bmaporgx = ((temp_blockmaplump[0] >> 0)  & 0xFFFF)<<FRACBITS;
    bmaporgy = ((temp_blockmaplump[0] >> 16)  & 0xFFFF)<<FRACBITS;
    bmapwidth = ((temp_blockmaplump[1] >> 0)  & 0xFFFF);
    bmapheight = ((temp_blockmaplump[1] >> 16)  & 0xFFFF);

    PRINTF("BlockMap: %d %d %d %d\n", ((temp_blockmaplump[0] >> 0)  & 0xFFFF), ((temp_blockmaplump[0] >> 16)  & 0xFFFF), bmapwidth, bmapheight);
        
    // Clear out mobj chains
    int block_count =  bmapwidth * bmapheight;

    // int blocklinks_size = sizeof(*blocklinks) * block_count;
    // blocklinks = Z_Malloc(blocklinks_size, PU_LEVEL, 0);
    // memset(blocklinks, 0, blocklinks_size);

    memset(blocklinks, 0, BLOCKLINKS_SIZE*sizeof(*blocklinks));
 }



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    PRINTF("P_GroupLines\n");

    int                 i;
    int                 j;
    line_t*             li;
    sector_t*           sector;
    subsector_t*        ss; 
    seg_t*              seg; // X-HEEP comment : seg is an adress in flash it must be read using X_spi_read
    fixed_t             bbox[4];
    int                 block;
        
    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
        seg = GetSeg(ss->firstline); //&segs[ss->firstline];
        ss->sector = SegFrontSector(seg); //seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    totallines = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
        sector_t *li_fs = LineFrontSector(li);
        sector_t *li_bs = LineBackSector(li);
        totallines++;
        li_fs->linecount++;

        if (li_bs && li_bs != li_fs)
        {
            li_bs->linecount++;
            totallines++;
        }
    }

    // build line tables for each sector 
    linebuffer = N_malloc(totallines*sizeof(line_t *));       
    //linebuffer = Z_Malloc (totallines*sizeof(line_t *), PU_LEVEL, 0);

    for (i=0; i<numsectors; ++i)
    {
        // Assign the line buffer for this sector

        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.

        sectors[i].linecount = 0;
    }

    // Assign lines to sectors
    for (i=0; i<numlines; ++i)
    { 
        li = &lines[i];

        sector_t *li_fs = LineFrontSector(li);
        sector_t *li_bs = LineBackSector(li);

        if (li_fs != NULL)
        {
            sector = li_fs;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }

        if (li_bs != NULL && li_fs != li_bs)
        {
            sector = li_bs;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }
    }
    
    // Generate bounding boxes for sectors
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        M_ClearBox (bbox);

        for (j=0 ; j<sector->linecount; j++)
        {
            vertex_t v1, v2;

            li = sector->lines[j];
            v1 = LineV1(li);
            v2 = LineV2(li);

            M_AddToBox (bbox, v1.x, v1.y);
            M_AddToBox (bbox, v2.x, v2.y);
        }

        // set the degenmobj_t to the middle of the bounding box
        // NRFD-TODO: sound
        // sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
        // sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
                
        // adjust bounding box to map blocks
        block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight-1 : block;
        sector->blockbox[BOXTOP]=block;

        block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM]=block;

        block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth-1 : block;
        sector->blockbox[BOXRIGHT]=block;

        block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT]=block;
    }
        
}

// Pad the REJECT lump with extra data when the lump is too small,
// to simulate a REJECT buffer overflow in Vanilla Doom.

static void PadRejectArray(byte *array, unsigned int len)
{
    unsigned int i;
    unsigned int byte_num;
    byte *dest;
    unsigned int padvalue;

    // Values to pad the REJECT array with:

    unsigned int rejectpad[4] =
    {
        0,                                    // Size
        0,                                    // Part of z_zone block header
        50,                                   // PU_LEVEL
        0x1d4a11                              // DOOM_CONST_ZONEID
    };

    rejectpad[0] = ((totallines * 4 + 3) & ~3) + 24;

    // Copy values from rejectpad into the destination array.

    dest = array;

    for (i=0; i<len && i<sizeof(rejectpad); ++i)
    {
        byte_num = i % 4;
        *dest = (rejectpad[i / 4] >> (byte_num * 8)) & 0xff;
        ++dest;
    }

    // We only have a limited pad size.  Print a warning if the
    // REJECT lump is too small.

    if (len > sizeof(rejectpad))
    {
        fprintf(stderr, "PadRejectArray: REJECT lump too short to pad! (%i > %i)\n",
                        len, (int) sizeof(rejectpad));

        // Pad remaining space with 0 (or 0xff, if specified on command line).

        if (M_CheckParm("-reject_pad_with_ff"))
        {
            padvalue = 0xff;
        }
        else
        {
            padvalue = 0xf00;
        }

        memset(array + sizeof(rejectpad), padvalue, len - sizeof(rejectpad));
    }
}

static void P_LoadReject(int lumpnum)
{
    PRINTF("P_LoadReject\n");

    rejectmatrix = W_CacheLumpNum(lumpnum, PU_LEVEL);
    
    /*  X-HEEP comment : avoid Z_Malloc of rejectmatrix
    int minlength;
    int lumplen;

    // Calculate the size that the REJECT lump *should* be.

    minlength = (numsectors * numsectors + 7) / 8;

    // If the lump meets the minimum length, it can be loaded directly.
    // Otherwise, we need to allocate a buffer of the correct size
    // and pad it with appropriate data.

    lumplen = W_LumpLength(lumpnum);

    if (lumplen >= minlength)
    {
        rejectmatrix = W_CacheLumpNum(lumpnum, PU_LEVEL);
    }
    else
    {
        rejectmatrix = Z_Malloc(minlength, PU_LEVEL, &rejectmatrix);
        W_ReadLump(lumpnum, rejectmatrix);

        PadRejectArray(rejectmatrix + lumplen, minlength - lumplen);
    }
    */
}

//X-HEEP comment : function to free malloc data from levels
void P_FreeLevelData()
{
    PRINTF("P_FreeLevelData\n");

    N_free(sectors, numsectors*sizeof(sector_t)); 
    N_free(sides ,numsides*sizeof(side_t)); 
    N_free(lines, numlines*sizeof(line_t));
    N_free(subsectors, numsubsectors*sizeof(subsector_t)); 
    N_free(linebuffer, totallines*sizeof(line_t *));  
}


//
// P_SetupLevel
//
void
P_SetupLevel
( int           episode,
  int           map,
  int           playermask,
  skill_t       skill)
{
    // map = 4;
    PRINTF("P_SetupLevel %d %d\n", episode, map);
    int         i;
    char        lumpname[9];
    int         lumpnum;

    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        players[i].killcount = players[i].secretcount 
            = players[i].itemcount = 0;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // Make sure all sounds are stopped before Z_FreeTags.
    //S_Start ();                 

    //Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1); //X-HEEP comment need to uncoment or replace if mallocs left 

    // UNUSED W_Profile ();
    P_InitThinkers ();

    // if working with a devlopment map, reload it
    //W_Reload (); //useless 

    // find map name
    if ( gamemode == commercial)
    {
        if (map<10)
            DEH_snprintf(lumpname, 9, "map0%i", map);
        else
            DEH_snprintf(lumpname, 9, "map%i", map);
    }
    else
    {
        lumpname[0] = 'E';
        lumpname[1] = '0' + episode;
        lumpname[2] = 'M';
        lumpname[3] = '0' + map;
        lumpname[4] = 0;
    }


    lumpnum = W_GetNumForName (lumpname);
        
    leveltime = 0;

    PRINTF("In P_SetupLevel before loads\n");
    // note: most of this ordering is important 
    P_LoadBlockMap (lumpnum+ML_BLOCKMAP);    //X-HEEP comment : No more Z_malloc
    P_LoadVertexes (lumpnum+ML_VERTEXES);    //X-HEEP comment : No more Z_malloc : in flash
    P_LoadSectors (lumpnum+ML_SECTORS);      //X-HEEP comment : Uses N_malloc: sectors in heap    
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS);    //X-HEEP comment : Uses N_malloc: sides in heap
    P_LoadLineDefs (lumpnum+ML_LINEDEFS);    //X-HEEP comment : Uses N_malloc: lines in heap
    P_LoadSubsectors (lumpnum+ML_SSECTORS);  //X-HEEP comment : Uses N_malloc: subsectors in heap
    P_LoadNodes (lumpnum+ML_NODES);          //X-HEEP comment : No more Z_malloc : in flash
    P_LoadSegs (lumpnum+ML_SEGS);            //X-HEEP comment : No more Z_malloc : in flash
    P_GroupLines ();                         //X-HEEP comment : Uses N_malloc: linebuffer in heap
    P_LoadReject (lumpnum+ML_REJECT);        //X-HEEP comment : No more Z_malloc : in flash


    PRINTF("In P_SetupLevel before loadthings\n");
    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_LoadThings (lumpnum+ML_THINGS);
    
    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
            if (playeringame[i])
            {
                players[i].mo = NULL;
                G_DeathMatchSpawnPlayer (i);
            }
                        
    }

    // clear special respawning que
    iquehead = iquetail = 0;            
    
    PRINTF("In P_SetupLevel before SpawnSpecials\n");
    // set up world state
    P_SpawnSpecials ();
        
    // build subsector connect matrix
    //  UNUSED P_ConnectSubsectors ();

    // preload graphics
    if (precache)
        R_PrecacheLevel ();

    //PRINTF ("free memory: 0x%x\n", Z_FreeMemory());

}



//
// P_Init
//
void P_Init (void)
{ 
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites ();
}

