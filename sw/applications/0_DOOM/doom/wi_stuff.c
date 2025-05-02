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
//      Intermission screens.
//


#include <stdio.h>

#include "z_zone.h"

#include "m_misc.h"
#include "m_random.h"

#include "deh_doomTop.h"
#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "g_game.h"

#include "r_local.h"
//#include "s_sound.h"

#include "doomstat.h"

// Data.
//#include "sounds.h"

// Needs access to LFB.
#include "v_video.h"

#include "wi_stuff.h"
#include "x_spi.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES     4
#define NUMMAPS         9


// in tics
//U #define PAUSELEN            (TICRATE*2) 
//U #define SCORESTEP           100
//U #define ANIMPERIOD          32
// pixel distance from "(YOU)" to "PLAYER N"
//U #define STARDIST            10 
//U #define WK 1


// GLOBAL LOCATIONS
#define WI_TITLEY               2
#define WI_SPACINGY             33

// SINGPLE-PLAYER STUFF
#define SP_STATSX               50
#define SP_STATSY               50

#define SP_TIMEX                16
#define SP_TIMEY                (SCREENHEIGHT-32)


// NET GAME STUFF
#define NG_STATSY               50
#define NG_STATSX               (32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX             64


// DEATHMATCH STUFF
#define DM_MATRIXX              42
#define DM_MATRIXY              68

#define DM_SPACINGX             40

#define DM_TOTALSX              269

#define DM_KILLERSX             10
#define DM_KILLERSY             100
#define DM_VICTIMSX             5
#define DM_VICTIMSY             50




typedef enum
{
    ANIM_ALWAYS,
    ANIM_RANDOM,
    ANIM_LEVEL

} animenum_t;

typedef struct
{
    int         x;
    int         y;
    
} point_t;


//
// Animation.
// There is another anim_t used in p_spec.
//
// NRFD-TODO: Split into constant data and state
// NRFD-NOTE: Changed int to short below
typedef struct
{
    animenum_t  type;

    // period in tics between animations
    int       period;

    // number of animation frames
    int         nanims;

    // location of animation
    point_t     loc;

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int         data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int         data2; 

    // // actual graphics for frames of animations
    // patch_t*    p[3]; 

    // // following must be initialized to zero before use!

    // // next value of bcnt (used in conjunction with period)
    // int         nexttic;

    // // last drawn animation frame
    // int       lastdrawn;

    // // next frame number to animate
    // int       ctr;
    
    // // used by RANDOM and LEVEL when animating
    // int         state;  

} anim_t;

typedef struct  __attribute__((packed))
{
    // actual graphics for frames of animations
    patch_t*    p[3]; 

    // following must be initialized to zero before use!

    // next value of bcnt (used in conjunction with period)
    int         nexttic;

    // last drawn animation frame
    int8_t       lastdrawn;

    // next frame number to animate
    int8_t       ctr;
    
    // used by RANDOM and LEVEL when animating
    int8_t         state;  

} anim_state_t;


const point_t lnodes[NUMEPISODES][NUMMAPS] =
{
    // Episode 0 World Map
    {
        { 185, 164 },   // location of level 0 (CJ)
        { 148, 143 },   // location of level 1 (CJ)
        { 69, 122 },    // location of level 2 (CJ)
        { 209, 102 },   // location of level 3 (CJ)
        { 116, 89 },    // location of level 4 (CJ)
        { 166, 55 },    // location of level 5 (CJ)
        { 71, 56 },     // location of level 6 (CJ)
        { 135, 29 },    // location of level 7 (CJ)
        { 71, 24 }      // location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    {
        { 254, 25 },    // location of level 0 (CJ)
        { 97, 50 },     // location of level 1 (CJ)
        { 188, 64 },    // location of level 2 (CJ)
        { 128, 78 },    // location of level 3 (CJ)
        { 214, 92 },    // location of level 4 (CJ)
        { 133, 130 },   // location of level 5 (CJ)
        { 208, 136 },   // location of level 6 (CJ)
        { 148, 140 },   // location of level 7 (CJ)
        { 235, 158 }    // location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    {
        { 156, 168 },   // location of level 0 (CJ)
        { 48, 154 },    // location of level 1 (CJ)
        { 174, 95 },    // location of level 2 (CJ)
        { 265, 75 },    // location of level 3 (CJ)
        { 130, 48 },    // location of level 4 (CJ)
        { 279, 23 },    // location of level 5 (CJ)
        { 198, 48 },    // location of level 6 (CJ)
        { 140, 25 },    // location of level 7 (CJ)
        { 281, 136 }    // location of level 8 (CJ)
    }

};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//

/*
// #define ANIM(type, period, nanims, x, y, nexttic)            \
//    { (type), (period), (nanims), { (x), (y) }, (nexttic),    \
//      0, { NULL, NULL, NULL }, 0, 0, 0, 0 }
*/

