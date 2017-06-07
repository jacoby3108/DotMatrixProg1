/*
 *  Joystick and Display Driver Test Bench
 *
 * Copyright (c) 2017  Catedra Programacion 1 ITBA GEDA
 *
 *  Author(s):
 * 
 *  Nicolas Magliola
 *  Clementina Calvo
 *  Daniel Jacoby
 * 
 * 
 * Compile: 
 * gcc testall.c joydrv.o disdrv.o termlib.o
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "termlib.h"
#include "joydrv.h"
#include "disdrv.h"

 
#define JOY_AXIS_INVERT J_INV_TRUE 

#define JOY_THRESHOLD 40     // 10-100 Joystick threshold (sensitivity)

int main(int argc, char *argv[])  //Sample main Test Bench 
{
	int ret=0;
	int i,j;

	jcoord_t joy_coordinates1;
	jswitch_t joy_switch1=J_NOPRESS;
	int8_t posx,posy,last_posx,last_posy;
	
	clrscr();					//Termlib Clear Screen (see termlib.h for more info)
	
	
	joy_init();					// Initialize Joystick Hardware
	display_init();				// Initialize Display Hardware 
	
	
	set_joy_axis(JOY_ROTATE);	// Set joystick axis orientaton  
	
	set_display_axis(NORMAL);	// Set display axis orientation 
	
	set_joy_direction(J_INV_TRUE,J_INV_TRUE); // Invert both x and y joystick axis direction
	
	for(i=0;i<16;i++)			// All Dots on (write to display buffer)
		for(j=0;j<16;j++)
			display_write(i,j,D_ON);
			
	display_update();			// Send display buffer to display 
	
//  Wait until joystick switch is presed to start	

	while(joy_switch1==J_NOPRESS) 
	{ 
		joystick_update();						 // Read joystick Hardware
		joy_switch1=joystick_get_switch_value(); // And get switch value
	}

	display_clear();			// Clears Display buffer and Hardware Display 
   
	posx=0;						// Default initial position
	posy=0;
	
	forever
	{
	
	joystick_update();         // Read joystick Hardware
		
	joy_coordinates1=joystick_get_coord();  //And get coordinate values



//  Convert analog to digital based on simetrical threshold for both axis
//  (-) -----------||dead zone||----------- (+)     <-- Joystcick position
//  Advance forward or backward (Meaning: || --> Thresholds , ----- active zone)   
	
	if(joy_coordinates1.x > JOY_THRESHOLD )
				posx++;
	else if(joy_coordinates1.x < -JOY_THRESHOLD )
				posx--;
				
	if(joy_coordinates1.y > JOY_THRESHOLD )
				posy++;
	else if(joy_coordinates1.y < -JOY_THRESHOLD )
				posy--;			
				
				
// Saturate if necessary to stay inside of the visible area of the display
				
	if (posx > MAX_X) 
		posx =15;			//Saturate
	if (posy > MAX_Y) 
		posy =15;			//Saturate
		
	if (posx < MIN_X) 
		posx =0;			//Saturate
	if (posy < MIN_Y) 
		posy =0;			//Saturate
	
	
	printf(GREEN_TEXT "posx: %d posy: %d joyx: %d joyy: %d \n",posx,posy,joy_coordinates1.x,joy_coordinates1.y);
	//usleep(200*1000); // 200ms 
	
	display_write(last_posx,last_posy,D_OFF);  // Turn off last dot 
	display_write(posx,posy,D_ON);			   // Turn on actual dot
  
    
    last_posx=posx;							  // Keep last coordinate 
    last_posy=posy;


    display_update();					// Reflect changes on display matrix
	
	}
	return ret;							// Thats all folks!!

}
