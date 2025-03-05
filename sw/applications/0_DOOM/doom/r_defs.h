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
//      Refresh/rendering module, shared data struct definitions.
//


#ifndef __R_DEFS__
#define __R_DEFS__


// Screenwidth.
#include "doomdef.h"

// Some more or less basic data types
// we depend on.
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"
// SECTORS do store MObjs anyway.
#include "p_mobj.h"

#include "i_video.h"

#include "v_patch.h"

#include "i_swap.h"



// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE        0
#define SIL_BOTTOM      1
#define SIL_TOP         2
#define SIL_BOTH        3

#define MAXDRAWSEGS     128 //256 // NRFD-TODO





//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
//  like some DOOM-alikes ("wt", "WebView") did.
//
typedef struct
{
    fixed_t x;
    fixed_t y;
    
} vertex_t;


typedef struct
{
    fixed_t dx;
    fixed_t dy;
    
} vector_t;

// Forward of LineDefs, for Sectors.
struct line_s;

// Each sector has a degenmobj_t in its center
//  for sound origin purposes.
// I suppose this does not handle sound from
//  moving objects (doppler), because
//  position is prolly just buffered, not
//  updated.
typedef struct
{
    thinker_t       thinker;    // not used for anything
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;

} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct  __attribute__((packed))
{
    fixed_t floorheight;
    fixed_t ceilingheight;
    short   floorpic;
    short   ceilingpic;
    short   lightlevel;
    short   special;
    short   tag;

    // 0 = untraversed, 1,2 = sndlines -1
    short     soundtraversed; //NRFD-NOTE: changed from int to short

    // thing that made a sound (or null)
    mobj_t* soundtarget;

    // mapblock bounding box for height changes
    short       blockbox[4];  //NRFD-NOTE: changed from int to short

    // origin for any sounds played by the sector
    // degenmobj_t soundorg;

    // if == validcount, already checked
    uint8_t     validcount;  //NRFD-NOTE: changed from int to uint8_t

    // list of mobjs in sector
    mobj_t* thinglist;

    // thinker_t for reversable actions
    void*   specialdata;

    short         linecount;  //NRFD-NOTE: changed from int to short
    struct line_s** lines;  // [linecount] size
    
} sector_t;

degenmobj_t *SectorSoundOrg(sector_t *sec);


//
// The SideDef.
//

typedef struct  __attribute__((packed))
{
    // add this to the calculated texture column
    short textureoffset_short; // NRFD-NOTE: Was fixed_t
    
    // add this to the calculated texture top
    short rowoffset_short; // NRFD-NOTE: Was fixed_t

    // Texture indices.
    // We do not maintain names here. 
    // NRFD-TODO: Support more than 256 textures
    byte   toptexture;
    byte   bottomtexture;
    byte   midtexture;

    // Sector the SideDef is facing.
    // sector_t*   sector;
    uint8_t sector_num;
    
} side_t;

fixed_t R_SideTextureOffset(side_t *side);
fixed_t R_SideRowOffset(side_t *side);
sector_t *SideSector(side_t *side);
sector_t *SideNumSector(int sidenum);

//
// Move clipping aid for LineDefs.
//
typedef enum
{
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE

} slopetype_t;



typedef struct  __attribute__((packed)) line_s
{
    maplinedef_t* mld;
    // Vertices, from v1 to v2.

    // vertex_t*   v1_x;
    // vertex_t*   v2_x;

    // Precalculated v2 - v1 for side checking.
    // fixed_t dx;
    // fixed_t dy;

    // Animation related.
    // short   flags;
    byte   special; // NRFD-NOTE: Was short
    // short   tag;

    // Visual appearance: SideDefs.
    //  sidenum[1] will be -1 if one sided
    // short   sidenum[2];         

    // Neat. Another bounding box, for the extent
    //  of the LineDef.
    // fixed_t bbox[4];

    // To aid move clipping.
    // slopetype_t slopetype;

    // Front and back sector.
    // Note: redundant? Can be retrieved from SideDefs.
    // sector_t*   frontsector;
    // sector_t*   backsector;

    // if == validcount, already checked
    // NRFD-TODO: Optimize?
    int8_t     validcount; // NRFD-NOTE: Was int

    // NRFD-TODO?
    // thinker_t for reversable actions
    // void*   specialdata;        
} line_t;

vertex_t    LineV1          (line_t *line);
vertex_t    LineV2          (line_t *line);
void        LineSetMapped   (line_t *line);
boolean     LineIsMapped    (line_t *line);
boolean     LineDontDraw    (line_t *line);
short       LineFlags       (line_t *line);
short       LineTag         (line_t *line);
void        LineTagSet666   (line_t *line);
short       LineSideNum     (line_t *line, int num);
side_t*     LineSide        (line_t *line, int num);
sector_t *  LineFrontSector (line_t *line);
sector_t *  LineBackSector  (line_t *line);
short       LineTag         (line_t *line);
slopetype_t LineSlopeType   (line_t *line);
vector_t    LineVector      (line_t* line);
fixed_t*    LineBBox        (line_t* line);
//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//
typedef struct  __attribute__((packed)) subsector_s
{
    sector_t*   sector;
    short   numlines;
    short   firstline;
    
} subsector_t;