#define ANIM(type, period, nanims, x, y, nexttic)            \
   { (type), (period), (nanims), { (x), (y) }, (nexttic), 0 }

const anim_t epsd0animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 224, 104, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 184, 160, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 112, 136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 72, 112, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 88, 96, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64, 48, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 192, 40, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 136, 16, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 80, 16, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 64, 24, 0),
};

const anim_t epsd1animinfo[] =
{
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 1),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 2),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 3),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 4),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 5),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 6),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 7),
    ANIM(ANIM_LEVEL, TICRATE/3, 3, 192, 144, 8),
    ANIM(ANIM_LEVEL, TICRATE/3, 1, 128, 136, 8),
};

const anim_t epsd2animinfo[] =
{
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 168, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 40, 136, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 160, 96, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 104, 80, 0),
    ANIM(ANIM_ALWAYS, TICRATE/3, 3, 120, 32, 0),
    ANIM(ANIM_ALWAYS, TICRATE/4, 3, 40, 0, 0),
};

const int NUMANIMS[NUMEPISODES] =
{
    arrlen(epsd0animinfo),
    arrlen(epsd1animinfo),
    arrlen(epsd2animinfo),
};

const anim_t* const anims[NUMEPISODES] =
{
    epsd0animinfo,
    epsd1animinfo,
    epsd2animinfo
};

anim_state_t anim_states[10];


//
// GENERAL DATA
//

//
// Locally used stuff.
//

// States for single-player
#define SP_KILLS                0
#define SP_ITEMS                2
#define SP_SECRET               4
#define SP_FRAGS                6 
#define SP_TIME                 8 
#define SP_PAR                  ST_TIME

#define SP_PAUSE                1

// in seconds
#define SHOWNEXTLOCDELAY        4
//#define SHOWLASTLOCDELAY      SHOWNEXTLOCDELAY


// used to accelerate or skip a stage
static int              acceleratestage;

// wbs->pnum
static uint8_t         me;

 // specifies current state
static stateenum_t      state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static uint8_t             cnt;  

// used for timing of background animation
static uint8_t              bcnt;

// signals to refresh everything for one frame
static int              firstrefresh; 

static int              cnt_kills[MAXPLAYERS];
static int              cnt_items[MAXPLAYERS];
static int              cnt_secret[MAXPLAYERS];
static int              cnt_time;
static int              cnt_par;
static int              cnt_pause;

// # of commercial levels
static int              NUMCMAPS; 


//
//      GRAPHICS
//

// You Are Here graphic
static patch_t*         yah[3] = { NULL, NULL, NULL }; // X-HEEP comment : The elements of yah are adresses in flash they must be read using X_spi_read

// splat
static patch_t*         splat[2] = { NULL, NULL }; //X-HEEP comment : The elements of splat are adresses in flash they must be read using X_spi_read

// %, : graphics
static patch_t*         percent; // X-HEEP comment : percent is an adress in flash it must be read using X_spi_read
static patch_t*         colon; // X-HEEP comment : colon is an adress in flash it must be read using X_spi_read

// 0-9 graphic
static patch_t*         num[10]; //X-HEEP comment : The elements of num are adresses in flash they must be read using X_spi_read

// minus sign
static patch_t*         wiminus; // X-HEEP comment : wiminus is an adress in flash it must be read using X_spi_read

// "Finished!" graphics
static patch_t*         finished; // X-HEEP comment : finished is an adress in flash it must be read using X_spi_read

// "Entering" graphic
static patch_t*         entering; // X-HEEP comment : entering is an adress in flash it must be read using X_spi_read

// "secret"
static patch_t*         sp_secret; // X-HEEP comment : sp_secret is an adress in flash it must be read using X_spi_read

 // "Kills", "Scrt", "Items", "Frags"
static patch_t*         kills;   // X-HEEP comment : kills is an adress in flash it must be read using X_spi_read
static patch_t*         secret;  // X-HEEP comment : secret is an adress in flash it must be read using X_spi_read
static patch_t*         items;   // X-HEEP comment : items is an adress in flash it must be read using X_spi_read
static patch_t*         frags;   // X-HEEP comment : frags is an adress in flash it must be read using X_spi_read

// Time sucks.
static patch_t*         timepatch; // X-HEEP comment : timepatch is an adress in flash it must be read using X_spi_read
static patch_t*         par;       // X-HEEP comment : par is an adress in flash it must be read using X_spi_read
static patch_t*         sucks;     // X-HEEP comment : sucks is an adress in flash it must be read using X_spi_read

// "killers", "victims"
static patch_t*         killers; // X-HEEP comment : killers is an adress in flash it must be read using X_spi_read
static patch_t*         victims; // X-HEEP comment : victims is an adress in flash it must be read using X_spi_read

