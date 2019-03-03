/*****************************************************************
* File Name: stepper.c
* Description: This file is a stepper related function, includes
*             time controller, stepper controller, task controller.
* Date: 2018.11.14
* Author: Bob, Zhang
* E-mail: 156500117@qq.coom
*****************************************************************/

#include <reg52.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char bool;
#define false ( 0 )
#define true  ( !false )

// stepper pins
sbit steperA = P1^0;
sbit steperB = P1^1;
sbit steperA_ = P1^2;
sbit steperB_ = P1^3;

// count for 1 ms
#define TIMER_VAL_FOR_1MS (0xFC18)
int direction_change_flag = 0x00;

void delay(int aUs)
{
    static int i;
    static int us;

    us = aUs;

    while(us--)
    {
        i = 11;
        while(--i);
    }
}

// stepper left turn
void MotorLeft()
{
    static int i = 3;
   
    switch( i-- )
    {
        case 0:
				    steperA = 1;
				    steperA_ = 0;
				    steperB = 0;
				    steperB_ = 0;
				    break;
        case 1:
				    steperA = 0;
				    steperA_ = 0;
				    steperB = 1;
				    steperB_ = 0;
				    break;
        case 2:
    				steperA = 0;
	    			steperA_ = 1;
		    		steperB = 0;
			    	steperB_ = 0;
				    break;
        case 3:
				    steperA = 0;
				    steperA_ = 0;
				    steperB = 0;
				    steperB_ = 1;
			    	break;
        default:
				    break;
    }
   
    if( -1 >= i )
        i = 3;
 }

// stepper right turn
void MotorRight()
{
    static int i = 0;
   
    switch( i++ )
    {
        case 0:
				    steperA = 1;
				    steperA_ = 0;
				    steperB = 0;
				    steperB_ = 0;
				    break;
        case 1:
				    steperA = 0;
				    steperA_ = 0;
				    steperB = 1;
				    steperB_ = 0;
				    break;
        case 2:
    				steperA = 0;
	    			steperA_ = 1;
		    		steperB = 0;
			    	steperB_ = 0;
				    break;
        case 3:
				    steperA = 0;
				    steperA_ = 0;
				    steperB = 0;
				    steperB_ = 1;
			    	break;
        default:
				    break;
    }
   
    if( 4 <= i )
        i = 0;
}

// init timer
void InitTimer()
{
	   TMOD = 0x01;
	   TH0 = TIMER_VAL_FOR_1MS >> 8;	// 1 ms
	   TL0 = TIMER_VAL_FOR_1MS & 0xFF;
	   ET0 = 0x01;
	   EA = 0x01;
	   TR0 = 0x01;
}

// state machine
typedef uint8_t task_state_t; enum
{
	TASK_STATE_INIT_TIMER_STATE=0,
	TASK_STATE_FEED_FISH_MOTOR_LEFT_STATE,
	TASK_STATE_FEED_FISH_MOTOR_RIGHT_STATE,
	TASK_STATE_WAIT_STATE,
};

#define FEED_FISH_TIME_PERIOD 3 //( 8*60*60 ) // 8h
static const uint32_t feed_fish_period = FEED_FISH_TIME_PERIOD; // 8h
static const uint32_t feed_first_period = FEED_FISH_TIME_PERIOD + 4; // 4s
static const uint32_t feed_second_period = FEED_FISH_TIME_PERIOD + 4 + 4; // 4s

static task_state_t next_state = TASK_STATE_INIT_TIMER_STATE;

void TaskLoop()
{
	switch(next_state)
	{
		case TASK_STATE_INIT_TIMER_STATE:
			InitTimer();
			next_state = TASK_STATE_WAIT_STATE;
		break;
		case TASK_STATE_FEED_FISH_MOTOR_LEFT_STATE:
            MotorLeft();
		break;
		case TASK_STATE_FEED_FISH_MOTOR_RIGHT_STATE:
            MotorRight();
		break;
		case TASK_STATE_WAIT_STATE:
		break;
	}
}

void StateLoop()
{
	static uint32_t fedd_fish_time_count = FEED_FISH_TIME_PERIOD;

	if( fedd_fish_time_count == feed_fish_period )
	{
		next_state = TASK_STATE_FEED_FISH_MOTOR_LEFT_STATE;
	}
	else if( fedd_fish_time_count == feed_first_period )
	{
		next_state = TASK_STATE_FEED_FISH_MOTOR_RIGHT_STATE;
	}
	else if( fedd_fish_time_count == feed_second_period )
	{
		next_state = TASK_STATE_WAIT_STATE;
		fedd_fish_time_count = 0;
	}

	++fedd_fish_time_count;
}

// timer interrupt function
#define STATE_CHECK_TIME ( 1000 )
void TimerIrq( void ) interrupt 1 using 1
{
	static long time_count = 0;

	++time_count;

	// every second run once state check
	if( STATE_CHECK_TIME == time_count )
	{
		StateLoop();
		time_count = 0;
	}

	TH0 = TIMER_VAL_FOR_1MS >> 8;	// 1 ms
	TL0 = TIMER_VAL_FOR_1MS & 0xFF;
	
}

void main()
{	
    do
    {
		TaskLoop();
        delay(100);
    } while( 1 );
}