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
} CPU;

typedef struct StronkBox
{
	CPU cpu;
	Memory memory;
} StronkBox;

StronkBox stronkbox;

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
	ADD_IMM,
	SUB,
	MULT,
	MULT_IMM,
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
	DRAW,
	SET,
	LOAD,
	STAB,
	TAB,
	TBA,
	TOA,
	MOV_IMM,
	opcode_count,
} Instruction;

#define RAM stronkbox.memory.RAM
#define A   stronkbox.cpu.A
#define B   stronkbox.cpu.B
#define O   stronkbox.cpu.O
#define PC  stronkbox.cpu.PC
#define NPC stronkbox.cpu.NPC

void reset()
{
	memset(&stronkbox, 0, sizeof(stronkbox));
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

void jump_cond(bool cond)
{
	if(cond)
		NPC = RAM[PC + 1];
	else
		NPC++; //skip the next value (which is just the jump address we didn't use)
}

typedef enum DestMode
{
	d_A = 0,
	d_B = 0x80,
} DestMode;

typedef enum SourceMode
{
	s_imm = 0,
	s_mem = 0x40,
	s_B   = 0x40,
} SourceMode;

void tick()
{
	Instruction I = RAM[PC];

    NPC = PC + 1;

    Instruction OPCODE = I & 0x3F;
	switch(OPCODE)
	{
		case NOP:
			break;
		case AND:
			A = A & B;
			break;
		case OR:
			A = A | B;
			break;
		case XOR:
			A = A ^ B;
			break;
		case left_shift:
			A = A << B;
			break;
		case right_shift:
			A = A >> B;
			break;
		case INC:
			A++;
			break;
		case DEC:
			A--;
			break;
		case ADD:
			A = A + B;
			break;
		case SUB:
			A = A - B;
			break;
		case MULT:
			A = A * B;
			break;
		case DIV:
			A = A / B;
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
		case JUMP: //note: any attempted jump to an invalid address just resets PC to 0 what happens if you attempt an instruction at an address where the values it's accessing are invalid?
			NPC = RAM[PC + 1];
			printf("jump %d\n",NPC);
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
		case DRAW:
			break; //outputs frame buffer to device screen. handled by client app
		case SET:
			RAM[RAM[PC + 1]] = RAM[PC + 2];
			NPC+=2;
			break;
		case LOAD:
		{
			ld:
			{
				//todo encode dest and address mode in instruction byte in actual bin file

				//check destination bit
				u32 *dest = (I&0x80) ? (&B) : (&A);
				//check address-mode bit
				if(I&0x40){
					*dest = RAM[RAM[PC + 1]];
				}
				else
					*dest = RAM[PC + 1];
				NPC++;
			}
		} break;

		case STAB:
		{
			RAM[B] = A;
		} break;
		case TBA:
		{
			A = B;
		} break;
		case TOA:
		{
			A = O;
		} break;
		case MOV_IMM:
		{
			RAM[RAM[PC+2]] = RAM[PC+1];
			NPC+=2;
		} break;
		default:
			printf("unimplemented instruction: %d\n", I);
	}

	PC = NPC;
	
	if(PC >= word_count)
		PC = 0;
}

//helpers
#define start_address ((sizeof(stronkbox.memory.game_pads)+sizeof(stronkbox.memory.audio_buffer)+sizeof(stronkbox.memory.frame_buffer))/sizeof(u32))
#define screen_address ((sizeof(stronkbox.memory.game_pads)+sizeof(stronkbox.memory.audio_buffer))/sizeof(u32))


int next_word = 0;
typedef struct
{
	int count;
	char *keys[100];
	int   vals[100];
} LabelDict;

LabelDict labels;
void label(char *name)
{
	labels.keys[labels.count] = name;
	labels.vals[labels.count] = next_word;
	labels.count++;
}


int get_jump_label(char *name)
{
	for (int i = 0; i < labels.count; ++i)
	{
		if(labels.keys[i] == name)
			return labels.vals[i];
	}

	printf("failure to locate label");
	exit(1);
}


void mov(u32 val, u32 dest) { RAM[dest] = val; }

void mult(u32 a, u32 b, u32 dest) { RAM[dest] = RAM[a] * RAM[b]; }

void add(u32 a, u32 b, u32 dest) { RAM[dest] = RAM[a] + RAM[b]; }

inc(u32 dest) { RAM[dest]++; }

mov_ind(u32 imm, u32 dest) { RAM[RAM[dest]] = imm; }

jlt(u32 a, u32 b, char *label) { if(RAM[a] < RAM[b]) PC = get_jump_label(label); }

#undef RAM
#undef A
#undef B
#undef O
#undef PC
#undef NPC