// "Total", your face, your dead face
static patch_t*         total;  // X-HEEP comment : star is an adress in flash it must be read using X_spi_read
static patch_t*         star;   // X-HEEP comment : star is an adress in flash it must be read using X_spi_read
static patch_t*         bstar;  // X-HEEP comment : bstar is an adress in flash it must be read using X_spi_read

// "red P[1..MAXPLAYERS]"
static patch_t*         p[MAXPLAYERS]; //X-HEEP comment : The elements of p are adresses in flash they must be read using X_spi_read

// "gray P[1..MAXPLAYERS]"
static patch_t*         bp[MAXPLAYERS]; //X-HEEP comment : The elements of bp are adresses in flash they must be read using X_spi_read

 // Name graphics of each level (centered)
static patch_t**        lnames;

// Buffer storing the backdrop
static patch_t *background; // X-HEEP comment : background is an adress in flash it must be read using X_spi_read

//
// CODE
//

// slam background
void WI_slamBackground(void)
{
    V_DrawPatch(0, 0, background);
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}


// Draws "<Levelname> Finished!"
void WI_drawLF(void)
{
    int y = WI_TITLEY;

    if (gamemode != commercial || wbs->last < NUMCMAPS)
    {
        // draw <LevelName> 
        V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->last]->width))/2,
                    y, lnames[wbs->last]);

        // draw "Finished!"
        y += (5*SHORT(lnames[wbs->last]->height))/4;
        patch_t temp_finished; 
        X_spi_read(finished, &temp_finished, sizeof(temp_finished)/4); 
        V_DrawPatch((SCREENWIDTH - SHORT(temp_finished.width)) / 2, y, finished);
    }
    else if (wbs->last == NUMCMAPS)
    {
        // MAP33 - nothing is displayed!
    }
    else if (wbs->last > NUMCMAPS)
    {
        // > MAP33.  Doom bombs out here with a Bad V_DrawPatch error.
        // I'm pretty sure that doom2.exe is just reading into random
        // bits of memory at this point, but let's try to be accurate
        // anyway.  This deliberately triggers a V_DrawPatch error.

        patch_t tmp = { SCREENWIDTH, SCREENHEIGHT, 1, 1, 
                        { 0, 0, 0, 0, 0, 0, 0, 0 } };

        V_DrawPatch(0, y, &tmp);
    }
}



// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // draw "Entering"
    patch_t temp_entering; 
    X_spi_read (entering, &temp_entering, sizeof(temp_entering)/4); 
    V_DrawPatch((SCREENWIDTH - SHORT(temp_entering.width))/2,
                y,
                entering);

    // draw level
    y += (5*SHORT(lnames[wbs->next]->height))/4;

    V_DrawPatch((SCREENWIDTH - SHORT(lnames[wbs->next]->width))/2,
                y, 
                lnames[wbs->next]);

}

void
WI_drawOnLnode
( int           n,
  patch_t*      c[] )
{

    int         i;
    int         left;
    int         top;
    int         right;
    int         bottom;
    boolean     fits = false;

    i = 0;
    do
    {
        patch_t c_temp;
        X_spi_read(c[i], &c_temp, sizeof(c_temp)/4); 
        left = lnodes[wbs->epsd][n].x - SHORT(c_temp.leftoffset);
        top = lnodes[wbs->epsd][n].y - SHORT(c_temp.topoffset);
        right = left + SHORT(c_temp.width);
        bottom = top + SHORT(c_temp.height);

        if (left >= 0
            && right < SCREENWIDTH
            && top >= 0
            && bottom < SCREENHEIGHT)
        {
            fits = true;
        }
        else
        {
            i++;
        }
    } while (!fits && i!=2 && c[i] != NULL);

    if (fits && i<2)
    {
        V_DrawPatch(lnodes[wbs->epsd][n].x,
                    lnodes[wbs->epsd][n].y,
                    c[i]);
    }
    else
    {
        // DEBUG
        PRINTF("Could not place patch on level %d", n+1); 
    }
}



void WI_initAnimatedBack(void)
{
    int            i,j;
    const anim_t*  a;
    anim_state_t*  as;
    char name[9];

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (j=0;j<NUMANIMS[wbs->epsd];j++)
    {
        a = &anims[wbs->epsd][j];
        as = &anim_states[j];

        // init variables
        as->ctr = -1;

        // specify the next time to draw it
        if (a->type == ANIM_ALWAYS)
            as->nexttic = bcnt + 1 + (M_Random()%a->period);
        else if (a->type == ANIM_RANDOM)
            as->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
        else if (a->type == ANIM_LEVEL)
            as->nexttic = bcnt + 1;


        // NRFD-NOTE: Moved here from loadUnloadData
        for (i=0;i<a->nanims;i++)
        {
            // MONDO HACK! // NRFD-TODO?
            // if (wbs->epsd != 1 || j != 8)
            // {
                // animations
                //DEH_snprintf(name, 9, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                as->p[i] = W_CacheLumpName(name, PU_STATIC);
            // }
            // else
            // {
            //     // HACK ALERT!
            //     a->p[i] = anims[1][4].p[i];
            // }
        }
    }
}

