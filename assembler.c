#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "input.h"
#include "vm.c"

typedef enum
{
	keyw_set,
	keyw_inc,
	keyw_add,
	keyw_mult,
	keyw_jmp,
	keyw_jlt,
	keyw_halt,
	keyword_count,
} Keyword;

char * keywords[keyword_count] =
{
	[keyw_set] = "set",
	[keyw_inc] = "inc",
	[keyw_halt] = "halt",
	[keyw_jmp] = "jmp",
	[keyw_add] = "add",
	[keyw_mult] = "mult",
	[keyw_jlt] = "jlt",
};

char str_eq(char *a, char *b)
{
	while(*a != 0 && b != 0)
	{
		if(*(a++) != *(b++))
			return 0;
	}

	return 1;
}


int parse_int(char *s)
{
	int ret = 0;
	while(*s != 0)
	{
		ret += (*(s++)-'0');
		if(*s != 0)
			ret *= 10;
	}

	return ret;
}

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		char* text;
		int characters_long;
		
		//read file
		{
			FILE* file = fopen(argv[1],"rb");
			if(file == NULL){
				printf("File \"%s\" does not exist\n", argv[1]);
				exit(-1);
			}
			else
			{
				fseek(file,0,SEEK_END);
				characters_long = ftell(file);
				rewind(file);
				text = (char*)malloc(characters_long+1);
				text[characters_long] = 0;
				fread(text, characters_long,1,file);
				fclose(file);
			}
		}

		while(*text != 0)
		{		
			char line[300] = {0};
			//read line
			{
				int i = 0;
				while(*text != '\n')
				{
					line[i++] = *(text++);
				}
				line[i] = '\n';
				text++;
			}


			Keyword keyword = -1;
			int cursor = 0;
			//get keyword
			{
				char keyword_text[100] = {0};
				{
					while(line[cursor] != ' ' && line[cursor] != '\n')
					{
						keyword_text[cursor] = line[cursor++];
					}
				}

				for (int i = 0; i < keyword_count; ++i)
				{
					if(str_eq(keyword_text, keywords[i]))
					{
						keyword = i;
						break;
					}
				}
			}

			switch(keyword)
			{
				case keyw_set:
				{
					cursor++;
					AddressingMode dest_mode = IMM;
					if(line[cursor] == '@')
					{
						dest_mode  = IND;
						cursor++;
					}

					char dest_text[50] = {0};
					int i = 0;
					while(line[cursor] != ' ')
					{
						dest_text[i++] = line[cursor++];
					}

					u32 dest = parse_int(dest_text);
					cursor++;
					printf("next char:%c\n", line[cursor]);
					AddressingMode val_mode = IMM;
					if(line[cursor] == '@')
					{
						val_mode  = IND;
						cursor++;
					}

					char val_text[50] = {0};
					i = 0;
					while(line[cursor] != '\n')
					{
						val_text[i++] = line[cursor++];
					}

					u32 val = parse_int(val_text);
					cursor++;
					u32 res = set(dest, val, dest_mode, val_mode);
					printf("res: %d\n", res);
				} break;
				case keyw_halt:
				{
					halt();
				} break;
				case keyw_jmp:
				{
					cursor++;
					AddressingMode dest_mode = IMM;
					if(line[cursor] == '@')
					{
						dest_mode  = IND;
						cursor++;
					}

					char dest_text[50] = {0};
					int i = 0;
					while(line[cursor] != '\n')
					{
						dest_text[i++] = line[cursor++];
					}

					u32 dest = parse_int(dest_text);
					cursor++;

					jmp(dest, dest_mode);		
				} break;
				case keyw_jlt:
				{
					cursor++;
					AddressingMode dest_mode = IMM;
					if(line[cursor] == '@')
					{
						dest_mode  = IND;
						cursor++;
					}

					char text[50] = {0};
					int i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 dest = parse_int(text);
					cursor++;

					AddressingMode lh_m = IMM;
					if(line[cursor] == '@')
					{
						lh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 lhs = parse_int(text);
					cursor++;


					AddressingMode rh_m = IMM;
					if(line[cursor] == '@')
					{
						rh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != '\n')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 rhs = parse_int(text);
					cursor++;

					jlt(dest, lhs, rhs, dest_mode, lh_m, rh_m);		
				} break;
				case keyw_inc:
				{
					cursor++;
					char dest_text[50] = {0};
					int i = 0;
					while(line[cursor] != '\n')
					{
						dest_text[i++] = line[cursor++];
					}

					u32 dest = parse_int(dest_text);
					cursor++;

					inc(dest);		
				} break;
				case keyw_add:
				{
					cursor++;
					AddressingMode dest_mode = IMM;
					if(line[cursor] == '@')
					{
						dest_mode  = IND;
						cursor++;
					}

					char text[50] = {0};
					int i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 dest = parse_int(text);
					cursor++;

					AddressingMode lh_m = IMM;
					if(line[cursor] == '@')
					{
						lh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 lhs = parse_int(text);
					cursor++;


					AddressingMode rh_m = IMM;
					if(line[cursor] == '@')
					{
						rh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != '\n')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 rhs = parse_int(text);
					cursor++;

					add(dest, lhs, rhs, dest_mode, lh_m, rh_m);		
				} break;
				case keyw_mult:
				{
					cursor++;
					AddressingMode dest_mode = IMM;
					if(line[cursor] == '@')
					{
						dest_mode  = IND;
						cursor++;
					}

					char text[50] = {0};
					int i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 dest = parse_int(text);
					cursor++;

					AddressingMode lh_m = IMM;
					if(line[cursor] == '@')
					{
						lh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != ' ')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 lhs = parse_int(text);
					cursor++;


					AddressingMode rh_m = IMM;
					if(line[cursor] == '@')
					{
						rh_m  = IND;
						cursor++;
					}

					i = 0;
					while(line[cursor] != '\n')
					{
						text[i++] = line[cursor++];
					}
					text[i] = 0;

					u32 rhs = parse_int(text);
					cursor++;

					mult(dest, lhs, rhs, dest_mode, lh_m, rh_m);		
				} break;
			}
		}

		u32 byte_count = (next_word-start_address)*4;

		FILE *file = fopen("assembly.bin", "wb");
		fwrite(&stronkbox.memory.RAM[start_address], byte_count,1,file);
	}
}