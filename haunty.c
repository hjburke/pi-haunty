// HAUNTY.C
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <bcm2835.h>
#include "haunty.h"

STEP steps[] = {
	{input,0, 0,TRUE, bitset,   0,1,"",0},	// Relay is NO, with pullup, goes low when relay energized
	{bit,  0, 1,FALSE,timerset, 0,START,"",0},
	
	{timer,0, 0,TRUE, video,    0,0,"/home/pi/Creepy-Crawlies-Dvd-rats.m4v",0},
	
//	{timer,0, 2,TRUE, output,   7,0,"",0},	// Red spotlight off
	
	{timer,0,30,TRUE, reset,    0,0,"",0}
};

char *videos[]={
"/home/pi/Creepy-Crawlies-Dvd-rats.m4v",
"/home/pi/Creepy-Crawlies-Dvd-roaches.m4v",
"/home/pi/Creepy-Crawlies-Dvd-snakes.m4v",
"/home/pi/Creepy-Crawlies-Dvd-spiders.m4v"
};

int numsteps=sizeof(steps)/sizeof(STEP);

int checkSteps() {
	int n;

	// First check over the steps to see what we are using
	for (n=0; n<numsteps; n++) {
		int tparam=steps[n].trigger_param;
		int aparam=steps[n].action_param;
		
		switch (steps[n].trigger_type) {
		case input:
			if (tparam<NUM_PINS) {
				pins[tparam].isInput=1;
			} else {
				printf("ERROR: Input %d out of range (range 0 to %d)\n", tparam, NUM_PINS-1);
				return EXIT_FAILURE;				
			}
			break;
		case bit:
			if (tparam<NUM_BITS) {
				bits[tparam].used_trigger=1;		
			} else {
				printf("ERROR: Bit %d out of range (range 0 to %d)\n", tparam, NUM_BITS-1);
				return EXIT_FAILURE;
			}
			break;
		case timer:
			if (tparam<NUM_TIMERS) {
					timers[tparam].used_trigger=1;			
			} else {
				printf("ERROR: Timer %d out of range (range 0 to %d)\n", tparam, NUM_TIMERS-1);
				return EXIT_FAILURE;
			}
			break;
		default:
			break;
		}
		
		switch (steps[n].action_type) {
		case output:
			if (tparam<NUM_PINS) {
				pins[aparam].isOutput=1;
			} else {
				printf("ERROR: Output %d out of range (range 0 to %d)\n", tparam, NUM_PINS-1);
				return EXIT_FAILURE;				
			}
			break;
		case bitset:
			if (tparam<NUM_BITS) {
				bits[aparam].used_action=1;			
			} else {
				printf("ERROR: Bit %d out of range (range 0 to %d)\n", tparam, NUM_BITS-1);
				return EXIT_FAILURE;
			}
			break;
		case timerset:
			if (tparam<NUM_TIMERS) {
				timers[aparam].used_action=1;			
			} else {
				printf("ERROR: Timer %d out of range (range 0 to %d)\n", tparam, NUM_TIMERS-1);
				return EXIT_FAILURE;
			}
			break;
		default:
			break;
		}		
	}
	
	// Check the bit registers
	int num_bits=0;
	for(n=0; n<NUM_BITS; n++) {
		if (bits[n].used_trigger==1 || bits[n].used_action==1) {
			num_bits++;
			if (bits[n].used_trigger > bits[n].used_action) {
				printf("WARNING: bit register %d used as trigger but not set with an action\n", n);
			} else if (bits[n].used_trigger < bits[n].used_action) {
				printf("WARNING: bit register %d set in action but not used as trigger\n", n);
			}
		}
	}	
	
	// Check the timers
	int num_timers=0;
	for(n=0; n<NUM_TIMERS; n++) {
		if (timers[n].used_trigger==1 || timers[n].used_action==1) {
			num_timers++;
			if (timers[n].used_trigger > timers[n].used_action) {
				printf("WARNING: timer %d used as trigger but not set with an action\n", n);
			} else if (timers[n].used_trigger < timers[n].used_action) {
				printf("WARNING: timer %d set in action but not used as trigger\n", n);
			}
		}
	}
	
	// Display IO Usage
	for(n=0; n<NUM_PINS; n++) {
		printf("Pin %d", n);
		if (pins[n].isInput) { printf(" INPUT"); }
		if (pins[n].isOutput) { printf(" OUTPUT"); }
		if (!pins[n].isInput && !pins[n].isOutput) { printf(" FREE"); }
		printf("\n");
	}
	
	// Print usage statistics
	printf("There are %d elements in the array\n",numsteps);
	printf("Total number of bit registers used %d\n", num_bits);
	printf("Total number of timers used %d\n", num_timers);
	
	return EXIT_SUCCESS;
}