void WI_updateAnimatedBack(void)
{
    int            i;
    const anim_t*  a;
    anim_state_t*  as;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
        a = &anims[wbs->epsd][i];
        as = &anim_states[i];

        if (bcnt == as->nexttic)
        {
            switch (a->type)
            {
              case ANIM_ALWAYS:
                if (++as->ctr >= a->nanims) as->ctr = 0;
                as->nexttic = bcnt + a->period;
                break;

              case ANIM_RANDOM:
                as->ctr++;
                if (as->ctr == a->nanims)
                {
                    as->ctr = -1;
                    as->nexttic = bcnt+a->data2+(M_Random()%a->data1);
                }
                else as->nexttic = bcnt + a->period;
                break;
                
              case ANIM_LEVEL:
                // gawd-awful hack for level anims
                if (!(state == StatCount && i == 7)
                    && wbs->next == a->data1)
                {
                    as->ctr++;
                    if (as->ctr == a->nanims) as->ctr--;
                    as->nexttic = bcnt + a->period;
                }
                break;
            }
        }
    }
}

void WI_drawAnimatedBack(void)
{
    int                  i;
    const anim_t*        a;
    anim_state_t*        as;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (i=0 ; i<NUMANIMS[wbs->epsd] ; i++)
    {
        a = &anims[wbs->epsd][i];
        as = &anim_states[i];

        if (as->ctr >= 0)
            V_DrawPatch(a->loc.x, a->loc.y, as->p[as->ctr]);
    }
}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int
WI_drawNum
( int           x,
  int           y,
  int           n,
  int           digits )
{
    patch_t temp_num; 
    X_spi_read(num[0], &temp_num, sizeof(temp_num)/4); 
    int         fontwidth = SHORT(temp_num.width);
    int         neg;
    int         temp;

    if (digits < 0)
    {
        if (!n)
        {
            // make variable-length zeros 1 digit long
            digits = 1;
        }
        else
        {
            // figure out # of digits in #
            digits = 0;
            temp = n;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    neg = n < 0;
    if (neg)
        n = -n;

    // if non-number, do not draw it
    if (n == 1994)
        return 0;

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        V_DrawPatch(x, y, num[ n % 10 ]);
        n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawPatch(x-=8, y, wiminus);

    return x;

}

void
WI_drawPercent
( int           x,
  int           y,
  int           p )
{
    if (p < 0)
        return;

    V_DrawPatch(x, y, percent);
    WI_drawNum(x, y, p, -1);
}



//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
void
WI_drawTime
( int           x,
  int           y,
  int           t )
{

    int         div;
    int         n;

    if (t<0)
        return;

    if (t <= 61*59)
    {
        div = 1;

        do
        {
            n = (t / div) % 60;
            patch_t temp_colon; 
            X_spi_read(colon, &temp_colon, sizeof(temp_colon)/4); 
            x = WI_drawNum(x, y, n, 2) - SHORT(temp_colon.width);
            div *= 60;

            // draw
            if (div==60 || t / div)
                V_DrawPatch(x, y, colon);
            
        } while (t / div);
    }
    else
    {
        // "sucks"
        patch_t temp_sucks; 
        X_spi_read(sucks, &temp_sucks, sizeof(temp_sucks)/4); 
        V_DrawPatch(x - SHORT(temp_sucks.width), y, sucks); 
    }
}


void WI_End(void)
{
    void WI_unloadData(void);
    WI_unloadData();
}

void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}

void WI_updateNoState(void) {

    WI_updateAnimatedBack();

    if (!--cnt)
    {
        // Don't call WI_End yet.  G_WorldDone doesnt immediately 
        // change gamestate, so WI_Drawer is still going to get
        // run until that happens.  If we do that after WI_End
        // (which unloads all the graphics), we're in trouble.
        //WI_End();
        G_WorldDone();
    }

}

static boolean          snl_pointeron = false;


void WI_initShowNextLoc(void)
{
    state = ShowNextLoc;
    acceleratestage = 0;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    WI_initAnimatedBack();
}

void WI_updateShowNextLoc(void)
{
    WI_updateAnimatedBack();

    if (!--cnt || acceleratestage)
        WI_initNoState();
    else
        snl_pointeron = (cnt & 31) < 20;
}

void WI_drawShowNextLoc(void)
{

    int         i;
    int         last;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    if ( gamemode != commercial)
    {
        if (wbs->epsd > 2)
        {
            WI_drawEL();
            return;
        }
        
        last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

        // draw a splat on taken cities.
        for (i=0 ; i<=last ; i++)
            WI_drawOnLnode(i, splat);

        // splat the secret level?
        if (wbs->didsecret)
            WI_drawOnLnode(8, splat);

        // draw flashing ptr
        if (snl_pointeron)
            WI_drawOnLnode(wbs->next, yah); 
    }

    // draws which level you are entering..
    if ( (gamemode != commercial)
         || wbs->next != 30)
        WI_drawEL();  

}

void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}

