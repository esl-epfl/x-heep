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
//      Rendering main loop and setup functions,
//       utility functions (BSP, geometry, trigonometry).
//      See tables.c, too.
//





#include <stdlib.h>
#include <math.h>


#include "doomdef.h"
#include "d_loop.h"

#include "m_bbox.h"
#include "m_menu.h"

#include "r_local.h"
#include "r_sky.h"





// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW             2048    



int                     viewangleoffset;

// increment every time a check is made
uint8_t                 validcount = 1;         


lighttable_t*           fixedcolormap;
extern lighttable_t**   walllights;

int                     centerx;
int                     centery;

fixed_t                 centerxfrac;
fixed_t                 centeryfrac;
fixed_t                 projection;

// just for profiling purposes
int                     framecount;     

int                     sscount;
int                     linecount;
int                     loopcount;

fixed_t                 viewx;
fixed_t                 viewy;
fixed_t                 viewz;

angle_t                 viewangle;

fixed_t                 viewcos;
fixed_t                 viewsin;

player_t*               viewplayer;

// 0 = high, 1 = low
int                     detailshift;    

//
// precalculated math tables
//
angle_t                 clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
// Was: int viewangletox
const int                     viewangletox[FINEANGLES/2] = 
{320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 320, 
320, 320, 320, 320, 319, 319, 319, 319, 318, 318, 318, 318, 317, 317, 317, 317, 
316, 316, 316, 316, 315, 315, 315, 315, 314, 314, 314, 314, 314, 313, 313, 313, 
313, 312, 312, 312, 312, 311, 311, 311, 311, 311, 310, 310, 310, 310, 309, 309, 
309, 309, 308, 308, 308, 308, 308, 307, 307, 307, 307, 306, 306, 306, 306, 306, 
305, 305, 305, 305, 304, 304, 304, 304, 304, 303, 303, 303, 303, 302, 302, 302, 
302, 302, 301, 301, 301, 301, 300, 300, 300, 300, 300, 299, 299, 299, 299, 299, 
298, 298, 298, 298, 297, 297, 297, 297, 297, 296, 296, 296, 296, 296, 295, 295, 
295, 295, 295, 294, 294, 294, 294, 293, 293, 293, 293, 293, 292, 292, 292, 292, 
292, 291, 291, 291, 291, 291, 290, 290, 290, 290, 290, 289, 289, 289, 289, 289, 
288, 288, 288, 288, 288, 287, 287, 287, 287, 287, 286, 286, 286, 286, 286, 285, 
285, 285, 285, 285, 284, 284, 284, 284, 284, 283, 283, 283, 283, 283, 282, 282, 
282, 282, 282, 281, 281, 281, 281, 281, 281, 280, 280, 280, 280, 280, 279, 279, 
279, 279, 279, 278, 278, 278, 278, 278, 277, 277, 277, 277, 277, 277, 276, 276, 
276, 276, 276, 275, 275, 275, 275, 275, 274, 274, 274, 274, 274, 274, 273, 273, 
273, 273, 273, 272, 272, 272, 272, 272, 272, 271, 271, 271, 271, 271, 270, 270, 
270, 270, 270, 270, 269, 269, 269, 269, 269, 268, 268, 268, 268, 268, 268, 267, 
267, 267, 267, 267, 267, 266, 266, 266, 266, 266, 265, 265, 265, 265, 265, 265, 
264, 264, 264, 264, 264, 264, 263, 263, 263, 263, 263, 263, 262, 262, 262, 262, 
262, 261, 261, 261, 261, 261, 261, 260, 260, 260, 260, 260, 260, 259, 259, 259, 
259, 259, 259, 258, 258, 258, 258, 258, 258, 257, 257, 257, 257, 257, 257, 256, 
256, 256, 256, 256, 256, 255, 255, 255, 255, 255, 255, 254, 254, 254, 254, 254, 
254, 253, 253, 253, 253, 253, 253, 252, 252, 252, 252, 252, 252, 251, 251, 251, 
251, 251, 251, 251, 250, 250, 250, 250, 250, 250, 249, 249, 249, 249, 249, 249, 
248, 248, 248, 248, 248, 248, 247, 247, 247, 247, 247, 247, 247, 246, 246, 246, 
246, 246, 246, 245, 245, 245, 245, 245, 245, 244, 244, 244, 244, 244, 244, 244, 
243, 243, 243, 243, 243, 243, 242, 242, 242, 242, 242, 242, 242, 241, 241, 241, 
241, 241, 241, 240, 240, 240, 240, 240, 240, 240, 239, 239, 239, 239, 239, 239, 
238, 238, 238, 238, 238, 238, 238, 237, 237, 237, 237, 237, 237, 236, 236, 236, 
236, 236, 236, 236, 235, 235, 235, 235, 235, 235, 235, 234, 234, 234, 234, 234, 
234, 234, 233, 233, 233, 233, 233, 233, 232, 232, 232, 232, 232, 232, 232, 231, 
231, 231, 231, 231, 231, 231, 230, 230, 230, 230, 230, 230, 230, 229, 229, 229, 
229, 229, 229, 229, 228, 228, 228, 228, 228, 228, 228, 227, 227, 227, 227, 227, 
227, 227, 226, 226, 226, 226, 226, 226, 226, 225, 225, 225, 225, 225, 225, 225, 
224, 224, 224, 224, 224, 224, 224, 223, 223, 223, 223, 223, 223, 223, 222, 222, 
222, 222, 222, 222, 222, 221, 221, 221, 221, 221, 221, 221, 220, 220, 220, 220, 
220, 220, 220, 219, 219, 219, 219, 219, 219, 219, 218, 218, 218, 218, 218, 218, 
218, 217, 217, 217, 217, 217, 217, 217, 217, 216, 216, 216, 216, 216, 216, 216, 
215, 215, 215, 215, 215, 215, 215, 214, 214, 214, 214, 214, 214, 214, 214, 213, 
213, 213, 213, 213, 213, 213, 212, 212, 212, 212, 212, 212, 212, 211, 211, 211, 
211, 211, 211, 211, 211, 210, 210, 210, 210, 210, 210, 210, 209, 209, 209, 209, 
209, 209, 209, 209, 208, 208, 208, 208, 208, 208, 208, 207, 207, 207, 207, 207, 
207, 207, 207, 206, 206, 206, 206, 206, 206, 206, 205, 205, 205, 205, 205, 205, 
205, 205, 204, 204, 204, 204, 204, 204, 204, 203, 203, 203, 203, 203, 203, 203, 
203, 202, 202, 202, 202, 202, 202, 202, 202, 201, 201, 201, 201, 201, 201, 201, 
200, 200, 200, 200, 200, 200, 200, 200, 199, 199, 199, 199, 199, 199, 199, 199, 
198, 198, 198, 198, 198, 198, 198, 197, 197, 197, 197, 197, 197, 197, 197, 196, 
196, 196, 196, 196, 196, 196, 196, 195, 195, 195, 195, 195, 195, 195, 195, 194, 
194, 194, 194, 194, 194, 194, 194, 193, 193, 193, 193, 193, 193, 193, 192, 192, 
192, 192, 192, 192, 192, 192, 191, 191, 191, 191, 191, 191, 191, 191, 190, 190, 
190, 190, 190, 190, 190, 190, 189, 189, 189, 189, 189, 189, 189, 189, 188, 188, 
188, 188, 188, 188, 188, 188, 187, 187, 187, 187, 187, 187, 187, 187, 186, 186, 
186, 186, 186, 186, 186, 186, 185, 185, 185, 185, 185, 185, 185, 185, 184, 184, 
184, 184, 184, 184, 184, 184, 183, 183, 183, 183, 183, 183, 183, 183, 182, 182, 
182, 182, 182, 182, 182, 182, 181, 181, 181, 181, 181, 181, 181, 181, 180, 180, 
180, 180, 180, 180, 180, 180, 179, 179, 179, 179, 179, 179, 179, 179, 178, 178, 
178, 178, 178, 178, 178, 178, 177, 177, 177, 177, 177, 177, 177, 177, 176, 176, 
176, 176, 176, 176, 176, 176, 175, 175, 175, 175, 175, 175, 175, 175, 174, 174, 
174, 174, 174, 174, 174, 174, 173, 173, 173, 173, 173, 173, 173, 173, 172, 172, 
172, 172, 172, 172, 172, 172, 171, 171, 171, 171, 171, 171, 171, 171, 171, 170, 
170, 170, 170, 170, 170, 170, 170, 169, 169, 169, 169, 169, 169, 169, 169, 168, 
168, 168, 168, 168, 168, 168, 168, 167, 167, 167, 167, 167, 167, 167, 167, 166, 
166, 166, 166, 166, 166, 166, 166, 165, 165, 165, 165, 165, 165, 165, 165, 164, 
164, 164, 164, 164, 164, 164, 164, 164, 163, 163, 163, 163, 163, 163, 163, 163, 
162, 162, 162, 162, 162, 162, 162, 162, 161, 161, 161, 161, 161, 161, 161, 161, 
160, 160, 160, 160, 160, 160, 160, 160, 159, 159, 159, 159, 159, 159, 159, 159, 
158, 158, 158, 158, 158, 158, 158, 158, 157, 157, 157, 157, 157, 157, 157, 157, 
157, 156, 156, 156, 156, 156, 156, 156, 156, 155, 155, 155, 155, 155, 155, 155, 
155, 154, 154, 154, 154, 154, 154, 154, 154, 153, 153, 153, 153, 153, 153, 153, 
153, 152, 152, 152, 152, 152, 152, 152, 152, 151, 151, 151, 151, 151, 151, 151, 
151, 150, 150, 150, 150, 150, 150, 150, 150, 150, 149, 149, 149, 149, 149, 149, 
149, 149, 148, 148, 148, 148, 148, 148, 148, 148, 147, 147, 147, 147, 147, 147, 
147, 147, 146, 146, 146, 146, 146, 146, 146, 146, 145, 145, 145, 145, 145, 145, 
145, 145, 144, 144, 144, 144, 144, 144, 144, 144, 143, 143, 143, 143, 143, 143, 
143, 143, 142, 142, 142, 142, 142, 142, 142, 142, 141, 141, 141, 141, 141, 141, 
141, 141, 140, 140, 140, 140, 140, 140, 140, 140, 139, 139, 139, 139, 139, 139, 
139, 139, 138, 138, 138, 138, 138, 138, 138, 138, 137, 137, 137, 137, 137, 137, 
137, 137, 136, 136, 136, 136, 136, 136, 136, 136, 135, 135, 135, 135, 135, 135, 
135, 135, 134, 134, 134, 134, 134, 134, 134, 134, 133, 133, 133, 133, 133, 133, 
133, 133, 132, 132, 132, 132, 132, 132, 132, 132, 131, 131, 131, 131, 131, 131, 
131, 131, 130, 130, 130, 130, 130, 130, 130, 130, 129, 129, 129, 129, 129, 129, 
129, 129, 128, 128, 128, 128, 128, 128, 128, 127, 127, 127, 127, 127, 127, 127, 
127, 126, 126, 126, 126, 126, 126, 126, 126, 125, 125, 125, 125, 125, 125, 125, 
125, 124, 124, 124, 124, 124, 124, 124, 124, 123, 123, 123, 123, 123, 123, 123, 
122, 122, 122, 122, 122, 122, 122, 122, 121, 121, 121, 121, 121, 121, 121, 121, 
120, 120, 120, 120, 120, 120, 120, 119, 119, 119, 119, 119, 119, 119, 119, 118, 
118, 118, 118, 118, 118, 118, 118, 117, 117, 117, 117, 117, 117, 117, 116, 116, 
116, 116, 116, 116, 116, 116, 115, 115, 115, 115, 115, 115, 115, 114, 114, 114, 
114, 114, 114, 114, 114, 113, 113, 113, 113, 113, 113, 113, 112, 112, 112, 112, 
112, 112, 112, 112, 111, 111, 111, 111, 111, 111, 111, 110, 110, 110, 110, 110, 
110, 110, 110, 109, 109, 109, 109, 109, 109, 109, 108, 108, 108, 108, 108, 108, 
108, 107, 107, 107, 107, 107, 107, 107, 107, 106, 106, 106, 106, 106, 106, 106, 
105, 105, 105, 105, 105, 105, 105, 104, 104, 104, 104, 104, 104, 104, 104, 103, 
103, 103, 103, 103, 103, 103, 102, 102, 102, 102, 102, 102, 102, 101, 101, 101, 
101, 101, 101, 101, 100, 100, 100, 100, 100, 100, 100, 99, 99, 99, 99, 99, 
99, 99, 98, 98, 98, 98, 98, 98, 98, 97, 97, 97, 97, 97, 97, 97, 
96, 96, 96, 96, 96, 96, 96, 95, 95, 95, 95, 95, 95, 95, 94, 94, 
94, 94, 94, 94, 94, 93, 93, 93, 93, 93, 93, 93, 92, 92, 92, 92, 
92, 92, 92, 91, 91, 91, 91, 91, 91, 91, 90, 90, 90, 90, 90, 90, 
90, 89, 89, 89, 89, 89, 89, 89, 88, 88, 88, 88, 88, 88, 87, 87, 
87, 87, 87, 87, 87, 86, 86, 86, 86, 86, 86, 86, 85, 85, 85, 85, 
85, 85, 85, 84, 84, 84, 84, 84, 84, 83, 83, 83, 83, 83, 83, 83, 
82, 82, 82, 82, 82, 82, 81, 81, 81, 81, 81, 81, 81, 80, 80, 80, 
80, 80, 80, 79, 79, 79, 79, 79, 79, 79, 78, 78, 78, 78, 78, 78, 
77, 77, 77, 77, 77, 77, 77, 76, 76, 76, 76, 76, 76, 75, 75, 75, 
75, 75, 75, 74, 74, 74, 74, 74, 74, 74, 73, 73, 73, 73, 73, 73, 
72, 72, 72, 72, 72, 72, 71, 71, 71, 71, 71, 71, 70, 70, 70, 70, 
70, 70, 70, 69, 69, 69, 69, 69, 69, 68, 68, 68, 68, 68, 68, 67, 
67, 67, 67, 67, 67, 66, 66, 66, 66, 66, 66, 65, 65, 65, 65, 65, 
65, 64, 64, 64, 64, 64, 64, 63, 63, 63, 63, 63, 63, 62, 62, 62, 
62, 62, 62, 61, 61, 61, 61, 61, 61, 60, 60, 60, 60, 60, 60, 59, 
59, 59, 59, 59, 58, 58, 58, 58, 58, 58, 57, 57, 57, 57, 57, 57, 
56, 56, 56, 56, 56, 56, 55, 55, 55, 55, 55, 54, 54, 54, 54, 54, 
54, 53, 53, 53, 53, 53, 53, 52, 52, 52, 52, 52, 51, 51, 51, 51, 
51, 51, 50, 50, 50, 50, 50, 49, 49, 49, 49, 49, 49, 48, 48, 48, 
48, 48, 47, 47, 47, 47, 47, 47, 46, 46, 46, 46, 46, 45, 45, 45, 
45, 45, 44, 44, 44, 44, 44, 44, 43, 43, 43, 43, 43, 42, 42, 42, 
42, 42, 41, 41, 41, 41, 41, 40, 40, 40, 40, 40, 40, 39, 39, 39, 
39, 39, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 36, 36, 36, 36, 
36, 35, 35, 35, 35, 35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 
32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29, 
29, 29, 29, 29, 28, 28, 28, 28, 28, 27, 27, 27, 27, 26, 26, 26, 
26, 26, 25, 25, 25, 25, 25, 24, 24, 24, 24, 24, 23, 23, 23, 23, 
22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 20, 20, 20, 20, 19, 19, 
19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 17, 16, 16, 16, 16, 
15, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12, 
12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 10, 9, 9, 9, 9, 8, 
8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 
4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// The xtoviewangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
const angle_t                 xtoviewangle[SCREENWIDTH+1] = 
{537395200, 534773760, 532676608, 530579456, 528482304, 526385152, 524288000, 521666560, 
519569408, 517472256, 514850816, 512753664, 510656512, 508035072, 505937920, 503316480, 
501219328, 498597888, 496500736, 493879296, 491782144, 489160704, 486539264, 484442112, 
481820672, 479199232, 476577792, 474480640, 471859200, 469237760, 466616320, 463994880, 
461373440, 458752000, 456130560, 453509120, 450887680, 448266240, 445644800, 443023360, 
439877632, 437256192, 434634752, 432013312, 428867584, 426246144, 423624704, 420478976, 
417857536, 414711808, 412090368, 408944640, 406323200, 403177472, 400031744, 397410304, 
394264576, 391118848, 387973120, 385351680, 382205952, 379060224, 375914496, 372768768, 
369623040, 366477312, 363331584, 360185856, 357040128, 353894400, 350224384, 347078656, 
343932928, 340787200, 337117184, 333971456, 330825728, 327155712, 324009984, 320339968, 
317194240, 313524224, 310378496, 306708480, 303562752, 299892736, 296222720, 292552704, 
289406976, 285736960, 282066944, 278396928, 274726912, 271056896, 267386880, 263716864, 
260046848, 256376832, 252706816, 249036800, 245366784, 241696768, 238026752, 234356736, 
230162432, 226492416, 222822400, 218628096, 214958080, 211288064, 207093760, 203423744, 
199229440, 195559424, 191365120, 187695104, 183500800, 179830784, 175636480, 171442176, 
167772160, 163577856, 159383552, 155713536, 151519232, 147324928, 143130624, 138936320, 
135266304, 131072000, 126877696, 122683392, 118489088, 114294784, 110100480, 105906176, 
101711872, 97517568, 93323264, 89128960, 84934656, 80740352, 76546048, 72351744, 
68157440, 63963136, 59768832, 55574528, 51380224, 47185920, 42467328, 38273024, 
34078720, 29884416, 25690112, 21495808, 17301504, 12582912, 8388608, 4194304, 
0, -4194304, -8388608, -12582912, -17301504, -21495808, -25690112, -29884416, 
-34078720, -38273024, -42467328, -47185920, -51380224, -55574528, -59768832, -63963136, 
-68157440, -72351744, -76546048, -80740352, -84934656, -89128960, -93323264, -97517568, 
-101711872, -105906176, -110100480, -114294784, -118489088, -122683392, -126877696, -131072000, 
-135266304, -138936320, -143130624, -147324928, -151519232, -155713536, -159383552, -163577856, 
-167772160, -171442176, -175636480, -179830784, -183500800, -187695104, -191365120, -195559424, 
-199229440, -203423744, -207093760, -211288064, -214958080, -218628096, -222822400, -226492416, 
-230162432, -234356736, -238026752, -241696768, -245366784, -249036800, -252706816, -256376832, 
-260046848, -263716864, -267386880, -271056896, -274726912, -278396928, -282066944, -285736960, 
-289406976, -292552704, -296222720, -299892736, -303562752, -306708480, -310378496, -313524224, 
-317194240, -320339968, -324009984, -327155712, -330825728, -333971456, -337117184, -340787200, 
-343932928, -347078656, -350224384, -353894400, -357040128, -360185856, -363331584, -366477312, 
-369623040, -372768768, -375914496, -379060224, -382205952, -385351680, -387973120, -391118848, 
-394264576, -397410304, -400031744, -403177472, -406323200, -408944640, -412090368, -414711808, 
-417857536, -420478976, -423624704, -426246144, -428867584, -432013312, -434634752, -437256192, 
-439877632, -443023360, -445644800, -448266240, -450887680, -453509120, -456130560, -458752000, 
-461373440, -463994880, -466616320, -469237760, -471859200, -474480640, -476577792, -479199232, 
-481820672, -484442112, -486539264, -489160704, -491782144, -493879296, -496500736, -498597888, 
-501219328, -503316480, -505937920, -508035072, -510656512, -512753664, -514850816, -517472256, 
-519569408, -521666560, -524288000, -526385152, -528482304, -530579456, -532676608, -534773760 };

lighttable_t*           scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t*           scalelightfixed[MAXLIGHTSCALE];
lighttable_t*           zlight[LIGHTLEVELS][MAXLIGHTZ];

// bumped light from gun blasts
int                     extralight;                     



void (*colfunc) (void);
void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);
void (*transcolfunc) (void);
void (*spanfunc) (void);



