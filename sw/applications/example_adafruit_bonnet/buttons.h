#ifndef X_BUTTONS_H
#define X_BUTTONS_H

// Include any necessary headers here

// Define any constants or macros here

// Declare any global variables here
extern bool button_state[6];
extern bool button_posedge[6];
extern bool button_negedge[6];

// Declare any function prototypes here
void buttonsInit(void);
int buttonState(int num);
void readButtons(void);

#endif // X_BUTTONS_H