int WI_fragSum(int playernum)
{
    int         i;
    int         frags = 0;
    
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i]
            && i!=playernum)
        {
            frags += plrs[playernum].frags[i];
        }
    }

        
    // JDC hack - negative frags.
    frags -= plrs[playernum].frags[playernum];
    // UNUSED if (frags < 0)
    //  frags = 0;

    return frags;
}


/* NRFD-TODO: multiplayer
static int              dm_state;
static int              dm_frags[MAXPLAYERS][MAXPLAYERS];
static int              dm_totals[MAXPLAYERS];



void WI_initDeathmatchStats(void)
{

    int         i;
    int         j;

    state = StatCount;
    acceleratestage = 0;
    dm_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            for (j=0 ; j<MAXPLAYERS ; j++)
                if (playeringame[j])
                    dm_frags[i][j] = 0;

            dm_totals[i] = 0;
        }
    }
    
    WI_initAnimatedBack();
}



void WI_updateDeathmatchStats(void)
{

    int         i;
    int         j;
    
    boolean     stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && dm_state != 4)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
            {
                for (j=0 ; j<MAXPLAYERS ; j++)
                    if (playeringame[j])
                        dm_frags[i][j] = plrs[i].frags[j];

                dm_totals[i] = WI_fragSum(i);
            }
        }
        

        //S_StartSound(0, sfx_barexp);
        dm_state = 4;
    }

    
    if (dm_state == 2)
    {
        if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);
        
        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (playeringame[i])
            {
                for (j=0 ; j<MAXPLAYERS ; j++)
                {
                    if (playeringame[j]
                        && dm_frags[i][j] != plrs[i].frags[j])
                    {
                        if (plrs[i].frags[j] < 0)
                            dm_frags[i][j]--;
                        else
                            dm_frags[i][j]++;

                        if (dm_frags[i][j] > 99)
                            dm_frags[i][j] = 99;

                        if (dm_frags[i][j] < -99)
                            dm_frags[i][j] = -99;
                        
                        stillticking = true;
                    }
                }
                dm_totals[i] = WI_fragSum(i);

                if (dm_totals[i] > 99)
                    dm_totals[i] = 99;
                
                if (dm_totals[i] < -99)
                    dm_totals[i] = -99;
            }
            
        }
        if (!stillticking)
        {
            //S_StartSound(0, sfx_barexp);
            dm_state++;
        }

    }
    else if (dm_state == 4)
    {
        if (acceleratestage)
        {
            //S_StartSound(0, sfx_slop);

            if ( gamemode == commercial)
                WI_initNoState();
            else
                WI_initShowNextLoc();
        }
    }
    else if (dm_state & 1)
    {
        if (!--cnt_pause)
        {
            dm_state++;
            cnt_pause = TICRATE;
        }
    }
}



void WI_drawDeathmatchStats(void)
{

    int         i;
    int         j;
    int         x;
    int         y;
    int         w;

    WI_slamBackground();
    
    // draw animated background
    WI_drawAnimatedBack(); 
    WI_drawLF();

    // draw stat titles (top line)
    V_DrawPatch(DM_TOTALSX-SHORT(total->width)/2,
                DM_MATRIXY-WI_SPACINGY+10,
                total);
    
    V_DrawPatch(DM_KILLERSX, DM_KILLERSY, killers);
    V_DrawPatch(DM_VICTIMSX, DM_VICTIMSY, victims);

    // draw P?
    x = DM_MATRIXX + DM_SPACINGX;
    y = DM_MATRIXY;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (playeringame[i])
        {
            V_DrawPatch(x-SHORT(p[i]->width)/2,
                        DM_MATRIXY - WI_SPACINGY,
                        p[i]);
            
            V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)/2,
                        y,
                        p[i]);

            if (i == me)
            {
                V_DrawPatch(x-SHORT(p[i]->width)/2,
                            DM_MATRIXY - WI_SPACINGY,
                            bstar);

                V_DrawPatch(DM_MATRIXX-SHORT(p[i]->width)/2,
                            y,
                            star);
            }
        }
        else
        {
            // V_DrawPatch(x-SHORT(bp[i]->width)/2,
            //   DM_MATRIXY - WI_SPACINGY, bp[i]);
            // V_DrawPatch(DM_MATRIXX-SHORT(bp[i]->width)/2,
            //   y, bp[i]);
        }
        x += DM_SPACINGX;
        y += WI_SPACINGY;
    }

    // draw stats
    y = DM_MATRIXY+10;
    w = SHORT(num[0]->width);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        x = DM_MATRIXX + DM_SPACINGX;

        if (playeringame[i])
        {
            for (j=0 ; j<MAXPLAYERS ; j++)
            {
                if (playeringame[j])
                    WI_drawNum(x+w, y, dm_frags[i][j], 2);

                x += DM_SPACINGX;
            }
            WI_drawNum(DM_TOTALSX+w, y, dm_totals[i], 2);
        }
        y += WI_SPACINGY;
    }
}

static int      cnt_frags[MAXPLAYERS];
static int      dofrags;
static int      ng_state;

void WI_initNetgameStats(void)
{

    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

        cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

        dofrags += WI_fragSum(i);
    }

    dofrags = !!dofrags;

    WI_initAnimatedBack();
}



void WI_updateNetgameStats(void)
{

    int         i;
    int         fsum;
    
    boolean     stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && ng_state != 10)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;

            if (dofrags)
                cnt_frags[i] = WI_fragSum(i);
        }
        //S_StartSound(0, sfx_barexp);
        ng_state = 10;
    }

    if (ng_state == 2)
    {
        if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_kills[i] += 2;

            if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
                cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
            else
                stillticking = true;
        }
        
        if (!stillticking)
        {
            //S_StartSound(0, sfx_barexp);
            ng_state++;
        }
    }
    else if (ng_state == 4)
    {
        if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_items[i] += 2;
            if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
                cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
            else
                stillticking = true;
        }
        if (!stillticking)
        {
            //S_StartSound(0, sfx_barexp);
            ng_state++;
        }
    }
    else if (ng_state == 6)
    {
        if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_secret[i] += 2;

            if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
                cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
            else
                stillticking = true;
        }
        
        if (!stillticking)
        {
            //S_StartSound(0, sfx_barexp);
            ng_state += 1 + 2*!dofrags;
        }
    }
    else if (ng_state == 8)
    {
        if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_frags[i] += 1;

            if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
                cnt_frags[i] = fsum;
            else
                stillticking = true;
        }
        
        if (!stillticking)
        {
            //S_StartSound(0, sfx_pldeth);
            ng_state++;
        }
    }
    else if (ng_state == 10)
    {
        if (acceleratestage)
        {
            //S_StartSound(0, sfx_sgcock);
            if ( gamemode == commercial )
                WI_initNoState();
            else
                WI_initShowNextLoc();
        }
    }
    else if (ng_state & 1)
    {
        if (!--cnt_pause)
        {
            ng_state++;
            cnt_pause = TICRATE;
        }
    }
}



void WI_drawNetgameStats(void)
{
    int         i;
    int         x;
    int         y;
    int         pwidth = SHORT(percent->width);

    WI_slamBackground();
    
    // draw animated background
    WI_drawAnimatedBack(); 

    WI_drawLF();

    // draw stat titles (top line)
    V_DrawPatch(NG_STATSX+NG_SPACINGX-SHORT(kills->width),
                NG_STATSY, kills);

    V_DrawPatch(NG_STATSX+2*NG_SPACINGX-SHORT(items->width),
                NG_STATSY, items);

    V_DrawPatch(NG_STATSX+3*NG_SPACINGX-SHORT(secret->width),
                NG_STATSY, secret);
    
    if (dofrags)
        V_DrawPatch(NG_STATSX+4*NG_SPACINGX-SHORT(frags->width),
                    NG_STATSY, frags);

    // draw stats
    y = NG_STATSY + SHORT(kills->height);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

        x = NG_STATSX;
        V_DrawPatch(x-SHORT(p[i]->width), y, p[i]);

        if (i == me)
            V_DrawPatch(x-SHORT(p[i]->width), y, star);

        x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_kills[i]);   x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_items[i]);   x += NG_SPACINGX;
        WI_drawPercent(x-pwidth, y+10, cnt_secret[i]);  x += NG_SPACINGX;

        if (dofrags)
            WI_drawNum(x, y+10, cnt_frags[i], -1);

        y += WI_SPACINGY;
    }

}
*/

