//********************************************************************
//*                    EEE2046F/EEE2050F C template                  *
//*==================================================================*
//* WRITTEN BY:    	                 		             *
//* DATE CREATED:                                                    *
//* MODIFIED:                                                        *
//*==================================================================*
//* PROGRAMMED IN: Eclipse Luna Service Release 1 (4.4.1)            *
//* TARGET:    PC or STM32F0?                                        *
//*==================================================================*
//* DESCRIPTION:                                                     *
//*                                                                  *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include "stm32f0xx.h"
#include <stdlib.h>
#include "stdio.h"
#include "stdint.h"
//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================
#define sw0		 GPIO_IDR_0		//Bitmask for switch 0
#define sw1      GPIO_IDR_1     //Bitmask for switch 1
#define PB3      GPIO_IDR_3
#define PB4      GPIO_IDR_4
#define PB5      GPIO_IDR_5
#define PB6      GPIO_IDR_6
#define PB7      GPIO_IDR_7
#define PB8      GPIO_IDR_8
#define PA1      GPIO_ODR_1
#define PA2      GPIO_ODR_2
#define PA3      GPIO_ODR_3
#define PA4      GPIO_ODR_4
//====================================================================
// GLOBAL VARIABLES
//====================================================================
char start = 0;					//For starting switch
char sensor_state;				//Current sensor state
char Path[100];					//Array to record all turns taken
int ptr = 0;					//Initialize pointer to record current sensor states in Path array
char direction;                 // direction for path array

//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void init_Ports(void);
void get_sensor_status(void);
void delay(int a);
void move_Forward(void);
void turn_Left(void);
void turn_Right(void);
void U_turn(void);
void stop(void);
void shorten_path(void);
//====================================================================
// MAIN FUNCTION
//====================================================================

void main(void)
{
	init_Ports();

	//wait for start button to be pushed
    while(1){							//infinite loop to keep checking start switch(sw0)
		if(!(GPIOA->IDR & sw0)){		//sw0 pushed
			start = 1;					//start path finder algorithm
			delay(100000);
		}
	}

	//start once sw0 has been pressed
	while(start == 1){
		get_sensor_status();
		move_Forward();					//Default function is move Forward

		for(int i = 0;i < 4; i++){		//Prevents more than 4 consecutive left turns in a row
			switch(sensor_state)		//Switch case for sensor states commands
			{
			case 'R':						//Right Turn Only detected
				turn_Right();				//Call function to turn robot right

				Path[ptr] = direction;	    //Record the direction in the Path array
				if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the Path array
				i = 0;						//restart left turn counter
				break;
			case 'I':						//Right T Junction detected
				move_Forward();				//Call function to move robot forward

				Path[ptr] = direction;	    //Record the direction in the Path array
                if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the Path array
				i = 0;						//restart left turn counter
				break;
			case 'U':						//Dead end detected
				U_turn();					//Call function to turn robot around

				Path[ptr] = direction;	    //Record the direction in the Path array
				ptr++;						//increment pointer to the next element in the Path array
				i = 0;						//restart left turn counter
				break;
			case 'E':						//End of maze detected
				stop();
				U_turn();					//Turn robot around
				get_shortest_path();		//Find the shortest path home.(No idea how we're going to do this!)
				break;
//Left turn only junctions
			case 'T':						//T Junction detected
				turn_Left();				//Call function to turn robot left

				Path[ptr] = direction;	    //Record the direction in the Path array
                if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the array
				i++;						//increment left turn counter
				break;
			case 'J':						//Left T Junction detected
				turn_Left();				//Call function to turn robot left

				Path[ptr] = direction;	    //Record the direction in the Path array
                if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the array
				i++;						//increment left turn counter
				break;
			case 'L':						//Left Turn detected
				turn_Left();				//Call function to turn robot left

				Path[ptr] = direction;	    //Record the direction in the Path array
                if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the array
				i++;						//increment left turn counter
				break;
			case 'X':						//cross intersection detected
				turn_Left();				//Call function to turn robot left

				Path[ptr] = direction;	    //Record the direction in the Path array
                if(Path[ptr-1] == 'U')
                {
                    shorten_path();
				}
				ptr++;						//increment pointer to the next element in the array
				i++;						//increment left turn counter
				break;
			}								//close switch case
		}									//close for loop

//If robot has made more than 4 consecutive left turns then do the following:
        ptr-=4;                         //Move to the element containing the 1st left turn
		i=0;                            //Reset counter
		if(sensor_state == 'T')			//T junction detected
		{
			turn_Right();				//Call function to turn robot right

			Path[ptr] = direction;	    //Record the direction in the Path array (overrides 1st left turn)
			ptr++;						//increment pointer to the next element in the array
		}
		else if(sensor_state == 'J')	//Left T junction detected
		{
			move_Forward();				//Call function to move robot forward.
			Path[ptr] = direction;	    //Record the direction in the Path array (overrides 1st left turn)
			ptr++;						//increment pointer to the next element in the array
		}
		else if(sensor_state == 'X')	//cross intersection detected
		{
			move_Forward();				//Call function to move robot forward.
			Path[ptr] = direction;	    //Record the direction in the Path array (overrides 1st left turn)
			ptr++;						//increment pointer to the next element in the array
		}
	}


}
// End of main
void init_Ports(void){

//connect clocks for port A and B
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		//Enable clock for port A
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		//Enable clock for port B

//INPUTS
//Start switch set up
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);	//Set PA0 for start switch (sw0) (solve maze)
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0;	//Enable pull up resister for start switch SW0

