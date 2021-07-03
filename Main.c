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
#define u64 unsigned long long
#define bytes_per_u64 8
#define memory_length 15*bytes_per_megabyte/bytes_per_u64
static u64 memory[memory_length];
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
	opcode_count
};


char* names[opcode_count] =
{
	"nop",
	"and",
	"or",
	"xor",
	"left_shift",
	"right_shift",
	"increment",
	"decrement",
	"add",
	"subtract",
	"multiply",
	"divide",
	"loadAi",
	"loadBi",
	"loadAm",
	"loadBm",
	"store",
	"jump",
	"jeq",
	"jump_not_equal",
	"jump_less_than",
	"jump_less_than_or_equal",
	"jgt",
	"jump_greater_than_or_equal",	
};

#define frame_buffer 600
#define gamepad 60+640*480
static bool window_is_open = true;
static int width = 640, height = 480;
void main()
{
	for (int i = 0; i < memory_length; ++i)
	{
		memory[i] = 0;
	}

	FILE* file = fopen("assembly.bin", "rb");
	fseek(file,0,SEEK_END);
	int bytes_long = ftell(file);
	fseek(file,0, SEEK_SET);
	fread(memory,bytes_long,1,file);

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
				case nop:
					break;
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
					if(memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
				case jump_equal:
					if(register_A == register_B && memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_not_equal:
					if(register_A != register_B && memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_less_than:
					if(register_A < register_B && memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_less_than_or_equal:
					if(register_A <= register_B && memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;					
				case jump_greater_than:
					if(register_A > register_B && memory[program_counter + 1] < memory_length)
					{
						program_counter = memory[program_counter + 1];
						goto skip_pc_increment;
					}
					break;
				case jump_greater_than_or_equal:
					if(register_A >= register_B && memory[program_counter + 1] < memory_length)
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
			
			if(program_counter  == memory_length)
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


		// //print memory state
		// {
		// 	byte* as_bytes = (byte*)memory;

		// 	for (int i = 0; i < 100; ++i)
		// 	{
		// 		printf("%02x ",as_bytes[i]);
		// 	}

		// 	printf("\n\n");
		// }

		// sleep(75);

		printf("\nA: %d, B: %d, out: %d, pc: %d, opcode: %s\n",register_A, register_B, output, program_counter, names[memory[program_counter]]);
    }
 }	