static uint8_t      sp_state;

void WI_initStats(void)
{
    PRINTF("WI_initStats\n");
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();
}

void WI_updateStats(void)
{
    // PRINTF("WI_updateStats\n");

    WI_updateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = 0;
        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
        cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
        cnt_time = plrs[me].stime / TICRATE;
        cnt_par = wbs->partime / TICRATE;
        //S_StartSound(0, sfx_barexp);
        sp_state = 10;
    }

    if (sp_state == 2)
    {
        cnt_kills[0] += 2;

        //if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
        {
            cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
            //S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 4)
    {
        cnt_items[0] += 2;

        //if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
        {
            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
            //S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }
    else if (sp_state == 6)
    {
        cnt_secret[0] += 2;

        //if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
        {
            cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
            //S_StartSound(0, sfx_barexp);
            sp_state++;
        }
    }

    else if (sp_state == 8)
    {
        //if (!(bcnt&3))
            //S_StartSound(0, sfx_pistol);

        cnt_time += 3;

        if (cnt_time >= plrs[me].stime / TICRATE)
            cnt_time = plrs[me].stime / TICRATE;

        cnt_par += 3;

        if (cnt_par >= wbs->partime / TICRATE)
        {
            cnt_par = wbs->partime / TICRATE;

            if (cnt_time >= plrs[me].stime / TICRATE)
            {
                //S_StartSound(0, sfx_barexp);
                sp_state++;
            }
        }
    }
    else if (sp_state == 10)
    {
        if (acceleratestage)
        {
            //S_StartSound(0, sfx_sgcock);

            if (gamemode == commercial)
                WI_initNoState();
            else
                WI_initShowNextLoc();
        }
    }
    else if (sp_state & 1)
    {
        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = TICRATE;
        }
    }

}

void WI_drawStats(void)
{
    // PRINTF("WI_drawStats\n");
    // line height
    int lh;     
    patch_t temp_num; 
    X_spi_read(num[0], &temp_num, sizeof(temp_num)/4); 
    lh = (3*SHORT(temp_num.height))/2;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();
    
    WI_drawLF();

    V_DrawPatch(SP_STATSX, SP_STATSY, kills);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawPatch(SP_STATSX, SP_STATSY+lh, items);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    V_DrawPatch(SP_STATSX, SP_STATSY+2*lh, sp_secret);
    WI_drawPercent(SCREENWIDTH - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

    V_DrawPatch(SP_TIMEX, SP_TIMEY, timepatch);
    WI_drawTime(SCREENWIDTH/2 - SP_TIMEX, SP_TIMEY, cnt_time);

    if (wbs->epsd < 3)
    {
        V_DrawPatch(SCREENWIDTH/2 + SP_TIMEX, SP_TIMEY, par);
        WI_drawTime(SCREENWIDTH - SP_TIMEX, SP_TIMEY, cnt_par);
    }

}

void WI_checkForAccelerate(void)
{
    int   i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
            {
                if (!player->attackdown)
                    acceleratestage = 1;
                player->attackdown = true;
            }
            else
                player->attackdown = false;
            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                    acceleratestage = 1;
                player->usedown = true;
            }
            else
                player->usedown = false;
        }
    }
}



