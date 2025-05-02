//
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
//       Joystick code.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "doomtype.h"
#include "d_event.h"
#include "i_joystick.h"
#include "i_system.h"

#include "m_config.h"
#include "m_misc.h"

// When an axis is within the dead zone, it is set to zero.
// This is 5% of the full range:

#define DEAD_ZONE (32768 / 3)

/* NRFD-TODO

static void *joystick = NULL;

// Configuration variables:

// Standard default.cfg Joystick enable/disable

static int usejoystick = 0;

// SDL GUID and index of the joystick to use.
static char *joystick_guid = "";
static int joystick_index = -1;

// Which joystick axis to use for horizontal movement, and whether to
// invert the direction:

static int joystick_x_axis = 0;
static int joystick_x_invert = 0;

// Which joystick axis to use for vertical movement, and whether to
// invert the direction:

static int joystick_y_axis = 1;
static int joystick_y_invert = 0;

// Which joystick axis to use for strafing?

static int joystick_strafe_axis = -1;
static int joystick_strafe_invert = 0;

// Which joystick axis to use for looking?

static int joystick_look_axis = -1;
static int joystick_look_invert = 0;

// Virtual to physical button joystick button mapping. By default this
// is a straight mapping.
static int joystick_physical_buttons[NUM_VIRTUAL_BUTTONS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};
void I_ShutdownJoystick(void)
{
    if (joystick != NULL)
    {
        joystick = NULL;
    }
}
*/

void I_InitJoystick(void)
{
    PRINTF("NRFD_TODO: I_InitJoystick\n");

    // I_AtExit(I_ShutdownJoystick, true);
}

void I_UpdateJoystick(void)
{
        PRINTF("NRFD_TODO: I_UpdateJoystick\n");
        /*

    if (joystick != NULL)
    {
        event_t ev;

        ev.type = ev_joystick;
        ev.data1 = GetButtonsState();
        ev.data2 = GetAxisState(joystick_x_axis, joystick_x_invert);
        ev.data3 = GetAxisState(joystick_y_axis, joystick_y_invert);
        ev.data4 = GetAxisState(joystick_strafe_axis, joystick_strafe_invert);
        ev.data5 = GetAxisState(joystick_look_axis, joystick_look_invert);

        D_PostEvent(&ev);
    }
        */
}

void I_BindJoystickVariables(void)
{
    int i;
    PRINTF("NRFD-TODO: I_BindJoystickVariables\n");/*
    M_BindIntVariable("use_joystick",          &usejoystick);
    M_BindStringVariable("joystick_guid",      &joystick_guid);
    M_BindIntVariable("joystick_index",        &joystick_index);
    M_BindIntVariable("joystick_x_axis",       &joystick_x_axis);
    M_BindIntVariable("joystick_y_axis",       &joystick_y_axis);
    M_BindIntVariable("joystick_strafe_axis",  &joystick_strafe_axis);
    M_BindIntVariable("joystick_x_invert",     &joystick_x_invert);
    M_BindIntVariable("joystick_y_invert",     &joystick_y_invert);
    M_BindIntVariable("joystick_strafe_invert",&joystick_strafe_invert);
    M_BindIntVariable("joystick_look_axis",    &joystick_look_axis);
    M_BindIntVariable("joystick_look_invert",  &joystick_look_invert);

    for (i = 0; i < NUM_VIRTUAL_BUTTONS; ++i)
    {
        char name[32];
        M_snprintf(name, sizeof(name), "joystick_physical_button%i", i);
        M_BindIntVariable(name, &joystick_physical_buttons[i]);
    }
    */
}

