#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#define SDL_MAIN_HANDLED
#include "Dependencies/SDL_Headers/SDL.h"
#include "debug.h"
#include "input.h"
#include "draw.h"
#include "vm.c"
static bool window_is_open = true;
int scale_factor = 1;
#define mem stronkbox.memory

void fill_audio(void *udata, Uint8 *stream, int len)
{
	int *foo = (int *)stream;
	for(int i = 0; i < len/4; i++, stronkbox.cpu.sample_cursor++)
	{
		foo[i] = mem.audio_buffer.samples[stronkbox.cpu.sample_cursor];
		mem.audio_buffer.samples[stronkbox.cpu.sample_cursor] = 0;
		if(stronkbox.cpu.sample_cursor == audio_sample_capacity)
		{
			stronkbox.cpu.sample_cursor = 0;
		}
	}
}

#include "zelda/zelda.h"

void main(int argc, char **argv)
{
	// if(argc > 1)
	// {
	// 	if(argv[1][0] == '-' && argv[1][1] == 'v' && argv[1][2] == 0)
	// 	{
	// 		printf("stronkbox version: 0.0.0\n");
	// 		exit(0);
	// 	}
	// 	else
	// 	{
	// 		printf("unrecognized commandline argument \"%s\"\n", argv[1]);
	// 		exit(1);
	// 	}
	// }

	reset();
	//load rom
	{
		FILE *file = fopen("assembly.bin", "rb");
		fseek(file, 0, SEEK_END);
		int c = ftell(file);
		rewind(file);
		fread(&stronkbox.memory.RAM[start_address],c,1,file);
		fclose(file);
	}

	init();
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);

    //Load Joysticks
    {
        //todo allow plugging in and removing joysticks at runtime
        int joy_count = SDL_NumJoysticks();
        printf("JOY COUNT: %d\n", joy_count);
        for (int i = 0; i < joy_count; ++i)
        {
			SDL_Joystick* game_controller = SDL_JoystickOpen( i );
	        if( game_controller == NULL )
	        {
	            printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
	        }
        }
    }

    //Initialize Audio
    {
		SDL_AudioSpec want, have;
		SDL_memset(&want, 0, sizeof(want));
		want.freq = 48000;
		want.format = AUDIO_S32;
		want.channels = 1;
		want.samples = 500;
		want.callback = fill_audio;
		SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		SDL_PauseAudioDevice(device, 0);
    }

	SDL_Window *window = SDL_CreateWindow("StronkBox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vm_width*scale_factor, vm_height*scale_factor, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	Color* pixels = (Color*)surface->pixels;

	s64 prev = 0;
	s64 cur = 0;
	u32 micros = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&prev);
    while (window_is_open)
    {
    	if(GetAsyncKeyState(VK_ESCAPE))
    		return;
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
                    	mem.game_pads[0].sticks.left_stick.X =  (signed char)((event.jaxis.value/((float)SHRT_MAX))*127);
                    else if( event.jaxis.axis == 1 )
                       mem.game_pads[0].sticks.left_stick.Y = -(signed char)((event.jaxis.value/((float)SHRT_MAX))*127);
                    break;
                case SDL_JOYBUTTONDOWN:
            	{
	    			//todo custom mappings at run time
	    			int mapping[10] = { A, B, Y, X, L2, R2, L1, R1, SELECT, START };
	    			if(event.jbutton.button < 10)
	    			mem.game_pads[0].buttons |= mapping[event.jbutton.button];
            	} break;
                case SDL_JOYBUTTONUP:
                {
               		int mapping[10] = { A, B, Y, X, L2, R2, L1, R1, SELECT, START };
               		if(event.jbutton.button < 10)
               		mem.game_pads[0].buttons &= ~(mapping[event.jbutton.button]);
                } break;
                case SDL_JOYHATMOTION:
                {
                	mem.game_pads[0].buttons &= ~(UP|DOWN|LEFT|RIGHT);
                	int up = event.jhat.value & 1;
                	int right = event.jhat.value & 2;
                	int down = event.jhat.value & 4;
                	int left = event.jhat.value & 8;

                	if(up)
                		mem.game_pads[0].buttons |= UP;
                	if(down)
                		mem.game_pads[0].buttons |= DOWN;
                	if(left)
                		mem.game_pads[0].buttons |= LEFT;
                	if(right)
                		mem.game_pads[0].buttons |= RIGHT;
                } break;          
        	}
        }
		
		//poll keyboard
		{
			mem.game_pads[0].buttons = 0;
			if(GetAsyncKeyState(VK_RIGHT))
			{
				mem.game_pads[0].buttons |= RIGHT;
			}
			if(GetAsyncKeyState(VK_LEFT))
			{
				mem.game_pads[0].buttons |= LEFT;
			}
			if(GetAsyncKeyState(VK_UP))
			{
				mem.game_pads[0].buttons |= UP;
			}
			if(GetAsyncKeyState(VK_DOWN))
			{
				mem.game_pads[0].buttons |= DOWN;
			}

			if(GetAsyncKeyState(VK_SPACE))
			{
				mem.game_pads[0].buttons |= A;
			}

			if(GetAsyncKeyState('F'))
			{
				mem.game_pads[0].buttons |= X;
			}
			
			if(GetAsyncKeyState('R'))
			{
				mem.game_pads[0].buttons |= B;
			}
		}

		_tick();
		memcpy(pixels, &mem.frame_buffer, vm_width*vm_height*4);
    	SDL_UpdateWindowSurface(window);
// #define clock_rate 100000000
// #define tick_count 100
// #define interval 1000000/clock_rate*tick_count
// 		u32 microseconds;
// 		static u32 micros2;
// 		QueryPerformanceCounter((LARGE_INTEGER*)&cur);
//     	diff = ((cur - prev) * 1000000) / freq;
//     	microseconds = (u32)(diff & 0xffffffff);
//     	micros2 += microseconds;

//     	if(micros2 > interval)
//     	{

// 	    	for (int i = 0; i < tick_count; ++i)
// 	    	{
// 	    		tick();
// 	    	}
// 	    	micros2=0;
//     	}

// 		QueryPerformanceCounter((LARGE_INTEGER*)&cur);
//     	diff = ((cur - prev) * 1000000) / freq;
//     	microseconds = (u32)(diff & 0xffffffff);
//     	micros += microseconds;

//     	//todo replace with interrupt
//     	if(micros >= 16667)
//     	{
// 			memcpy(pixels, &mem.frame_buffer, vm_width*vm_height*4);
// 	    	SDL_UpdateWindowSurface(window);
// 	    	micros = 0;
//     	}

// 		QueryPerformanceCounter((LARGE_INTEGER*)&prev);
    }
 }	