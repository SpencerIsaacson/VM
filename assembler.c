#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.h"

enum mneumonic_ids
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
	load_immediate,
	load_memory,
	store,
	jump,
	jump_equal,
	jump_not_equal,
	jump_less_than,
	jump_less_than_or_equal,
	jump_greater_than,
	jump_greater_than_or_equal,
};

#define mneumonic_count 22
char* mneumonic_names[mneumonic_count] =
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
	"loadi",
	"loadm",
	"store",
	"jump",
	"jeq",
	"jump_not_equal",
	"jump_less_than",
	"jump_less_than_or_equal",
	"jgt",
	"jump_greater_than_or_equal",	
};

int look_up_id(char* string);
char is_whitespace(char a);

int main(int argc, char** argv)
{
	StartTiming();
	if(argc == 2)
	{
		FILE* file = fopen(argv[1],"rb");
		if(file != NULL)
		{
			fseek(file,0,SEEK_END);
			int characters_long = ftell(file);
			fseek(file,0,0);

			char text[characters_long+1];
			text[characters_long] = 0;
			fread(text, characters_long,1,file);
			fclose(file);

			int line_number = 0;
			int scanner = 0;

			long long output_data[1000]; //todo make sized correctly
			int current_data_word = 0;
			while(scanner < characters_long)
			{
				char mneumonic[50], arg1[50], arg2[50];
				for (int o = 0; o < 50; ++o)
				{
					mneumonic[o] = 0;
					arg1[o] = 0;
					arg2[o] = 0;
				}

				char* pointers[3] = { mneumonic, arg1, arg2 };
				char was_space = 0;
				int token_count = 0;
				int token_scanner = 0;

				while(text[scanner] != '\r' && scanner != characters_long)
				{
					if(text[scanner] == ' ')
						was_space = 1;
					else
					{
						if(was_space)
						{
							token_count++;
							was_space = 0;
							token_scanner=0;
						}

						pointers[token_count][token_scanner] = text[scanner];
					}

					scanner++;
					token_scanner++;
				}

				scanner+=2; //skip over newline
				token_count++; //make it 1 indexed

				//print tokenized info
				{
					printf("%d. mneumonic: %s", line_number, mneumonic);
					if(token_count > 1)
						printf(" arg1: %s", arg1);
					if(token_count > 2)
						printf(" arg2: %s", arg2);
					printf("\n");
				}

				int mneumonic_id;
				//look up id
				{
					mneumonic_id = -1;

					for(int i = 0; i < mneumonic_count; i++)
					{					
						if(strcmp(mneumonic, mneumonic_names[i]) == 0)
						{
							mneumonic_id = i;
							break;
						}
					}
				}

				if(mneumonic_id == -1)
				{
					printf("invalid id");
					return;
				}


				switch(mneumonic_id)
				{
					case load_memory:
					case load_immediate:
					{
						if(arg1[0] == 'A')
							output_data[current_data_word] = 12;
						else if(arg1[0] == 'B')
							output_data[current_data_word] = 13;

						char* end;
						output_data[current_data_word+1] = _strtoui64(arg2,&end,10);//todo parse hex values too;						
						current_data_word += 2;
					} break;
					case store:
					case jump_equal:
					case jump:
					case jump_greater_than:
					case jump_less_than_or_equal:
					case jump_greater_than_or_equal:
					case jump_not_equal:
					case jump_less_than:
					{
						output_data[current_data_word] = mneumonic_id;
						output_data[current_data_word+1] = _strtoui64(arg1,&end,10);//todo parse hex values too;
						current_data_word+=2;						
					} break;
					case nop:
					case add:
					case or:
					case and:
					case xor:
					case left_shift:
					case right_shift:
					case increment:
					case decrement:
					case subtract:
					case multiply:
					case divide:
					default:
					{

						output_data[current_data_word] = mneumonic_id;
						current_data_word++;					
					} break;
				}

				line_number++;
			}

			char* as_bytes = (char*)(output_data);
			for (int i = 0; i < current_data_word*8; ++i)
			{
				printf("%02x ", as_bytes[i]);
			}			
		}
	}

	EndTiming();
}

