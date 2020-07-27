#include "HPS_PrivateTimer.h"
//Driver Base Addresses 
volatile unsigned int *timer_base_ptr = 0x0;	 //0xFFFEC600

//Driver Initialised 
bool timer_initialised = false; 
//Register Offsets 
#define TIMER_LOAD  		(0x00/sizeof(unsigned int))
#define TIMER_VALUE			(0x04/sizeof(unsigned int))
#define TIMER_CONTROL		(0x08/sizeof(unsigned int))
#define TIMER_INTERRUPT		(0x0C/sizeof(unsigned int))
//Function to initialise the Timer 
signed int Timer_initialise(unsigned int base_address){
	//Initialise base address pointers
	timer_base_ptr = (unsigned int *) base_address;
	//Ensure timer initialises to disabled
	timer_base_ptr[TIMER_CONTROL] = 0;
	//Timer now initialise
	timer_initialised = true;
	return TIMER_SUCCESS;
}
//Check if driver initialised
bool Timer_isInitialised() {
	return timer_initialised;
}
//Function to set prescaler value
signed int set_prescaler(unsigned int prescaler_value) {
	volatile unsigned int *timer_ptr;
	unsigned int currentVal;
	//Sanity checks
	if(!Timer_isInitialised()) return TIMER_ERRORNOINIT;
	currentVal = *timer_base_ptr;
	timer_ptr=(unsigned int*)&currentVal;
	timer_ptr[TIMER_CONTROL]|=(prescaler_value<<8);
	timer_base_ptr[TIMER_CONTROL]|=timer_ptr[TIMER_CONTROL];
	return TIMER_SUCCESS;
}
//Function to set load value
signed int set_load(unsigned int load_value) {
	if(!Timer_isInitialised()) return TIMER_ERRORNOINIT;
	timer_base_ptr[TIMER_LOAD]=load_value;
	return TIMER_SUCCESS;
}
//Function to set bits I,A,E
signed int set_control(unsigned int control_value) {
	volatile unsigned int *timer_ptr;
	unsigned int currentVal;
	//Sanity checks
	if(!Timer_isInitialised()) return TIMER_ERRORNOINIT;
	currentVal = *timer_base_ptr;
	timer_ptr=(unsigned int*)&currentVal;
	timer_ptr[TIMER_CONTROL]&=~0x7;
	timer_ptr[TIMER_CONTROL]|=(control_value&0x7);
	timer_base_ptr[TIMER_CONTROL]|=timer_ptr[TIMER_CONTROL];
	return TIMER_SUCCESS;
}
//Function to read counter register value
unsigned int read_current_timer(void) {
	volatile unsigned int *timer_ptr;
	unsigned int currentVal;
	//Sanity checks
	if(!Timer_isInitialised()) return TIMER_ERRORNOINIT;
	currentVal = *timer_base_ptr;
	timer_ptr=(unsigned int*)currentVal;
	return timer_base_ptr[TIMER_VALUE];
}


 
