#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#define SDL_MAIN_HANDLED
#include "Dependencies/SDL_Headers/SDL.h"
#include "Debug.h"

#define byte unsigned char
#define bytes_per_kilobyte 1024
#define bytes_per_megabyte 1024*1024
#define u64 long long
#define bytes_per_u64 8
static u64 memory[15*bytes_per_megabyte/bytes_per_u64];
static u64 register_A = 0;
static u64 register_B = 0;
static u64 output = 0;
static int program_counter = 0;
static short stack_pointer = 0; //not in use yet

enum opcodes
{
	nop,
	and,
	or,
	xor,
	left_shift,
	right_shift,
	increment,
	decrement,
	add,
	subtract,
	multiply,
	divide,
	loadA_immediate,
	loadB_immediate,
	loadA_memory,
	loadB_memory,
	store,
	jump,
	jump_equal,
	jump_not_equal,
	jump_less_than,
	jump_less_than_or_equal,
	jump_greater_than,
	jump_greater_than_or_equal,
};


#define frame_buffer 60

static bool window_is_open = true;
static int width = 640, height = 480;
void main()
{
	for (int i = 0; i < 1024; ++i)
	{
		memory[i] = 0;
	}

	memory[0] = loadA_immediate;
	memory[1] = 7;
	memory[2] = loadB_immediate;
	memory[3] = 5;
	memory[4] = add;
	memory[5] = store;
	memory[6] = 10;
	memory[7] = loadA_immediate;
	memory[8] = 12;
	memory[9] = loadB_immediate;
	memory[10] = 0; //where we save the result;
	memory[11] = jump_equal;
	memory[12] = 20;
	memory[13] = jump;
	memory[14] = 13;
	memory[20] = loadA_immediate;
	memory[21] = 0x0000FFFF00FF00FF;
	memory[22] = loadB_immediate;
	memory[23] = 0;
	memory[24] = or;
	memory[25] = store;
	memory[26] = frame_buffer;
	memory[27] = loadA_immediate;
	memory[28] = 1;
	memory[29] = loadB_memory;
	memory[30] = 26;
	memory[31] = add;
	memory[32] = store;
	memory[33] = 26;
	memory[34] = loadA_immediate;
	memory[35] = frame_buffer+(16*4);
	memory[36] = jump_greater_than;
	memory[37] = 20;
	memory[38] = jump;
	memory[39] = 38;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("VM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	unsigned int* pixels = surface->pixels;

    while (window_is_open) 
    {
		//tick
		{
			char instruction = memory[program_counter];

			switch(instruction)
			{
				case and:
					output = register_A & register_B;
					break;
				case or:
					output = register_A | register_B;
					break;
				case xor:
					output = register_A ^ register_B;
					break;
				case left_shift:
					output = register_A << register_B;
					break;
				case right_shift:
					output = register_A >> register_B;
					break;
				case increment:
					output = register_A+1;
					break;
				case decrement:
					output = register_A-1;
					break;
				case add:
					output = register_A + register_B;
					break;
				case subtract:
					output = register_A	- register_B;
					break;
				case multiply:
					output = register_A * register_B;
					break;
				case divide:
					output = register_A / register_B;
					break;
				case loadA_immediate:
					register_A = memory[program_counter + 1];
					program_counter++;
					break;
				case loadB_immediate:
					register_B = memory[program_counter + 1];
					program_counter++;
					break;
				case loadA_memory:
					register_A = memory[memory[program_counter + 1]];
					program_counter++;
					break;
				case loadB_memory:
					register_B = memory[memory[program_counter + 1]];
					program_counter++;
					break;									
				case store:
					memory[memory[program_counter+1]] = output;
					program_counter++;
					break;																
				case jump:
					program_counter = memory[program_counter + 1];
					goto skip_pc_increment;
				case jump_equal:
					if(register_A == register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_not_equal:
					if(register_A != register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_less_than:
					if(register_A < register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_less_than_or_equal:
					if(register_A <= register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;					
				case jump_greater_than:
					if(register_A > register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_greater_than_or_equal:
					if(register_A >= register_B)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				default:
					printf("unimplemented instruction\n");												
			}

			program_counter++;
			
			skip_pc_increment:
			
			if(program_counter  == 1024)
				program_counter = 0;
		}

        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch(event.type)
        	{
                case SDL_QUIT:
                    window_is_open = false;
                    break;
        	}
        }
	

			static float previous_time=0;
			static float elapsed= 0;
			elapsed += (float)(clock() - previous_time) / (float)CLOCKS_PER_SEC;
			

			previous_time = clock();            

		if(elapsed>.015f)
		{
     		memcpy(pixels, &(memory[frame_buffer]),width*height*4);
     		elapsed = 0;
        	SDL_UpdateWindowSurface(window);
		}
    }
 }	