//
// The LineSeg.
//
/*
typedef struct  __attribute__((packed))
{
    // vertex_t*   v1;
    // vertex_t*   v2;
    
    // fixed_t offset;

    // angle_t angle;

    // side_t* sidedef;
    // line_t* linedef;

    // Sector references.
    // Could be retrieved from linedef, too.
    // backsector is NULL for one sided lines
    // sector_t*   frontsector;
    // sector_t*   backsector;
} seg_t;*/
typedef mapseg_t seg_t;

seg_t *GetSeg(int num);
sector_t *SegFrontSector(seg_t *seg);
sector_t *SegBackSector(seg_t *seg); 
angle_t SegAngle(seg_t *seg);
fixed_t SegOffset(seg_t *seg);
vertex_t *SegV1(seg_t *seg);
vertex_t *SegV2(seg_t *seg);
line_t *SegLineDef(seg_t *seg);
side_t *SegSideDef(seg_t *seg);



//
// BSP node.
//

typedef struct  __attribute__((packed))
{
    // Partition line.
    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;

    // Bounding box for each child.
    fixed_t bbox[2][4];

    // If NF_SUBSECTOR its a subsector.
    unsigned short children[2];
    
} node_t;

extern mapnode_t*          mapnodes;

static inline node_t GetNode(unsigned short num)
{
    int         j;
    int         k;
    mapnode_t *mn = &mapnodes[num];
    node_t     no;

    no.x = SHORT(mn->x)<<FRACBITS;
    no.y = SHORT(mn->y)<<FRACBITS;
    no.dx = SHORT(mn->dx)<<FRACBITS;
    no.dy = SHORT(mn->dy)<<FRACBITS;
    for (j=0 ; j<2 ; j++)
    {
        no.children[j] = SHORT(mn->children[j]);
        for (k=0 ; k<4 ; k++) {
            no.bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }
    return no;
}

// PC direct to screen pointers
//B UNUSED - keep till detailshift in r_draw.c resolved
//extern byte*  destview;
//extern byte*  destscreen;





//
// OTHER TYPES
//

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even us emore than 32 levels.
typedef pixel_t     lighttable_t;




//
// ?
//
typedef struct  __attribute__((packed)) drawseg_s
{
    seg_t*      curline;
    short       x1; // NRFD-NOTE: Was int
    short       x2; // NRFD-NOTE: Was int

    fixed_t     scale1;
    fixed_t     scale2;
    fixed_t     scalestep;

    // 0=none, 1=bottom, 2=top, 3=both
    uint8_t     silhouette; // NRFD-NOTE: was int

    // do not clip sprites above this
    fixed_t     bsilheight;

    // do not clip sprites below this
    fixed_t     tsilheight;
    
    // Pointers to lists for sprite clipping,
    //  all three adjusted so [x1] is first value.
    short*      sprtopclip;     
    short*      sprbottomclip;  
    short*      maskedtexturecol;
    
} drawseg_t;



// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct  __attribute__((packed)) vissprite_s
{
    // Doubly linked list.
    // NRFD-TODO: XOR linked list possible?
    // struct vissprite_s* prev;
    struct vissprite_s* next;
    
    short         x1; // NRFD-NOTE: Was int
    short         x2; // NRFD-NOTE: Was int

    mobj_t  *thing;
    /*
    // for line side calculation
    fixed_t     gx;
    fixed_t     gy;     

    // global bottom / top for silhouette clipping
    fixed_t     gz;
    fixed_t     gzt;
    */

    // horizontal position of x1
    fixed_t     startfrac;
    
    fixed_t     scale;
    
    // negative if flipped
    fixed_t     xiscale;

    fixed_t     texturemid;
    short         patch; // a lump number, NRFD-NOTE: was int

    // for color translation and shadow draw,
    //  maxbright frames as well
    lighttable_t*   colormap;
   
    // int         mobjflags; // NRFD-TODO?
    
} vissprite_t;


//  
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct  __attribute__((packed))
{
    // If false use 0 for any position.
    // Note: as eight entries are available,
    //  we might as well insert the same name eight times.
    boolean rotate;

    // Lump to use for view angles 0-7.
    short   lump[8];

    // Flip bit (1 = flip) to use for view angles 0-7.
    // NRFD-NOTE: Optimized for memory, used to be: byte flip[8];
    byte    flip;
    
} spriteframe_t;


boolean R_SpriteGetFlip(spriteframe_t *sprite, int num);


//
// A sprite definition:
//  a number of animation frames.
//
typedef struct  __attribute__((packed))
{
    short         numframes;
    spriteframe_t*  spriteframes;

} spritedef_t;



//
// Now what is a visplane, anyway?
// 
typedef struct  __attribute__((packed))
{
  fixed_t       height;

  // NRFD-NOTO: was int
  short           picnum;
  short           lightlevel;
  short           minx;
  short           maxx;
  
  // leave pads for [minx-1]/[maxx+1]
  
  byte      pad1;
  // Here lies the rub for all
  //  dynamic resize/change of resolution.
  byte      top[SCREENWIDTH];
  byte      pad2;
  byte      pad3;
  // See above.
  byte      bottom[SCREENWIDTH];
  byte      pad4;

} visplane_t;




#endif
