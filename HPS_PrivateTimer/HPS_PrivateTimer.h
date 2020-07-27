#ifndef HPS_PrivateTimer_H_

#define HPS_PrivateTimer_H_
#include <stdbool.h>
#define TIMER_SUCCESS		0
#define TIMER_ERRORNOINIT   -1
signed int Timer_initialise(unsigned int base_address);
bool Timer_isInitialised(void);
signed int set_prescaler(unsigned int prescaler_value);
signed int set_load(unsigned int load_value);
signed int set_control(unsigned int control_value);
signed int clear_interrupt(void);
unsigned int read_current_timer(void);
unsigned int read_interrupt_flag(void);


#endif
