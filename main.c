/*
 * main.c
 *
 *  Created on: Mar 8, 2020
 *      Author: Yaswanth
 */

#include "HPS_PrivateTimer/HPS_PrivateTimer.h"
#include "HPS_Watchdog/HPS_Watchdog.h"
#include <stdlib.h>

//global variables
volatile unsigned int *display7Segment1= (unsigned int *)0xFF200020;
volatile unsigned int *display7Segment2= (unsigned int *)0xFF200030;
const unsigned int SUCCESS=0;

void exitOnFail(signed int status, signed int successStatus){
    if (status != successStatus) {
        exit((int)status); //Add breakpoint here to catch failure
    }
}

signed int conversion(unsigned int x)
{
	switch(x){
		case 0: return 0x3F;
		case 1: return 0x06;
		case 2: return 0x5B;
		case 3: return 0x4F;
		case 4: return 0x66;
		case 5: return 0x6D;
		case 6: return 0x7D;
		case 7: return 0x07;
		case 8: return 0x7F;
		case 9: return 0x67;
		default: return 0x00;
	}
}

//function to display time
signed int display(unsigned int hours, unsigned int minutes, unsigned int seconds){
    *display7Segment1=(conversion(minutes/10)<<24 | conversion(minutes%10)<<16 | conversion(seconds/10)<<8 | conversion(seconds%10)<<0);
    *display7Segment2=(conversion(hours/10)<<8 | conversion(hours%10)<<0);
    return SUCCESS;
}

