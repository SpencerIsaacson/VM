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
	u32 count;
	u32 address;
} ChunkHeader;

typedef struct
{
	char start_code[4];
	u32 chunk_count;
} ROMHeader;

typedef enum
{
	ROMType_FULL = 0, //simply freads the entire contents of the file after the start code into memory
	ROMType_MAPPED = 1, //does a chunked load to save space on the rom file
} ROMType;

//maps the chunks of a ROMType_MAPPED ROM file into memory (called only by load_ROM)
bool load_ROM_mapped(FILE *file, ROMHeader rom_header)
{
	for (int current_chunk = 0; current_chunk < rom_header.chunk_count; ++current_chunk)
	{
		ChunkHeader chunk_header;
		byte bytes_read = fread(&chunk_header, 1, sizeof(ChunkHeader), file);
		
		if(bytes_read != sizeof(ChunkHeader))
			return false;

		bytes_read = fread(&RAM[chunk_header.address], 1, chunk_header.count, file);

		if(bytes_read != chunk_header.count)
			return false;
	}

	return true;
}

//loads the contents of a ROMType_FULL ROM file directly into memory following the start code (called only by load_ROM)
bool load_ROM_full(FILE *file, ROMHeader rom_header)
{
	if(file)
	{
		fseek(file, 0, SEEK_END);
		int bytes_long = ftell(file) - 4;//adjust size by start code
		fseek(file, 4, SEEK_SET);
		fread(RAM, 1, bytes_long, file);
		return true;
	}

	return false;
}

//loads the contents of a ROM file into memory, checking for a ROM header and then mapping the chunks accordingly
bool load_ROM(char *path)
{
	FILE* file = fopen(path, "rb");

	if(file)
	{
		ROMHeader rom_header;
		byte bytes_read = fread(&rom_header, 1, sizeof(ROMHeader), file);

		if(bytes_read != sizeof(ROMHeader))
			return false;

		//todo strcmp
		if(rom_header.start_code[0] != 'H' || rom_header.start_code[1] != 'U' || rom_header.start_code[2] != 'M')
			return false;

		ROMType rom_type = rom_header.start_code[3];

		if(rom_type == ROMType_MAPPED)
			return load_ROM_mapped(file, rom_header);

		if(rom_type == ROMType_FULL)
			return load_ROM_full(file, rom_header);
	}

	return false;
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