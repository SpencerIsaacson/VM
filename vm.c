#include <stdio.h>
#include <stdbool.h>
#include "input.h"
#define kilobyte (1024)
#define megabyte (1024*1024)

typedef unsigned char byte;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

#define vm_width 480
#define vm_height 270

typedef u32 Color;
typedef struct FrameBuffer
{
	Color pixels[vm_width*vm_height];
} FrameBuffer;

typedef s32 AudioSample;
#define audio_sample_capacity 22050
typedef struct AudioBuffer
{
	AudioSample samples[audio_sample_capacity];
} AudioBuffer;

#define capacity (15*megabyte)
#define word_count (capacity/sizeof(u32))
typedef union Memory
{
	struct
	{
		byte        user_mem[capacity-(sizeof(GamePad)*4+sizeof(AudioBuffer)+sizeof(FrameBuffer))];
		GamePad     game_pads[4];
		AudioBuffer audio_buffer;
		FrameBuffer frame_buffer;
	};
	
	u32			RAM[word_count];
} Memory;

typedef struct CPU
{
	int PC; //program counter
	int NPC; //next program counter.just a helper value
	short SP; //stack pointer not in use yet
	short sample_cursor; //not REALLY part of the CPU
	bool halted;
} CPU;

typedef struct Humidor
{
	CPU cpu;
	Memory memory;
} Humidor;

Humidor humidor;

typedef enum
{
	IMM = 0,
	IND = 1,
} AddrMode;

#define a_mode(op) ((AddrMode)((op & 0x80) >> 7))
#define b_mode(op) ((AddrMode)((op & 0x40) >> 6))
#define c_mode(op) ((AddrMode)((op & 0x20) >> 5))

typedef enum OpCode
{
	//basics
	OP_NOP,
	OP_INC,
	OP_DEC,
	OP_SET,
	//arithmetic
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	//logic/bitwise
	OP_NOT,
	OP_AND,
	OP_IOR,
	OP_XOR,
	OP_LSL,
	OP_RSL,
	//branch/jump
	OP_JMP,
	OP_JLT,
	OP_JGT,
	OP_JEQ,
	OP_JNE,
	//cpu state
	OP_HALT,
	opcode_count,
} OpCode;

// char* OpNames[0] = 
// {
// 	// "NOP",
// 	// "SET",
// 	// "MULT",
// 	// "INC",
// 	// "ADD",
// 	// "JLT",
// 	// "JMP",
// 	// "HALT",
// };

#define RAM humidor.memory.RAM
#define PC  humidor.cpu.PC
#define NPC humidor.cpu.NPC

//helpers
#define start_address 0
#define screen_address ((sizeof(humidor.memory.user_mem)+sizeof(humidor.memory.game_pads)+sizeof(humidor.memory.audio_buffer))/sizeof(u32))


#include "live_assembler.c"

void reset()
{
	memset(&humidor, 0, sizeof(humidor));
	PC = start_address;
}

typedef struct
{
	u32 address;
	u32 count;
} ChunkHeader;

void load_ROM(char* path)
{
	FILE* file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	int bytes_long = ftell(file);
	rewind(file);
	fread(RAM, bytes_long, 1, file);
}

void jump_cond(bool cond, u32 dest)
{
	if(cond)
		NPC = dest;
	else
		NPC+=3; //skip the next value (which is just the jump address we didn't use)
}

u32 _a;
u32 _b;
void op_set(OpCode op)
{
	int a = PC+1;
	int b = PC+2;

	_a = a_mode(op) ? RAM[RAM[a]] : RAM[a];
	_b = b_mode(op) ? RAM[b] : b;

	RAM[_a] = RAM[_b];	
}

void tick()
{
	if(!humidor.cpu.halted)
	{
		OpCode op = RAM[PC];

	    NPC = PC + 1;
		switch(op & 0x1F)
		{
			case OP_NOP:
			{
			} break;
			case OP_SET:
			{
				op_set(op);
				//printf("set %c%d, %c%d\n", a_mode(op)? '@' : ' ', RAM[PC+1], b_mode(op)? '@' : ' ', RAM[PC+2]);
				NPC+=2;	
			} break;
			case OP_MUL:
			{
				u32 *a;
				u32 *b;	
				u32 *c;
				
				if(a_mode(op))
					a = &RAM[RAM[RAM[PC+1]]];
				else
					a = &RAM[RAM[PC+1]];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];
				
				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];
				
				*a = (*b) * (*c);
				NPC+=3;
			} break;
			case OP_INC:
			{
				RAM[RAM[PC+1]]++;
				NPC++;
			} break;
			case OP_ADD:
			{
				u32 *a;
				u32 *b;	
				u32 *c;
				
				if(a_mode(op))
					a = &RAM[RAM[RAM[PC+1]]];
				else
					a = &RAM[RAM[PC+1]];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];
				
				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];
				
				*a = (*b) + (*c);
				NPC+=3;
			} break;
			case OP_SUB:
			{
				u32 *a;
				u32 *b;	
				u32 *c;
				
				if(a_mode(op))
					a = &RAM[RAM[RAM[PC+1]]];
				else
					a = &RAM[RAM[PC+1]];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];
				
				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];
				
				*a = (*b) - (*c);
				NPC+=3;
			} break;			
			case OP_JLT:
			{
				u32 *a;
				u32 *b;			
				u32 *c;

				if(a_mode(op))
					a = &RAM[RAM[PC+1]];
				else
					a = &RAM[PC+1];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];

				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];

				jump_cond(((*b) < (*c)), (*a));
			} break;
			case OP_JGT:
			{
				u32 *a;
				u32 *b;			
				u32 *c;

				if(a_mode(op))
					a = &RAM[RAM[PC+1]];
				else
					a = &RAM[PC+1];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];

				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];

				jump_cond(((*b) > (*c)), (*a));
			} break;			
			case OP_JMP:
			{
				if(a_mode(op))
					NPC = RAM[RAM[PC+1]];
				else
					NPC = RAM[PC+1];
			} break;
			case OP_JEQ:
			{
				u32 *a;
				u32 *b;			
				u32 *c;

				if(a_mode(op))
					a = &RAM[RAM[PC+1]];
				else
					a = &RAM[PC+1];

				if(b_mode(op))
					b = &RAM[RAM[PC+2]];
				else
					b = &RAM[PC+2];

				if(c_mode(op))
					c = &RAM[RAM[PC+3]];
				else
					c = &RAM[PC+3];

				jump_cond(((*b) == (*c)), (*a));
			} break;		
			case OP_HALT:
			{
				printf("HALTED!\n");
				humidor.cpu.halted = true;
			} break;
			default:
			{
				printf("invalid instruction:%x\n", op);
				exit(-1);
			}
		}

		PC = NPC;
		
		if(PC >= word_count)
			PC = 0;
	}
}