//
// R_AddPointToBox
// Expand a given bbox
// so that it encloses a given point.
//
void
R_AddPointToBox
( int           x,
  int           y,
  fixed_t*      box )
{
    if (x< box[BOXLEFT])
        box[BOXLEFT] = x;
    if (x> box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y< box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    if (y> box[BOXTOP])
        box[BOXTOP] = y;
}


//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
int
R_PointOnSide
( fixed_t       x,
  fixed_t       y,
  node_t*       node )
{
    // PRINTF("R_PointOnSide\n");

    fixed_t     dx;
    fixed_t     dy;
    fixed_t     left;
    fixed_t     right;
        
    if (!node->dx)
    {
        if (x <= node->x)
            return node->dy > 0;
        
        return node->dy < 0;
    }
    if (!node->dy)
    {
        if (y <= node->y)
            return node->dx < 0;
        
        return node->dx > 0;
    }
        
    dx = (x - node->x);
    dy = (y - node->y);
        
    // Try to quickly decide by looking at sign bits.
    if ( (node->dy ^ node->dx ^ dx ^ dy)&0x80000000 )
    {
        if  ( (node->dy ^ dx) & 0x80000000 )
        {
            // (left is negative)
            return 1;
        }
        return 0;
    }

    left = FixedMul ( node->dy>>FRACBITS , dx );
    right = FixedMul ( dy , node->dx>>FRACBITS );
        
    if (right < left)
    {
        // front side
        return 0;
    }
    // back side
    return 1;                   
}


int
R_PointOnSegSide
( fixed_t       x,
  fixed_t       y,
  seg_t*        line )
{
    fixed_t     lx;
    fixed_t     ly;
    fixed_t     ldx;
    fixed_t     ldy;
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     left;
    fixed_t     right;

    vertex_t *v1 = SegV1(line);
    vertex_t *v2 = SegV2(line);

    lx = v1->x;
    ly = v1->y;
        
    ldx = v2->x - lx;
    ldy = v2->y - ly;
        
    if (!ldx)
    {
        if (x <= lx)
            return ldy > 0;
        
        return ldy < 0;
    }
    if (!ldy)
    {
        if (y <= ly)
            return ldx < 0;
        
        return ldx > 0;
    }
        
    dx = (x - lx);
    dy = (y - ly);
        
    // Try to quickly decide by looking at sign bits.
    if ( (ldy ^ ldx ^ dx ^ dy)&0x80000000 )
    {
        if  ( (ldy ^ dx) & 0x80000000 )
        {
            // (left is negative)
            return 1;
        }
        return 0;
    }

    left = FixedMul ( ldy>>FRACBITS , dx );
    right = FixedMul ( dy , ldx>>FRACBITS );
        
    if (right < left)
    {
        // front side
        return 0;
    }
    // back side
    return 1;                   
}


//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table.

//




angle_t
R_PointToAngle
( fixed_t       x,
  fixed_t       y )
{       
    x -= viewx;
    y -= viewy;
    
    if ( (!x) && (!y) )
        return 0;

    if (x>= 0)
    {
        // x >=0
        if (y>= 0)
        {
            // y>= 0

            if (x>y)
            {
                // octant 0
                return tantoangle[ SlopeDiv(y,x)];
            }
            else
            {
                // octant 1
                return ANG90-1-tantoangle[ SlopeDiv(x,y)];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 8
                return -tantoangle[SlopeDiv(y,x)];
            }
            else
            {
                // octant 7
                return ANG270+tantoangle[ SlopeDiv(x,y)];
            }
        }
    }
    else
    {
        // x<0
        x = -x;

        if (y>= 0)
        {
            // y>= 0
            if (x>y)
            {
                // octant 3
                return ANG180-1-tantoangle[ SlopeDiv(y,x)];
            }
            else
            {
                // octant 2
                return ANG90+ tantoangle[ SlopeDiv(x,y)];
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 4
                return ANG180+tantoangle[ SlopeDiv(y,x)];
            }
            else
            {
                 // octant 5
                return ANG270-1-tantoangle[ SlopeDiv(x,y)];
            }
        }
    }
    return 0;
}


angle_t
R_PointToAngle2
( fixed_t       x1,
  fixed_t       y1,
  fixed_t       x2,
  fixed_t       y2 )
{       
    viewx = x1;
    viewy = y1;
    
    return R_PointToAngle (x2, y2);
}


fixed_t
R_PointToDist
( fixed_t       x,
  fixed_t       y )
{
    int         angle;
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     temp;
    fixed_t     dist;
    fixed_t     frac;
        
    dx = abs(x - viewx);
    dy = abs(y - viewy);
        
    if (dy>dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    // Fix crashes in udm1.wad

    if (dx != 0)
    {
        frac = FixedDiv(dy, dx);
    }
    else
    {
        frac = 0;
    }
        
    angle = (tantoangle[frac>>DBITS]+ANG90) >> ANGLETOFINESHIFT;

    // use as cosine
    dist = FixedDiv (dx, finesine[angle] );     
        
    return dist;
}




//
// R_InitPointToAngle
//
void R_InitPointToAngle (void)
{
    // UNUSED - now getting from tables.c
#if 0
    int i;
    long        t;
    float       f;
//
// slope (tangent) to angle lookup
//
    for (i=0 ; i<=SLOPERANGE ; i++)
    {
        f = atan( (float)i/SLOPERANGE )/(3.141592657*2);
        t = 0xffffffff*f;
        tantoangle[i] = t;
    }
#endif
}


//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
    fixed_t             scale;
    angle_t             anglea;
    angle_t             angleb;
    int                 sinea;
    int                 sineb;
    fixed_t             num;
    int                 den;

    // UNUSED
#if 0
{
    fixed_t             dist;
    fixed_t             z;
    fixed_t             sinv;
    fixed_t             cosv;
        
    sinv = finesine[(visangle-rw_normalangle)>>ANGLETOFINESHIFT];       
    dist = FixedDiv (rw_distance, sinv);
    cosv = finecosine[(viewangle-visangle)>>ANGLETOFINESHIFT];
    z = abs(FixedMul (dist, cosv));
    scale = FixedDiv(projection, z);
    return scale;
}
#endif

    anglea = ANG90 + (visangle-viewangle);
    angleb = ANG90 + (visangle-rw_normalangle);

    // both sines are allways positive
    sinea = finesine[anglea>>ANGLETOFINESHIFT]; 
    sineb = finesine[angleb>>ANGLETOFINESHIFT];
    num = FixedMul(projection,sineb)<<detailshift;
    den = FixedMul(rw_distance,sinea);

    if (den > num>>FRACBITS)
    {
        scale = FixedDiv (num, den);

        if (scale > 64*FRACUNIT)
            scale = 64*FRACUNIT;
        else if (scale < 256)
            scale = 256;
    }
    else
        scale = 64*FRACUNIT;
        
    return scale;
}



//
// R_InitTables
//
void R_InitTables (void)
{
    // UNUSED: now getting from tables.c
#if 0
    int         i;
    float       a;
    float       fv;
    int         t;
    
    // viewangle tangent table
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        a = (i-FINEANGLES/4+0.5)*PI*2/FINEANGLES;
        fv = FRACUNIT*tan (a);
        t = fv;
        finetangent[i] = t;
    }
    
    // finesine table
    for (i=0 ; i<5*FINEANGLES/4 ; i++)
    {
        // OPTIMIZE: mirror...
        a = (i+0.5)*PI*2/FINEANGLES;
        t = FRACUNIT*sin (a);
        finesine[i] = t;
    }
#endif

}



