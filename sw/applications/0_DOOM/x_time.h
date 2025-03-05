#ifndef X_TIME_H
#define X_TIME_H

#include <stdint.h>

void X_start_time(void);
void X_stop_time(void);
uint32_t X_get_time(void);
uint32_t X_time_in_secs(uint32_t ticks);
uint32_t X_time_in_msecs(uint32_t ticks);

void X_milli_delay(int n_milli_seconds);
void X_micro_delay(int n_milli_seconds);

#endif // X_TIME_H
