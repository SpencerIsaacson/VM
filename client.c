#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#define SDL_MAIN_HANDLED
#include "Dependencies/SDL_Headers/SDL.h"
#include "Debug.h"
#define DEBUG 1
typedef struct 
{
	float x, y;
} v2;

typedef struct PadState
{
	bool buttons[10];
	v2 left_stick;
	v2 right_stick;
} PadState;


typedef struct ShortStick //16 bit (short) representation of analog stick state
{
	signed char X;
	signed char Y;
} ShortStick;

typedef struct Buttons
{
	// bit flags, 16 buttons, up down left right A B X Y select start L1 R1 L2 R2 L3 R3
	unsigned char main_buttons; //UP DOWN LEFT RIGHT (first nibble) |  A B X Y (second nibble)
	unsigned short extra_buttons; // SELECT START L1 L2 (first nibble) | L2 R2 L3 R3 (second nibble)
} Buttons;

typedef struct Triggers //sizeof short
{
	unsigned char l_trigger;
	unsigned char r_trigger;
} Triggers;

typedef struct Sticks
{
	ShortStick left_stick;
	ShortStick right_stick;
} Sticks;

typedef struct Pad //total size of 8 bytes (64 bits)
{
	Buttons buttons;
	Triggers triggers;
	/*boundary between first int and second int ----------*/
	Sticks sticks;
} Pad;

static bool window_is_open = true;
int scale_factor = 2;

PadState* pad;

#include "vm.c"
void main()
{
	pad = (PadState*)&RAM[gamepad];

	reset();
	load_ROM("assembly.bin");

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
    //Load Joysticks
    {
        //todo allow plugging in and removing joysticks at runtime
        int joy_count = SDL_NumJoysticks();
        for (int i = 0; i < joy_count; ++i)
        {
			SDL_Joystick* game_controller = SDL_JoystickOpen( i );
	        if( game_controller == NULL )
	        {
	            printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
	        }
        }
    }

	SDL_Window *window = SDL_CreateWindow("StronkBox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width*scale_factor, height*scale_factor, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	unsigned int* pixels = surface->pixels;

	static clock_t previous_time = 0;
	static float elapsed = 0;
	float interval = 1/100000000.0f;
	static int ticks = 0;
    while (window_is_open)
    {
    	//if(elapsed > interval)
    	{
    	    fetch();
    	    #define do_print 0
		    #if do_print
		    	printf("______________\n");
		    	printf("pc: %d, a: %d, b: %d, o: %d\n", PC, A, B, O);
    
    		    	if(I < opcode_count)
    		    		printf("%s\n", names[I]);
    		    	else
    		    	{
    		    		printf("%d\n", I);
    		    		Sleep(500);
    		    	}
		    #endif
    		    execute();
    		    ticks++;
    		if(ticks > 100000000)
    		{
    			ticks = 0;
    			printf("100 mil!\n");
    		}
    		#if do_print
    		    	printf("pc: %d, a: %d, b: %d, o: %d\n", PC, A, B, O);
		    #endif
    		elapsed	= 0;
    	}

        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch(event.type)
        	{
                case SDL_QUIT:
                    window_is_open = false;
                    break;
                case SDL_JOYAXISMOTION:
                    if( event.jaxis.axis == 0 )
                    	pad->left_stick.x = (event.jaxis.value);
                    else if( event.jaxis.axis == 1 )
                        pad->left_stick.y = -(event.jaxis.value);
                    break;
                case SDL_JOYBUTTONDOWN:
	                if(event.jbutton.type == SDL_JOYBUTTONDOWN)
	                	pad->buttons[event.jbutton.button] = true;
                    break;
                case SDL_JOYBUTTONUP:
	                if(event.jbutton.type == SDL_JOYBUTTONUP)
	                	pad->buttons[event.jbutton.button] = false;
                    break;                    
        	}
        }

		if(I == DRAW)
		{
			int screen_width = width*scale_factor;
			int screen_height = height*scale_factor;
			int* screen = (int*)&RAM[frame_buffer];

			for (int y = 0; y < screen_height; ++y)
			for (int x = 0; x < screen_width; ++x)
			{
				pixels[y*(screen_width)+x] = screen[(y/scale_factor)*(screen_width/scale_factor)+(x/scale_factor)];
			}
			
        	SDL_UpdateWindowSurface(window);
		}	

		elapsed += (float)(clock() - previous_time) / (float)CLOCKS_PER_SEC;
		//printf("elapsed:%f\n",elapsed);
		previous_time = clock();            
		//printf("clock: %d\n", clock());
		#if DEBUG
		static int sleep = 0;
		// if(sleep)//print memory state
		// {
		// 	byte* as_bytes = (byte*)memory;
		// 	printf("\n0:\t");
		// 	for (int i = 0; i < 8192; ++i)
		// 	{
		// 		printf("%02x ",as_bytes[i]);
		// 		if((i > 0) && i != 8191 && ((i+1) % 32 == 0))
		// 			printf("\n%d:\t",i+1);
		// 	}

		// 	printf("\n\n");
		// }
		
		printf("\nA: %d, B: %d, O: %d, PC: %d, opcode: %s\n",A, B, O, PC, names[RAM[PC]]);
		
		//if(PC == 53)
		//	sleep = 1600;
		Sleep(sleep);
		#endif

		
    }
 }	