int main(void)
{
    //Red LEDs base
    volatile unsigned int *LEDR_ptr = (unsigned int *) 0xFF200000;
    //KEY buttons base
    volatile unsigned int *KEY_ptr  = (unsigned int *) 0xFF200050;
    //Switch base
    volatile unsigned int *SWITCH_ptr  = (unsigned int *) 0xFF200040;
    //Local Variables
    unsigned int alarmHours=0;
    unsigned int alarmMinutes=0;
    unsigned int final_timer_value;
    unsigned int alarm=0;
    unsigned int alarmOnTime=0;
    unsigned int snoozeTime=0;
    unsigned int snoozeStart=0;
    unsigned int keyInput;
    unsigned int flag=0;
    unsigned int alarmFlag=0;
    unsigned int snoozeFlag=0;
    unsigned int snoozeCount=1;
    unsigned int alarmSwitch;
    unsigned int alarmStartTime=0;
    unsigned int taskID;
    const unsigned int taskCount=3;
    unsigned int taskInterval[taskCount]={900000,54000000,3240000000};
    unsigned int clockTime[taskCount]={0,0,0};
    unsigned int last_timer_value[taskCount];
    for (taskID = 2; taskID != -1; taskID--) {
    	last_timer_value[taskID] = 0xFFFFFFFF;      //All tasks start now
    }
    HPS_ResetWatchdog();
    *LEDR_ptr=0x0;
    display(0,0,0);

    exitOnFail(
       		Timer_initialise(0xFFFEC600),  //Initialise Timer
            TIMER_SUCCESS);                //Exit if not successful
    exitOnFail(
       		set_load(0xFFFFFFFF),  		   //Set timer load value
            TIMER_SUCCESS);                //Exit if not successful
    exitOnFail(
           		set_prescaler(249),  	   //Set timer prescaler value
                TIMER_SUCCESS);            //Exit if not successful
    exitOnFail(
       		set_control(3),  			   //Set control bits to start timer
            TIMER_SUCCESS);                //Exit if not successful
    //Wait a moment
    HPS_ResetWatchdog();
    while(1)
    {
    	final_timer_value=read_current_timer();
    	// Set N to KEY[3:0] input value.
    	keyInput = *KEY_ptr & 0x0F;
    	alarmSwitch=*SWITCH_ptr & 0x01;
    	//when keys are not pressed
    	if(keyInput==0x00){
    		flag=0;
    	}
    	//to increment minutes
    	else if(keyInput==0x01 && flag==0){
    		clockTime[1]=clockTime[1]+1;
 	   		last_timer_value[2]=last_timer_value[2]+54000000;  //sets hours accurately when minutes are manually changed
    		flag=1;
    	}
    	//to increment hours
 	   	else if(keyInput==0x02 && flag==0){
       		clockTime[2]=clockTime[2]+1;
       		flag=1;
       	}
    	//to set clock
 	   	else if(keyInput==0xC && alarmFlag==1)
 	   	{
 	   		alarmFlag=0;
 	   	}
    	//to increment minutes of alarm time
 	    else if(keyInput==0xD && alarmFlag==0)
 	    {
 	    	alarmMinutes=alarmMinutes+1;
 	    	alarmFlag=1;
 	    	if(alarmMinutes>=60){
 	    	   	alarmMinutes-=60;
 	        }
 	   	}
    	//to increment hours of alarm time
 	    else if(keyInput==0xE && alarmFlag==0)
 	    {
  	    	alarmHours=alarmHours+1;
  	    	alarmFlag=1;
  	    	if(alarmHours>=24){
  	    		alarmHours-=24;
  	    	}
 	    }
    	//turns off snooze and alarm
    	if(alarmSwitch==0x0){
    		snoozeFlag=0;
    		snoozeStart=0;
    		snoozeCount=1;
    		alarm=0;
    		alarmOnTime=0;
    		snoozeTime=0;
    		*LEDR_ptr=0;
    	}
        HPS_ResetWatchdog();
        // Check if it is time to increment time
        for (taskID = 2; taskID !=-1; taskID--) {
               if ((last_timer_value[taskID] - final_timer_value) >= taskInterval[taskID]) {
                   	clockTime[taskID]+=1;
                   	last_timer_value[taskID] = last_timer_value[taskID] - taskInterval[taskID];
                    if(clockTime[2]>=24){			//24 hour clock is displayed
                       	clockTime[2]-=24;
                    }
                    if(clockTime[1]>=60){			//Minutes are set to 0 after 59
                      	clockTime[1]-=60;
                    }
                    //to verify if alarm time is equal to clock time after every minute
                    if(clockTime[0]%60==0){
                    	if(clockTime[2]==alarmHours && clockTime[1]==alarmMinutes){  //alarm time is verified
                    		alarm=1;
                    		alarmStartTime=clockTime[0];
                    	}
                    	if(clockTime[0]==(alarmStartTime+300*snoozeCount) && snoozeFlag==1){			 //snooze time is verified
                    	    snoozeStart=1;
                    	}
                    }
                    //this part is executed every second
                    if(taskID==0){
                    	if(alarm==1){						//LEDs blink
                    		*LEDR_ptr=~*LEDR_ptr;
                    		alarmOnTime+=1;
                    	}
                    	if(alarmOnTime>=60){				//executed if alarm is not turned off manually
                    		    alarm=0;
                    		    alarmOnTime=0;
                    		    *LEDR_ptr=0;
                    		    snoozeFlag=1;
                    	}
                    	if(snoozeStart==1){					//blinks LEDs when snoozing
                    		*LEDR_ptr=~*LEDR_ptr;
                    		snoozeTime=snoozeTime+1;
                    		if(snoozeTime>=60){
                    			snoozeCount+=1;
                    			snoozeTime=0;
                    			*LEDR_ptr=0;
                    			snoozeStart=0;
                    		}
                    	}
                    	while(snoozeFlag==0 && clockTime[0]>=86400){		//to avoid overflow
                    	       	clockTime[0]-=86400;
                    	}
                    }
                }
        }
        HPS_ResetWatchdog();
        if(keyInput<0xC){																	//displays clock time
        	exitOnFail(
        	       		display(clockTime[2],clockTime[1],clockTime[0]%60),
						SUCCESS);
        }
        else{																		//displays alarm time
        	exitOnFail(
        			display(alarmHours,alarmMinutes,0),
        							SUCCESS);
        }
        HPS_ResetWatchdog();
    }
}
