#ifndef X_BUTTONS_H
#define X_BUTTONS_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize all buttons (configure GPIO, setup interrupts)
void X_ButtonsInit(void);

// (Optional) Manually read buttons (not used if interrupts are working)
void X_ReadButtons(void);

// Get the current state of a button (pressed or not)
// 0 = pressed, 1 = released (inverse logic!)
int X_ButtonState(int num);

// Get the raw GPIO read of a button (optional)
int X_ButtonStateRaw(int id);

void button_up_handler(void);
void button_down_handler(void);
void button_left_handler(void);
void button_right_handler(void);
void button_a_handler(void);
void button_b_handler(void);

//static void button_post_event(int button_idx);

#ifdef __cplusplus
}
#endif

#endif // X_BUTTONS_H