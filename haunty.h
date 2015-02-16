// HAUNTY.H
#ifndef HAUNTY_H
#define HAUNTY_H
     
#define PIN0	RPI_V2_GPIO_P1_23	//GPIO11
#define PIN1	RPI_V2_GPIO_P1_21	//GPIO09
#define PIN2	RPI_V2_GPIO_P1_19	//GPIO10
#define PIN3	RPI_V2_GPIO_P1_15	//GPIO22
#define PIN4	RPI_V2_GPIO_P1_26	//GPIO07
#define PIN5	RPI_V2_GPIO_P1_24	//GPIO08
#define PIN6	RPI_V2_GPIO_P1_22	//GPIO25
#define PIN7	RPI_V2_GPIO_P1_18	//GPIO24

#define NUM_PINS	8

#define START	1
#define STOP	0

#define TRUE	1
#define FALSE	0

#define NUM_TIMERS	10
#define NUM_BITS	10

enum ttypes { input, bit, timer };
enum atypes { bitset, timerset, output, audio, video, reset };

//int pins[]={PIN0,PIN1,PIN2,PIN3,PIN4,PIN5,PIN6,PIN7};

typedef struct {
	int pin;
	int isInput;
	int isOutput;
} IO;

IO pins[]={
	{ PIN0,0,0 },
	{ PIN1,0,0 },
	{ PIN2,0,0 },
	{ PIN3,0,0 },
	{ PIN4,0,0 },
	{ PIN5,0,0 },
	{ PIN6,0,0 },
	{ PIN7,0,0 },
};

typedef struct {
	int curr_val;
	int prev_val;
	int used_trigger;
	int used_action;
} BIT;

BIT bits[NUM_BITS];

typedef struct {
	time_t start_time;
	int running;
	int used_trigger;
	int used_action;
} TIMER; 

TIMER timers[NUM_TIMERS];

typedef struct {
	enum ttypes trigger_type;
	int trigger_param;
	int	trigger_value;
	int trigger_once;
	enum atypes action_type;
	int action_param;
	int action_value;
	char *action_extra;
	int triggered;
} STEP;
     
#endif