//Set pins to input for sensors
	GPIOB->MODER &= ~ (GPIO_MODER_MODER3_0|  //PB3 to input for sensor LL
					   GPIO_MODER_MODER4_0|  //PB4 to input for sensor L
					   GPIO_MODER_MODER5_0|  //PB5 to input for sensor |
					   GPIO_MODER_MODER6_0|  //PB6 to input for sensor R
					   GPIO_MODER_MODER7_0|	 //PB7 to input for sensor RR
					   GPIO_MODER_MODER8_0); //PB8 to input for FINISH sensor

//OUTPUTS
//Set pins to output for motor driver
	GPIOA->MODER |= GPIO_MODER_MODER1_0; 	//PA1 to output (input 1 of motor driver (left motor))
	GPIOA->MODER |= GPIO_MODER_MODER2_0; 	//PA2 to output (input 2 of motor driver (left motor))
	GPIOA->MODER |= GPIO_MODER_MODER3_0; 	//PA3 to output (input 3 of motor driver (right motor))
	GPIOA->MODER |= GPIO_MODER_MODER4_0; 	//PA4 to output (input 3 of motor driver (right motor))

//Set pins for PWM
}
void get_sensor_status(void)
{
    if(((GPIOB_IDR & PB3) != 0)&&((GPIOB_IDR & PB4) != 0)&&((GPIOB_IDR & PB5) != 0)
       &&((GPIOB_IDR & PB6) != 0)&&((GPIOB_IDR & PB7) != 0)&&((GPIOB_IDR & PB8) != 0))
    {
        sensor_state = 'E'; //End detected
    }
    else if(((GPIOB_IDR & PB3) == 0)&&((GPIOB_IDR & PB4) == 0)&&((GPIOB_IDR & PB5) != 0)
            &&((GPIOB_IDR & PB6) == 0)&&((GPIOB_IDR & PB7) == 0))
    {
        sensor_state = 'F'; //Single line detected
    }
    else if(((GPIOB_IDR & PB3) != 0)&&((GPIOB_IDR & PB4) != 0)&&((GPIOB_IDR & PB5) != 0)
            &&((GPIOB_IDR & PB6) != 0)&&((GPIOB_IDR & PB7) != 0)&&((GPIOB_IDR & PB8) == 0))
    {
        sensor_state = 'X'; //Cross intersection detected
    }
    else if(((GPIOB_IDR & PB3) != 0)&&((GPIOB_IDR & PB4) != 0)&&((GPIOB_IDR & PB5) == 0)
            &&((GPIOB_IDR & PB6) != 0)&&((GPIOB_IDR & PB7) != 0)&&((GPIOB_IDR & PB8) == 0))
    {
        sensor_state = 'T'; //T junction detected
    }
    else if(((GPIOB_IDR & PB3) != 0)&&((GPIOB_IDR & PB4) != 0)&&((GPIOB_IDR & PB5) != 0)
            &&((GPIOB_IDR & PB6) == 0)&&((GPIOB_IDR & PB7) == 0))
    {
        sensor_state = 'J'; // Left T detected
    }
    else if(((GPIOB_IDR & PB3) == 0)&&((GPIOB_IDR & PB4) == 0)&&((GPIOB_IDR & PB5) != 0)
            &&((GPIOB_IDR & PB6) != 0)&&((GPIOB_IDR & PB7) != 0))
    {
        sensor_state = 'I'; // Right T detected
    }
    else if(((GPIOB_IDR & PB3) != 0)&&((GPIOB_IDR & PB4) != 0)&&((GPIOB_IDR & PB5) == 0)
            &&((GPIOB_IDR & PB6) == 0)&&((GPIOB_IDR & PB7) == 0))
    {
        sensor_state = 'L'; // Left Turn detected
    }
    else if(((GPIOB_IDR & PB3) == 0)&&((GPIOB_IDR & PB4) == 0)&&((GPIOB_IDR & PB5) == 0)
            &&((GPIOB_IDR & PB6) != 0)&&((GPIOB_IDR & PB7) != 0))
    {
        sensor_state = 'R'; // Right Turn detected
    }
    else if(((GPIOB_IDR & PB3) == 0)&&((GPIOB_IDR & PB4) == 0)&&((GPIOB_IDR & PB5) == 0)
            &&((GPIOB_IDR & PB6) == 0)&&((GPIOB_IDR & PB7) == 0)&&((GPIOB_IDR & PB8) == 0))
    {
        printf("no lines detected");
        sensor_state= 'E'; //Stop robot to check for issues
    }
    else
    {
        printf("pattern not recognized");
        sensor_state = 'E'; //Stop robot to check for issues
    }
}

