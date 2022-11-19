#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef unsigned char byte;
#define kilobyte (1024)
#define megabyte (1024*1024)
typedef unsigned long long u64;
#define u32 unsigned int
#define capacity 15*megabyte
#define bytes_per_u64 8
#define memory_length capacity/bytes_per_u64

#define frame_buffer 600
static int width = 480, height = 270;
#define gamepad frame_buffer+width*height

typedef enum Instruction
{
	NOP,
	AND,
	OR,
	XOR,
	left_shift,
	right_shift,
	INC,
	DEC,
	ADD,
	SUB,
	MULT,
	DIV,
	loadA_immediate,
	loadB_immediate,
	loadA_memory,
	loadB_memory,
	STORE,
	JUMP,
	JEQ,
	JNEQ,
	JLT,
	JLE,
	JGT,
	JGE,
	WIPE,
	DRAW,
	SET,
	LOAD,
	opcode_count,
	HALT, //TODO
} Instruction;

char* names[opcode_count] =
{
	"NOP",
	"AND",
	"OR",
	"XOR",
	"left_shift",
	"right_shift",
	"INC",
	"DEC",
	"ADD",
	"SUB",
	"MULT",
	"DIV",
	"loadAi",
	"loadBi",
	"loadAm",
	"loadBm",
	"STORE",
	"JUMP",
	"JEQ",
	"JNEQ",
	"JLT",
	"JLE",
	"JGT",
	"JGE",
	"WIPE",
	"DRAW",
	"SET",
	"LOAD",
};

//TODO switch to 32 bit for ease of writing pixels/audio samples
u64 RAM[memory_length];
#define ram ((byte*)(&RAM))
u64 A = 0;
u64 B = 0;
u64 O = 0;
int PC = 0;
int NPC = 0; //next program counter.
short SP = 0; //stack pointer not in use yet

Instruction I;


void reset()
{
	memset(ram, 0, capacity);
	A = 0;
	B = 0;
	O = 0;
	PC = 0;
}

void load_ROM(char* path)
{
	FILE* file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	int bytes_long = ftell(file);
	rewind(file);
	fread(RAM, bytes_long, 1, file);
}

void fetch()
{
	I = RAM[PC];
}

void jump_cond(bool cond)
{
	if(cond)
		NPC = RAM[PC + 1];
	else
		NPC++; //skip the next value (which is just the jump address we didn't use)
}

void execute()
{
    NPC = PC + 1;

	switch(I)
	{
		case NOP:
			break;
		case AND:
			O = A & B;
			break;
		case OR:
			O = A | B;
			break;
		case XOR:
			O = A ^ B;
			break;
		case left_shift:
			O = A << B;
			break;
		case right_shift:
			O = A >> B;
			break;
		case INC:
			O = A + 1;
			break;
		case DEC:
			O = A - 1;
			break;
		case ADD:
			O = A + B;
			break;
		case SUB:
			O = A - B;
			break;
		case MULT:
			O = A * B;
			break;
		case DIV:
			O = A / B;
			break;
		case loadA_immediate:
			I &= ~0x80; //set up dest bit
			I &= ~0x40; //set up addr-mode bit
			goto ld;
		case loadB_immediate:
			I |= 0x80; //set up dest bit
			I &= ~0x40; //set up addr-mode bit
			goto ld;
		case loadA_memory:
			I &= ~0x80; //set up dest bit
			I |= 0x40; //set up addr-mode bit
			goto ld;
		case loadB_memory:
			I |= 0x80; //set up dest bit
			I |= 0x40; //set up addr-mode bit
			goto ld;
		case STORE:
			RAM[RAM[PC+1]] = O;
			NPC++;
			break;
/*note: any attempted jump to an invalid address just resets PC to 0
what happens if you attempt an instruction at an address where the values it's accessing are invalid?
*/
		case JUMP:
			NPC = RAM[PC + 1];
			break;
		case JEQ:
			jump_cond(A == B);
			break;
		case JNEQ:
			jump_cond(A != B);
			break;
		case JLT:
			jump_cond(A <  B);
			break;
		case JLE:
			jump_cond(A <= B);
			break;
		case JGT:
			jump_cond(A >  B);
			break;
		case JGE:
			jump_cond(A >= B);
			break;
		case WIPE: //clears the frame buffer
			for(int i = 0; i < width*height;i++)
				RAM[frame_buffer+i] = 0;
			break;
		case DRAW:
			break; //outputs frame buffer to device screen. handled by client app
		case SET:
			RAM[RAM[PC + 1]] = RAM[PC + 2];
			NPC++;
			break;
		case LOAD:
		{
			ld:
			{
				//todo encode dest and address mode in instruction byte in actual bin file

				//check destination bit
				u64 *dest = (I&0x80) ? (&B) : (&A);
				//check address-mode bit
				if(I&0x40)
					*dest = RAM[RAM[PC + 1]];
				else
					*dest = RAM[PC + 1];
				NPC++;
			}
		} break;				
		default:
			printf("unimplemented instruction: %d\n", I);												
	}

	PC = NPC;
	
	if(PC  >= memory_length)
		PC = 0;
}