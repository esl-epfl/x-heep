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
//	System specific interface stuff.
//


#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

// Screen width and height.

#define SCREENWIDTH  320
#define SCREENHEIGHT 200

// Screen height used when aspect_ratio_correct=true.

#define SCREENHEIGHT_4_3 240

// extern char *video_driver;
extern const boolean screenvisible;

extern const boolean vanilla_keyboard_mapping;
// extern const boolean screensaver_mode;
extern byte usegamma;
extern pixel_t *I_VideoBuffer;
//extern pixel_t *I_VideoBackBuffer;
extern pixel_t I_VideoBuffers[320*200];

typedef struct palette_pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} palette_pixel_t;

extern palette_pixel_t display_palette[256];

// extern int screen_width;
// extern int screen_height;
// extern int fullscreen;
// extern int aspect_ratio_correct;
// extern int integer_scaling;
// extern int vga_porch_flash;
// extern int force_software_renderer;

// Joystic/gamepad hysteresis
extern unsigned int joywait;

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);

void I_GraphicsCheckCommandLine(void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette);
int I_GetPaletteIndex(int r, int g, int b);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

void I_ReadScreen (pixel_t* scr);

void I_BeginRead (void);

void I_SetWindowTitle(char *title);

void I_DisplayFPSDots(boolean dots_on);
void I_BindVideoVariables(void);

void I_InitWindowTitle(void);
void I_InitWindowIcon(void);

// Called before processing any tics in a frame (just after displaying a frame).
// Time consuming syncronous operations are performed here (joystick reading).

void I_StartFrame (void);

// Called before processing each tic in a frame.
// Quick syncronous operations are performed here.

void I_StartTic (void);

// Enable the loading disk image displayed when reading from disk.

void I_EnableLoadingDisk(int xoffs, int yoffs);

void I_ClearVideoBuffer(void);

#endif