// Updates stuff each tick
void WI_Ticker(void)
{
    // PRINTF("WI_Ticker\n");
    
    // counter for general background animation
    bcnt++;  
/* X-HEEP COMMENT
    if (bcnt == 1)
    {
        // intermission music
        if ( gamemode == commercial )
          //S_ChangeMusic(mus_dm2int, true);
        else
          //S_ChangeMusic(mus_inter, true); 
    }
X-HEEP COMMENT END */
    WI_checkForAccelerate();

    switch (state)
    {
      case StatCount:
        // NRFD-TODO: Multiplayer
        // if (deathmatch) WI_updateDeathmatchStats();
        // else if (netgame) WI_updateNetgameStats();
        // else 
            WI_updateStats();
        break;
        
      case ShowNextLoc:
        WI_updateShowNextLoc();
        break;
        
      case NoState:
        WI_updateNoState();
        break;
    }

}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);

// Common load/unload function.  Iterates over all the graphics
// lumps to be loaded/unloaded into memory.

static void WI_loadUnloadData(load_callback_t callback)
{
    int i, j;
    char name[9];
    anim_t *a;

    if (gamemode == commercial)
    {
        for (i=0 ; i<NUMCMAPS ; i++)
        {
            DEH_snprintf(name, 9, "CWILV%2.2d", i);
            callback(name, &lnames[i]);
        }
    }
    else
    {
        for (i=0 ; i<NUMMAPS ; i++)
        {
            DEH_snprintf(name, 9, "WILV%d%d", wbs->epsd, i);
            callback(name, &lnames[i]);
        }

        // you are here
        callback(DEH_String("WIURH0"), &yah[0]);

        // you are here (alt.)
        callback(DEH_String("WIURH1"), &yah[1]);

        // splat
        callback(DEH_String("WISPLAT"), &splat[0]);

        /* NRFD-TODO:
        if (wbs->epsd < 3)
        {
            for (j=0;j<NUMANIMS[wbs->epsd];j++)
            {
                a = &anims[wbs->epsd][j];
                for (i=0;i<a->nanims;i++)
                {
                    // MONDO HACK!
                    if (wbs->epsd != 1 || j != 8)
                    {
                        // animations
                        //DEH_snprintf(name, 9, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                        callback(name, &a->p[i]);
                    }
                    else
                    {
                        // HACK ALERT!
                        a->p[i] = anims[1][4].p[i];
                    }
                }
            }
        }
        */
    }

    // More hacks on minus sign.
    callback(DEH_String("WIMINUS"), &wiminus);

    for (i=0;i<10;i++)
    {
         // numbers 0-9
        DEH_snprintf(name, 9, "WINUM%d", i);
        callback(name, &num[i]);
    }

    // percent sign
    callback(DEH_String("WIPCNT"), &percent);

    // "finished"
    callback(DEH_String("WIF"), &finished);

    // "entering"
    callback(DEH_String("WIENTER"), &entering);

    // "kills"
    callback(DEH_String("WIOSTK"), &kills);

    // "scrt"
    callback(DEH_String("WIOSTS"), &secret);

     // "secret"
    callback(DEH_String("WISCRT2"), &sp_secret);

    // french wad uses WIOBJ (?)
    if (W_CheckNumForName(DEH_String("WIOBJ")) >= 0)
    {
        // "items"
        if (netgame && !deathmatch)
            callback(DEH_String("WIOBJ"), &items);
        else
            callback(DEH_String("WIOSTI"), &items);
    } else {
        callback(DEH_String("WIOSTI"), &items);
    }

    // "frgs"
    callback(DEH_String("WIFRGS"), &frags);

    // ":"
    callback(DEH_String("WICOLON"), &colon);

    // "time"
    callback(DEH_String("WITIME"), &timepatch);

    // "sucks"
    callback(DEH_String("WISUCKS"), &sucks);

    // "par"
    callback(DEH_String("WIPAR"), &par);

    // "killers" (vertical)
    callback(DEH_String("WIKILRS"), &killers);

    // "victims" (horiz)
    callback(DEH_String("WIVCTMS"), &victims);

    // "total"
    callback(DEH_String("WIMSTT"), &total);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // "1,2,3,4"
        DEH_snprintf(name, 9, "STPB%d", i);
        callback(name, &p[i]);

        // "1,2,3,4"
        DEH_snprintf(name, 9, "WIBP%d", i+1);
        callback(name, &bp[i]);
    }

    // Background image

    if (gamemode == commercial)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else if (gameversion >= exe_ultimate && wbs->epsd == 3)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else
    {
        DEH_snprintf(name, sizeof(name), "WIMAP%d", wbs->epsd);
    }

    // Draw backdrop and save to a temporary buffer

    callback(name, &background);
}