u32 arg1,arg2,arg3;

void decode_1_arg(u32 op)
{
	arg1 = (op & 0xFFFFFF);
	if(op & 0x80000000)
		arg1 = RAM[arg1];
}

void decode_2_arg(u32 op)
{
	arg1 = (op & 0xFF0000) >> 16;
	arg2 = (op & 0xFFFF);
	if(op & 0x80000000)
		arg1 = RAM[arg1];
	if(op & 0x40000000)
	{
		arg2 = RAM[arg2];
		if(op & 0x20000000)
			arg2 = RAM[arg2];
	}
}

void decode_3_arg(u32 op)
{
	arg1 = (op & 0xFF0000) >> 16;
	arg2 = (op & 0xFF00) >> 8;
	arg3 = (op & 0xFF);
	if(op & 0x80000000)
		arg1 = RAM[arg1];
	if(op & 0x40000000)
		arg2 = RAM[arg2];
	if(op & 0x20000000)
		arg3 = RAM[arg3];
}

//used for shifts. Since a shift can only be 0-31, only used 5 bits for arg3 and gave the extra bits to arg2
void decode_shift_arg(u32 op)
{
	arg1 = (op & 0xFF0000) >> 16;
	arg2 = (op & 0xFFE0) >> 5;
	arg3 = (op & 0x1F);
	if(op & 0x80000000)
		arg1 = RAM[arg1];
	if(op & 0x40000000)
		arg2 = RAM[arg2];
	if(op & 0x20000000)
		arg3 = RAM[arg3];
}

void execute(int count, u32 ops[])
{
	for (int i = 0; i < count; ++i)
	{
		u32 op = ops[i];
		OpCode opcode = (op & 0x1F000000) >> 24;
		NPC = PC+1;
		//todo bounds check args
		switch(opcode)
		{
			//basics
			{
				case OP_NOP:
					break;
				case OP_INC:
					decode_1_arg(op);
					RAM[arg1]++;
					break;
				case OP_DEC:
					decode_1_arg(op);
					RAM[arg1]--;				
					break;			
				case OP_SET:
					decode_2_arg(op);
					RAM[arg1] = arg2;
					break;
			}
			//arithmetic
			{
				case OP_ADD:
					decode_3_arg(op);
					RAM[arg1] = arg2+arg3;
					break;
				case OP_SUB:
					decode_3_arg(op);
					RAM[arg1] = arg2-arg3;
					break;
				case OP_MUL:
					decode_3_arg(op);
					RAM[arg1] = arg2*arg3;
					break;
				case OP_DIV:
					decode_3_arg(op);
					RAM[arg1] = arg2/arg3;
					break;
			}
			//logic/bitwise ops
			{
				case OP_NOT:
					decode_2_arg(op);
					RAM[arg1] = !arg2;
					break;
				case OP_AND:
					decode_3_arg(op);
					RAM[arg1] = arg2 & arg3;
					break;							
				case OP_IOR:
					decode_3_arg(op);
					RAM[arg1] = arg2 | arg3;
					break;				
				case OP_XOR:
					decode_3_arg(op);
					RAM[arg1] = arg2 ^ arg3;
					break;				
				case OP_LSL:
					decode_shift_arg(op);
					RAM[arg1] = arg2 << arg3;
					break;
				case OP_RSL:
					decode_shift_arg(op);
					RAM[arg1] = arg2 >> arg3;
					break;
			}
			//branch/jump
			{
				case OP_JMP:
					decode_1_arg(op);
					NPC = arg1;
					break;
				case OP_JEQ:
					decode_3_arg(op);
					if(arg2 == arg3)
						NPC = arg1;
					break;
				case OP_JNE:
					decode_3_arg(op);
					if(arg2 != arg3)
						NPC = arg1;
					break;
				case OP_JLT:
					decode_3_arg(op);
					if(arg2 < arg3)
						NPC = arg1;
					break;
				case OP_JGT:
					decode_3_arg(op);
					if(arg2 > arg3)
						NPC = arg1;
					break;
			}
			//cpu state
			{
				case OP_HALT:
					humidor.cpu.halted = true;
					return;
			}
		}

		PC = NPC;
	}
}
#undef RAM
#undef PC
#undef NPC