void move_Forward(void)
{
    direction = 'F';
    GPIO->ODR = 0b10010; //Set PA1 and PA4 high; PA2 and PA3 low
    while(sensor_state=='F')
    {
        get_sensor_status();  //Breaks out of while loop when a junction/deadend is detected
    }
    GPIO->ODR = 0b00000; //Stop robot and go back to switch case with new sensor state
}

void turn_Left(void)
{
    direction = 'L';
    GPIO->ODR = 0b10000; // Set PA1,PA2,PA3 low and PA4 high
    delay(5000);         // Allows robot to turn for set amount of time
    while(sensor_state=='F')
    {
        GPIO->ODR = 0b10010;  // Make robot go forward
        get_sensor_status();  //Breaks out of while loop when a junction/deadend is detected
    }
    GPIO->ODR = 0b00000; //Stop robot
}

void turn_Right(void)
{
    direction = 'R';
    GPIO->ODR = 0b00010; // Set PA1 high; PA2,PA3 and PA4 low
    delay(5000);         // Allows robot to turn for set amount of time
    while(sensor_state=='F')
    {
        GPIO->ODR = 0b10010;  // Make robot go forward
        get_sensor_status();  //Breaks out of while loop when a junction/deadend is detected
    }
    GPIO->ODR = 0b00000; //Stop robot
}

void U_turn(void)
{
    direction = 'U';
    GPIO->ODR = 0b10100; // Set PA1, PA3 low and PA2, PA4 high
    delay(5000);         // Allows robot to turn for set amount of time
    while(sensor_state=='F')
    {
        GPIO->ODR = 0b10010;  // Make robot go forward
        get_sensor_status();  //Breaks out of while loop when a junction/deadend is detected
    }
    GPIO->ODR = 0b00000; //Stop robot
}

void stop(void)
{
    GIOP->ODR = 0b00000; //Set PA1, PA2, PA3 and PA4 low
}

void shorten_path(void)
{
    if((path[ptr-2]=='L') &&(path[ptr]=='R'))
    {
        ptr-=2;
        path[ptr]=='U';
    }
    else if((path[ptr-2]=='L' && (path[ptr]=='L'))
    {
        ptr-=2;
        path[ptr]=='F';
    }
    else if((path[ptr-2]=='R' && (path[ptr]=='L'))
    {
        ptr-=2;
        path[ptr]=='U';
    }
    else if((path[ptr-2]=='L' && (path[ptr]=='F'))
    {
        ptr-=2;
        path[ptr]=='R';
    }
    else if((path[ptr-2]=='F' && (path[ptr]=='F'))
    {
        ptr-=2;
        path[ptr]=='U';
    }
    else if((path[ptr-2]=='F' && (path[ptr]=='L'))
    {
        ptr-=2;
        path[ptr]=='R';
    }
}
void delay(int a)
{
	volatile int i,j;
	for(i=0;i<a;i++){
		for(j=0;j<a;j++){
		}
	}
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
