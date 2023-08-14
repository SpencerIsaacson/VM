int next_word = start_address;

/*
	A brief note: this system currently supports a fixed width and variable width encoding until further notice.
	The kinks are still being worked out. The fixed width (fw) and variable width (vw) emitters work very differently.
	The vw emitters assemble the instructions *into* the console memory and advance the "next_word" pointer.
	The fw emitters just return a u32 that represents the full instruction, and are in that way far more flexible.
*/

typedef struct { AddrMode mode; u8  val; } Op8;
typedef struct { AddrMode mode; u8  val; } Op5;
typedef struct { AddrMode mode; u16 val; } Op11;
typedef struct { AddrMode mode; u16 val; } Op16;
typedef struct { AddrMode mode; u32 val; } Op24;

//INSTRUCTION FORMATS
//todo guarantee args are in proper ranges

//Variable Width

//todo

//Fixed Width

//for ops that take no arguments
u32 emit_0_arg_fw(OpCode opcode)
{ return opcode << 24; }

//for ops that take 1 24-bit arg
u32 emit_1_arg_fw(OpCode opcode, Op24 addr)
{   return (((addr.mode << 7) | opcode) << 24) | addr.val;   }

//for ops that take 1 8-bit arg and 1 16-bit arg
u32 emit_2_arg_fw(OpCode opcode, Op8 dest, Op16 val) 
{   return ( ((dest.mode << 7) | (val.mode << 6) | opcode) << 24) | (dest.val << 16) | (val.val);   }

//for ops that take 3 8-bit args
u32 emit_3_arg_fw(OpCode opcode, Op8 dest, Op8 lhs, Op8 rhs)
{   return (((dest.mode << 7) | (lhs.mode << 6) | (rhs.mode << 5)  | opcode) << 24) | (dest.val << 16) | (lhs.val << 8) | (rhs.val);   }

//for ops that take 1 8-bit arg, 1 11-bit arg, and 1 5-bit arg
u32 emit_shift_arg_fw(OpCode opcode, Op8 dest, Op11 lhs, Op5 rhs)
{   return (((dest.mode << 7) | (lhs.mode << 6) | (rhs.mode << 5)  | opcode) << 24) | (dest.val << 16) | (lhs.val << 5) | (rhs.val);   }

#define      ARG0(func_name, opcode) u32 func_name()                     { return     emit_0_arg_fw(opcode); }
#define      ARG1(func_name, opcode) u32 func_name(Op24 a)               { return     emit_1_arg_fw(opcode, a); }
#define      ARG2(func_name, opcode) u32 func_name(Op8 a, Op16 b)        { return     emit_2_arg_fw(opcode, a, b); }
#define      ARG3(func_name, opcode) u32 func_name(Op8 a, Op8  b, Op8 c) { return     emit_3_arg_fw(opcode, a, b, c); }
#define ARG_SHIFT(func_name, opcode) u32 func_name(Op8 a, Op11 b, Op5 c) { return emit_shift_arg_fw(opcode, a, b, c); }

#define emit_func(func_name, opcode, format) \
    format(func_name, opcode)

//EMITTERS

//Variable Width

//basics
void nop_vw()
{   RAM[next_word++] = OP_NOP;   }

//todo add addr mode like in fw
void inc_vw(u32 addr)
{
	RAM[next_word] = OP_INC;
	next_word++;
	RAM[next_word] = addr;
	next_word++;
}

void dec_vw(u32 addr)
{
	RAM[next_word] = OP_DEC;
	next_word++;
	RAM[next_word] = addr;
	next_word++;
}

void set_vw(AddrMode dest_m, u32 dest, AddrMode val_m, u32 val) 
{
	u32 i = (dest_m << 7) | (val_m << 6) | OP_SET;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = val;
	next_word++;
}

//arithmetic
void add_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_ADD;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;	
}

void sub_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_SUB;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;	
}

void mult_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_MUL;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;	
}

void div_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_DIV;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;	
}

//logic/bitwise
void not_vw(AddrMode dest_m, u32 dest, AddrMode val_m, u32 val)
{   /*todo*/   }

void and_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{   /*todo*/   }

void or_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{   /*todo*/   }

void xor_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{   /*todo*/   }

void lsl_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{   /*todo*/   }

void rsl_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{   /*todo*/   }

//branch/jump
u32 jmp_vw(AddrMode dest_m, u32 dest)
{
	RAM[next_word] = (dest_m << 7)| OP_JMP;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
}

void jeq_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_JEQ;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;
}

void jne_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_JNE;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;
}

void jlt_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_JLT;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;
}

void jgt_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_JGT;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = lhs;
	next_word++;
	RAM[next_word] = rhs;
	next_word++;
}

//cpu state
void halt_vw()
{   RAM[next_word++] = OP_HALT;   }

//Fixed Width
#define f emit_func
//basics
f(nop_fw, OP_NOP, ARG0);
f(inc_fw, OP_INC, ARG1);
f(dec_fw, OP_DEC, ARG1);
f(set_fw, OP_SET, ARG2);
//arithmetic
f(add_fw, OP_ADD, ARG3);
f(sub_fw, OP_SUB, ARG3);
f(mul_fw, OP_MUL, ARG3);
f(div_fw, OP_DIV, ARG3);
//logic/bitwise
f(not_fw, OP_NOT, ARG2);
f(and_fw, OP_AND, ARG3);
f(ior_fw, OP_IOR, ARG3);
f(xor_fw, OP_XOR, ARG3);
f(lsl_fw, OP_LSL, ARG_SHIFT);
f(rsl_fw, OP_RSL, ARG_SHIFT);
//branch/jump
f(jmp_fw, OP_JMP, ARG1);
f(jeq_fw, OP_JEQ, ARG3);
f(jne_fw, OP_JNE, ARG3);
f(jlt_fw, OP_JLT, ARG3);
f(jgt_fw, OP_JGT, ARG3);
//cpu state
f(halt_fw, OP_HALT, ARG0);
#undef f

typedef struct
{
	int count;
	char *keys[100];
	int   vals[100];
} Dict;

Dict labels;
Dict unresolved_labels;
void label(char *name)
{
	//check for unresolved entry
	for (int i = 0; i < unresolved_labels.count;)
	{
		if(unresolved_labels.keys[i] == name)
		{
			RAM[unresolved_labels.vals[i]] = next_word;
			
			//remove unresolved entry
			{
				unresolved_labels.keys[i] = unresolved_labels.keys[unresolved_labels.count-1];
				unresolved_labels.vals[i] = unresolved_labels.vals[unresolved_labels.count-1];
				unresolved_labels.count--;
			}
		}
		else i++;
	}

	printf("\nlabel %s: %d\n", name, next_word);
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

	//put dummy entry into label dict for later resolution 
	unresolved_labels.keys[unresolved_labels.count] = name;
	unresolved_labels.vals[unresolved_labels.count] = next_word+1; //marker to let label function know this requires resolving
	printf("unresolved:%d\n",next_word+1);
	unresolved_labels.count++;
}

Dict aliases;
char* alias(char *name, u32 word)
{
	aliases.keys[aliases.count] = name;
	aliases.vals[aliases.count] = word;
	aliases.count++;
	return name;
}

u32 get_alias(char* alias)
{
	for (int i = 0; i < aliases.count; ++i)
	{
		if(aliases.keys[i] == alias)
			return aliases.vals[i];
	}

	printf("failure to locate alias: %s\n", alias);
	exit(1);	
}