int doAction(int stepnum) {
	int aparam=steps[stepnum].action_param;
	int avalue=steps[stepnum].action_value;

	// Record that this step was triggered (at least once)
	steps[stepnum].triggered++;
	
	switch (steps[stepnum].action_type) {
	case bitset:
		if (bits[aparam].curr_val!=avalue) {
			printf("ACTION: Setting bit %d to %d (prev %d)\n",aparam,avalue,bits[aparam].prev_val);
			bits[aparam].prev_val=bits[aparam].curr_val;
			bits[aparam].curr_val=avalue;
		}
		break;
	case timerset:
		if (avalue == START) {
			printf("ACTION: Starting timer %d\n",aparam);
			timers[aparam].start_time = time(NULL);
			timers[aparam].running = 1;
		} else {
			printf("ACTION: Stopping timer %d\n",aparam);
			timers[aparam].running = 0;		
		}
		break;
	case output:
		printf("ACTION: Setting output %d (PIN %d) to %d\n", aparam, pins[aparam].pin, avalue);
		if (avalue == 1) {
			bcm2835_gpio_set(pins[aparam].pin);
		} else {
			bcm2835_gpio_clr(pins[aparam].pin);
		} 
		break;
	case audio:
		{
			char *s = NULL;
			printf("ACTION: Playing audio clip %s\n", steps[stepnum].action_extra);
			asprintf(&s, "/usr/bin/aplay -q %s &", steps[stepnum].action_extra);
			system(s);
			free(s);
		}
		break;
	case video:
		{
			char *s = NULL;
			int r = rand() % 5;
			printf("ACTION: Playing video clip %s\n", videos[r]);
			asprintf(&s, "/usr/bin/omxplayer %s --win \"0 0 700 470\" > /dev/null &", videos[r]);
			system(s);
			free(s);
		}
		break;
	case reset:
		{
			int n;
			printf("ACTION: Resetting triggered flags,timers & bits\n");
			
			for (n=0; n<numsteps; n++) {		
				steps[n].triggered=0;
			}			
		
			for (n=0;n<NUM_TIMERS;n++) {
				timers[n].running = 0;
			}
		
			for (n=0;n<NUM_BITS;n++) {
				bits[n].curr_val = 0;
				bits[n].prev_val = 0;
			}		
		}
		break;
	default:
		printf("ACTION: Unknown action\n");
		break;	
	}
}

int main() {	

	if (checkSteps() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	// Now we can start for real
	
	// Set up the IO pins
	if (!bcm2835_init())
		return 1;

	int n;
	for(n=0; n<NUM_PINS; n++) {
		if (pins[n].isInput) { 
			// Set PIN to be an input
			bcm2835_gpio_fsel(pins[n].pin, BCM2835_GPIO_FSEL_INPT);
			//  with a pullup
			bcm2835_gpio_set_pud(pins[n].pin, BCM2835_GPIO_PUD_UP);
		} else if (pins[n].isOutput) { 
			// Set PIN to be an output
			bcm2835_gpio_fsel(pins[n].pin, BCM2835_GPIO_FSEL_OUTP);
		}
	}
	
	for (;;) {
		int value;
		int ovalue;
		time_t now;
		int stepnum;
		
		for (stepnum=0; stepnum<numsteps; stepnum++) {		
			int tparam=steps[stepnum].trigger_param;
			int tvalue=steps[stepnum].trigger_value;
			int tonce=steps[stepnum].trigger_once;
			int triggered=steps[stepnum].triggered;
			
			switch (steps[stepnum].trigger_type) {
			case input:
				value = bcm2835_gpio_lev(pins[tparam].pin);
				if (value == tvalue && ((tonce && !triggered) || (!tonce))) {
					doAction(stepnum);
				}
				break;
			case bit:
				value = bits[tparam].curr_val;
				ovalue = bits[tparam].prev_val;
				if (value == tvalue && ovalue != tvalue && ((tonce && !triggered) || (!tonce))) {
					doAction(stepnum);
					bits[tparam].prev_val = value;
				}		
				break;
			case timer:
				now=time(NULL);
				double time_elapsed = difftime(now,timers[tparam].start_time);
				if (timers[tparam].running && tvalue <= time_elapsed && ((tonce && !triggered) || (!tonce))) {
					doAction(stepnum);									
				}
				break;
			default:
				printf("Unknown command\n");			
			}
		}
	}
	
    bcm2835_close();
    return 0;
}