static void WI_loadCallback(char *name, patch_t **variable)
{
    *variable = W_CacheLumpName(name, PU_STATIC);
}

void WI_loadData(void)
{
    PRINTF("WI_loadData\n");

    if (gamemode == commercial)
    {
        NUMCMAPS = 32;
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMCMAPS,
                                       PU_STATIC, NULL);
    }
    else
    {
        lnames = (patch_t **) Z_Malloc(sizeof(patch_t*) * NUMMAPS,
                                       PU_STATIC, NULL);
    }

    WI_loadUnloadData(WI_loadCallback);

    // These two graphics are special cased because we're sharing
    // them with the status bar code

    // your face
    star = W_CacheLumpName(DEH_String("STFST01"), PU_STATIC);

    // dead face
    bstar = W_CacheLumpName(DEH_String("STFDEAD0"), PU_STATIC);
}

static void WI_unloadCallback(char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}

void WI_unloadData(void)
{
    PRINTF("WI_unloadData\n");
    WI_loadUnloadData(WI_unloadCallback);

    // We do not free these lumps as they are shared with the status
    // bar code.
   
    // W_ReleaseLumpName("STFST01");
    // W_ReleaseLumpName("STFDEAD0");
    PRINTF("WI_unloadData finished\n");
}

void WI_Drawer (void)
{
    // PRINTF("WI_Drawer\n");
    switch (state)
    {
      case StatCount:
        // NRFD-TODO: multiplayer
        // if (deathmatch)
        //     WI_drawDeathmatchStats();
        // else if (netgame)
        //     WI_drawNetgameStats();
        // else
            WI_drawStats();
        break;
        
      case ShowNextLoc:
        WI_drawShowNextLoc();
        break;
        
      case NoState:
        WI_drawNoState();
        break;
    }
}


void WI_initVariables(wbstartstruct_t* wbstartstruct)
{
    PRINTF("WI_initVariables\n");

    wbs = wbstartstruct;

#ifdef RANGECHECKING
    if (gamemode != commercial)
    {
      if (gameversion >= exe_ultimate)
        RNGCHECK(wbs->epsd, 0, 3);
      else
        RNGCHECK(wbs->epsd, 0, 2);
    }
    else
    {
        RNGCHECK(wbs->last, 0, 8);
        RNGCHECK(wbs->next, 0, 8);
    }
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
        wbs->maxkills = 1;

    if (!wbs->maxitems)
        wbs->maxitems = 1;

    if (!wbs->maxsecret)
        wbs->maxsecret = 1;

    if ( gameversion < exe_ultimate )
      if (wbs->epsd > 2)
        wbs->epsd -= 3;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();

    // NRFD-TODO: Multiplayer
    // if (deathmatch)
    //     WI_initDeathmatchStats();
    // else if (netgame)
    //     WI_initNetgameStats();
    // else
        WI_initStats();
}