//
// R_InitTextureMapping
//
void R_InitTextureMapping (void)
{
    int                 i;
    int                 x;
    int                 t;
    int                 t2;
    fixed_t             focallength;
    
    PRINTF("NRFD-TODO: R_InitTextureMapping (review)\n");
    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    focallength = FixedDiv (centerxfrac,
                            finetangent[FINEANGLES/4+FIELDOFVIEW/2] );
    
    /*
    PRINTF("viewangletox = {");
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        if (finetangent[i] > FRACUNIT*2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT*2)
            t = viewwidth+1;
        else
        {
            t = FixedMul (finetangent[i], focallength);
            t = (centerxfrac - t+FRACUNIT-1)>>FRACBITS;

            if (t < -1)
                t = -1;
            else if (t>viewwidth+1)
                t = viewwidth+1;
        }

        // Take out the fencepost cases from viewangletox.
        if (t == -1)
            t = 0;
        else if (t == viewwidth+1)
            t  = viewwidth;

        // viewangletox[i] = t;
        if (i != FINEANGLES/2-1)
            PRINTF("%d, ", t);
        else
            PRINTF("%d };\n\n", t);
        if (i % 16 == 15) PRINTF("\n");
    }
    
    // NRFD-NOTE: Has to be run twice to get correct results!
    // TODO? write viewangletox to malloc'ed buffer to get correct result in single pass?

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x. 
    PRINTF("xtoviewangle = {");
    for (x=0;x<=viewwidth;x++)
    {
        angle_t angle; 

        i = 0;
        while (viewangletox[i]>x)
            i++;

        angle = (i<<ANGLETOFINESHIFT)-ANG90;
        // xtoviewangle[x] = angle;

        if (x != viewwidth-1)
            PRINTF("%d, ", angle);
        else
            PRINTF("%d };\n\n", angle);
        if (x % 8 == 7) PRINTF("\n");
    }
    */

    clipangle = xtoviewangle[0];
}



