#define kilobyte (1024)
#define megabyte (1024*1024)

typedef unsigned char byte;
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
typedef struct AudioBuffer
{
	AudioSample samples[22050];
} AudioBuffer;

#define capacity (15*megabyte)
#define word_count (capacity/sizeof(u32))
typedef union Memory
{
	struct
	{
		GamePad     game_pads[4];
		AudioBuffer audio_buffer;
		FrameBuffer frame_buffer;
	};
	
	u32			RAM[word_count];
} Memory;

typedef struct CPU
{
	u32 A;
	u32 B;
	u32 O;
	int PC; //program counter
	int NPC; //next program counter.just a helper value
	short SP; //stack pointer not in use yet
	short sample_pointer; //not REALLY part of the CPU
	bool halted;
} CPU;

typedef struct StronkBox
{
	CPU cpu;
	Memory memory;
} StronkBox;

StronkBox stronkbox;

typedef enum
{
	IMM = 0,
	IND = 1,
} AddressingMode;

typedef enum OpCode
{
	OP_NOP,
	OP_SET,
	OP_MULT,
	OP_INC,
	OP_ADD,
	OP_JLT,
	OP_JMP,
	OP_HALT,
} OpCode;

char* OpNames[8] = 
{
	"NOP",
	"SET",
	"MULT",
	"INC",
	"ADD",
	"JLT",
	"JMP",
	"HALT",
};

#define RAM stronkbox.memory.RAM
#define A   stronkbox.cpu.A
#define B   stronkbox.cpu.B
#define O   stronkbox.cpu.O
#define PC  stronkbox.cpu.PC
#define NPC stronkbox.cpu.NPC

//helpers
#define start_address ((sizeof(stronkbox.memory.game_pads)+sizeof(stronkbox.memory.audio_buffer)+sizeof(stronkbox.memory.frame_buffer))/sizeof(u32))
#define screen_address ((sizeof(stronkbox.memory.game_pads)+sizeof(stronkbox.memory.audio_buffer))/sizeof(u32))


#include "live_assembler.c"

void reset()
{
	memset(&stronkbox, 0, sizeof(stronkbox));
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
	if(!stronkbox.cpu.halted)
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
			case OP_MULT:
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
			case OP_JMP:
			{
				if(a_mode(op))
					NPC = RAM[RAM[PC+1]];
				else
					NPC = RAM[PC+1];
			} break;
			case OP_HALT:
			{
				printf("HALTED!\n");
				stronkbox.cpu.halted = true;
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


#undef RAM
#undef A
#undef B
#undef O
#undef PC
#undef NPC