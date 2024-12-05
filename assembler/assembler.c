#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "input.h"
#include "vm.c"

typedef enum
{
	NOT_IMPLEMENTED = -17
} ErrorCodes;

char * keywords[] =
{
	//basics
	[OP_NOP]         = "nop",
	[OP_INC]         = "inc",
	[OP_DEC]         = "dec",
	[OP_SET]         = "set",
	//arithmetic
	[OP_ADD]         = "add",
	[OP_SUB]         = "sub",
	[OP_MUL]         = "mul",
	[OP_DIV]         = "div",
	//logic
	[OP_NOT]         = "not",
	[OP_AND]         = "and",
	[OP_IOR]         = "ior",
	[OP_XOR]         = "xor",
	[OP_LSL]         = "lsl",
	[OP_RSL]         = "rsl",
	//branch
	[OP_JMP]         = "jmp",
	[OP_JLT]         = "jlt",
	[OP_JGT]         = "jgt",
	[OP_JEQ]         = "jeq",
	[OP_JNE]         = "jne",
	[keyword_define] = "define",
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

unsigned int parse_hex(char *s)
{
	int ret = 0;
	while(*s != 0)
	{
		if(isdigit(*s))
			ret += (*(s++)-'0');
		else if(isalpha(*s)){
			*s &= ~0x20;
			ret += (*(s++)-('A'-0xA));		
		}
		if(*s != 0)
			ret *= 0x10;
	}

	return ret;
}

char* read_file(char *path, int* chl)
{
	FILE* file = fopen(path,"rb");
	if(file == NULL){
		printf("File \"%s\" does not exist\n", path);
		exit(-1);
	}
	else
	{
		fseek(file,0,SEEK_END);
		int chars_long = ftell(file);
		rewind(file);
		char *data = (char*)malloc(chars_long+1);
		data[chars_long] = 0;
		fread(data, chars_long,1,file);
		fclose(file);
		*chl = chars_long;
		return data;
	}
}

void uno_arg(OpCode opcode, int *cursor_o, char *line)
{
	int cursor = *cursor_o;
	cursor++;
	AddrMode dest_mode = IMM;
	if(line[cursor] == '@')
	{
		dest_mode  = IND;
		cursor++;
	}

	char dest_text[50] = {0};
	int i = 0;
	while(line[cursor] != '\n' && line[cursor] != ' ' && line[cursor] != '\t')
	{
		dest_text[i++] = line[cursor++];
	}

	u32 dest = parse_int(dest_text);
	cursor++;

	emit_1_arg_vw(opcode, (Op32){dest_mode, dest});
	*cursor_o = cursor;	
}

void tres_arg(OpCode opcode, int *cursor_o, char *line)
{
	int cursor = *cursor_o;	
	cursor++;
	AddrMode dest_mode = IMM;
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

	AddrMode lh_m = IMM;
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


	AddrMode rh_m = IMM;
	if(line[cursor] == '@')
	{
		rh_m  = IND;
		cursor++;
	}

	i = 0;
	while(line[cursor] != ' ' && line[cursor] != '\r' && line[cursor] != '\n' && line[cursor] != ';')
	{
		text[i++] = line[cursor++];
	}
	text[i] = 0;

	u32 rhs = parse_int(text);
	cursor++;

	emit_3_arg_vw(opcode, (Op32){dest_mode, dest}, (Op32){lh_m, lhs}, (Op32){rh_m, rhs});
	*cursor_o = cursor;	
}

void dos_arg(OpCode opcode, int *cursor_o, char *line)
{
	int cursor = *cursor_o;
	cursor++;
	AddrMode dest_mode = IMM;
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
	AddrMode val_mode = IMM;
	if(line[cursor] == '@')
	{
		val_mode = IND;
		cursor++;
	}

	char val_text[50] = {0};
	i = 0;
	while(line[cursor] != ' ' && line[cursor] != '\r' && line[cursor] != '\t' && line[cursor] != '\n' && line[cursor] != ';')
	{
		val_text[i++] = line[cursor++];
	}

	// for(int o = 0; o < 50; o++)
	// {
	// 	printf("%d",val_text[o]);
	// 	if(val_text[o]==0)
	// 		break;
	// 	else
	// 		printf(", ");
	// }
	// printf("val_text:%s\n", val_text);
	u32 val = parse_int(val_text);
	// printf("val: %d\n", val);
	cursor++;
	emit_2_arg_vw(opcode, (Op32){dest_mode, dest}, (Op32){val_mode, val});

	*cursor_o = cursor;

}

int main(int argc, char** argv)
{
	if(argc == 2)
	{
		int chars_long;
		char* text = read_file(argv[1], &chars_long);


		if(text != NULL)
		while(*text != 0)
		{		
			char line[300] = {0};
			
			//read line
			{
				int i = 0;
				while(*text != '\r' && *text != ';')
				{
					line[i++] = *(text++);
				}
				line[i] = '\n';
				while(*text != '\n') text++;
				text++;
			}

			OpCode keyword = -1;
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

				for (int i = 0; i < opcode_count; ++i)
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
				case OP_NOP:
				case OP_HALT:
				{
					exit(NOT_IMPLEMENTED);
				} break;
				case OP_INC:
				case OP_DEC:
				case OP_JMP:
				{
					uno_arg(keyword, &cursor,line);	
				} break;
				case OP_SET:
				case OP_NOT:
				{
					dos_arg(keyword, &cursor,line);
				} break;
				case OP_ADD:
				case OP_SUB:
				case OP_MUL:
				case OP_DIV:
				case OP_AND:
				case OP_IOR:
				case OP_XOR:
				case OP_LSL:
				case OP_RSL:				
				case OP_JLT:
				case OP_JGT:
				case OP_JEQ:
				case OP_JNE:
				{
					tres_arg(keyword, &cursor,line);	
				} break;
				case keyword_define:
				{
					printf("")
				}
				default:
				{
					//assume is label
				}
			}
		}

		u32 byte_count = (next_word-start_address)*4;

		FILE *file = fopen("assembly.bin", "wb");
		fwrite(&humidor.memory.RAM[start_address], byte_count,1,file);
	}
}