//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP         2

void R_InitLightTables (void)
{
    int         i;
    int         j;
    int         level;
    int         startmap;       
    int         scale;
    
    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
        startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
        for (j=0 ; j<MAXLIGHTZ ; j++)
        {
            scale = FixedDiv ((SCREENWIDTH/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
            scale >>= LIGHTSCALESHIFT;
            level = startmap - scale/DISTMAP;
            
            if (level < 0)
                level = 0;

            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS-1;

            zlight[i][j] = colormaps + level*256;
        }
    }
}



//
// R_SetViewSize
// Do not really change anything here,
//  because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
boolean         setsizeneeded;
int             setblocks;
int             setdetail;


void
R_SetViewSize
( int           blocks,
  int           detail )
{
    N_ldbg("R_SetViewSize\n");
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}


//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize (void)
{
    fixed_t     cosadj;
    fixed_t     dy;
    int         i;
    int         j;
    int         level;
    int         startmap;


    setsizeneeded = false;

    if (setblocks == 11)
    {
        scaledviewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth = setblocks*32;
        viewheight = (setblocks*168/10)&~7;
    }
    
    detailshift = setdetail;
    viewwidth = scaledviewwidth>>detailshift;
        
    centery = viewheight/2;
    centerx = viewwidth/2;
    centerxfrac = centerx<<FRACBITS;
    centeryfrac = centery<<FRACBITS;
    projection = centerxfrac;

    PRINTF("R_ExecuteSetViewSize\n");
    PRINTF("   scaledviewwidth = %d\n", scaledviewwidth);
    PRINTF("   viewheight      = %d\n", viewheight);
    PRINTF("   viewwidth       = %d\n", scaledviewwidth);
    PRINTF("   detailshift     = %d\n", detailshift);

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        fuzzcolfunc = R_DrawFuzzColumn;
        transcolfunc = R_DrawTranslatedColumn;
        spanfunc = R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        fuzzcolfunc = R_DrawFuzzColumnLow;
        transcolfunc = R_DrawTranslatedColumnLow;
        spanfunc = R_DrawSpanLow;
    }

    R_InitBuffer (scaledviewwidth, viewheight);
        
    R_InitTextureMapping ();
    

    // psprite scales
    pspritescale = FRACUNIT*viewwidth/SCREENWIDTH;
    pspriteiscale = FRACUNIT*SCREENWIDTH/viewwidth;
    
    /* NRFD-TODO: view size
    // thing clipping
    for (i=0 ; i<viewwidth ; i++)
        screenheightarray[i] = viewheight;
    */

    PRINTF("NRFD-TODO: R_ExecuteSetViewSize\n");
    /*
    // planes
    PRINTF("yslope = {\n");
    for (i=0 ; i<viewheight ; i++)
    {
        dy = ((i-viewheight/2)<<FRACBITS)+FRACUNIT/2;
        dy = abs(dy);
        // yslope[i] = FixedDiv ( (viewwidth<<detailshift)/2*FRACUNIT, dy);
        fixed_t ys = FixedDiv ( (viewwidth<<detailshift)/2*FRACUNIT, dy);
        PRINTF("0x%X,", ys);
    }
    PRINTF("}\n");
        
    PRINTF("distscale = {\n");
    for (i=0 ; i<viewwidth ; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
        // distscale[i] = FixedDiv (FRACUNIT,cosadj);
        fixed_t ds = FixedDiv (FRACUNIT,cosadj);
        PRINTF("0x%X,", ds);
    }
    PRINTF("}\n");
    */

    // NRF-TODO: Move to const table?

    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
        startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
        for (j=0 ; j<MAXLIGHTSCALE ; j++)
        {
            level = startmap - j*SCREENWIDTH/(viewwidth<<detailshift)/DISTMAP;
            
            if (level < 0)
                level = 0;

            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS-1;

            scalelight[i][j] = colormaps + level*256;
        }
    }
}



//
// R_Init
//



void R_Init (void)
{
    R_InitData ();
    R_InitPointToAngle ();
    R_InitTables ();
    // viewwidth / viewheight / detailLevel are set by the defaults

    R_SetViewSize (screenblocks, detailLevel);
    R_InitPlanes ();
    R_InitLightTables ();
    R_InitSkyMap ();
    R_InitTranslationTables ();
        
    framecount = 0;
}


//
// R_PointInSubsector
//
subsector_t*
R_PointInSubsector
( fixed_t       x,
  fixed_t       y )
{
    // PRINTF("R_PointInSubsector\n");
    
    int         side;
    int         nodenum;

    // single subsector is a special case
    if (!numnodes)                              
        return subsectors;
                
    nodenum = numnodes-1;

    while (! (nodenum & NF_SUBSECTOR) )
    {
        // NRFD-TODO: Optimize?
        node_t node = GetNode(nodenum); //&nodes[nodenum];
        side = R_PointOnSide (x, y, &node);
        nodenum = node.children[side];
    }
        
    return &subsectors[nodenum & ~NF_SUBSECTOR];
}



//
// R_SetupFrame
//
void R_SetupFrame (player_t* player)
{               
    int         i;
    
    viewplayer = player;
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewangle = player->mo->angle + viewangleoffset;
    extralight = player->extralight;

    viewz = player->viewz;
    
    viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];
        
    sscount = 0;
        
    if (player->fixedcolormap)
    {
        fixedcolormap =
            colormaps
            + player->fixedcolormap*256;
        
        walllights = scalelightfixed;

        for (i=0 ; i<MAXLIGHTSCALE ; i++)
            scalelightfixed[i] = fixedcolormap;
    }
    else
        fixedcolormap = 0;
                
    framecount++;
    validcount++;
}



//
// R_RenderView
//
void R_RenderPlayerView (player_t* player)
{       
    // PRINTF("Setup start ... \n");
    R_SetupFrame (player);

    // Clear buffers.
    R_ClearClipSegs ();
    R_ClearDrawSegs ();
    R_ClearPlanes ();
    R_ClearSprites ();
    // PRINTF("finish\n");
    
    // check for new console commands.
    NetUpdate ();

    // The head node is the last node output.
    // PRINTF("R_RenderBSPNode start ... \n");
    R_RenderBSPNode (numnodes-1);
    // PRINTF("finish\n");
    
    // Check for new console commands.
    NetUpdate ();
    
    // PRINTF("R_DrawPlanes start ... \n");
    R_DrawPlanes ();
    // PRINTF("finish\n");
    
    // Check for new console commands.
    NetUpdate ();
    
    // PRINTF("R_DrawMasked start ...\n");
    R_DrawMasked ();
    // PRINTF("finish\n");

    // Check for new console commands.
    NetUpdate ();                               
}
