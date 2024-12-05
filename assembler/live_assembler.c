int next_word = start_address;

/*
	A brief note: this system currently supports a fixed width and variable width encoding until further notice.
	The kinks are still being worked out. The fixed width (fw) and variable width (vw) emitters work very differently.
	The vw emitters assemble the instructions *into* the console memory and advance the "next_word" cursor.
	The fw emitters just return a u32 that represents the full instruction, and are in that way far more flexible.
*/


//Operand Types
typedef struct { AddrMode mode; u8  val; } Op8;
typedef struct { AddrMode mode; u8  val; } Op5;
typedef struct { AddrMode mode; u16 val; } Op11;
typedef struct { AddrMode mode; u16 val; } Op16;
typedef struct { AddrMode mode; u32 val; } Op24;
typedef struct { AddrMode mode; u32 val; } Op32;

//INSTRUCTION FORMATS
//todo guarantee args are in proper ranges (asserts)

//Variable Width

void emit_0_arg_vw(OpCode opcode)
{ RAM[next_word++] = opcode; }

void emit_1_arg_vw(OpCode opcode, Op32 addr)
{
	RAM[next_word++] = (addr.mode << 7) | opcode;
	RAM[next_word++] = addr.val;
}

void emit_2_arg_vw(OpCode opcode, Op32 dest, Op32 val) 
{
	u32 i = (dest.mode << 7) | (val.mode << 6) | opcode;
	RAM[next_word++] = i;
	RAM[next_word++] = dest.val;
	RAM[next_word++] = val.val;
}

void emit_3_arg_vw(OpCode opcode, Op32 dest, Op32 lhs, Op32 rhs)
{
	u32 i = (dest.mode << 7) | (lhs.mode << 6) | (rhs.mode << 5)  | opcode;
	RAM[next_word++] = i;
	RAM[next_word++] = dest.val;
	RAM[next_word++] = lhs.val;
	RAM[next_word++] = rhs.val;
}

#define VW0(func_name, opcode) void func_name()							{ emit_0_arg_vw(opcode); }
#define VW1(func_name, opcode) void func_name(Op32 a)					{ emit_1_arg_vw(opcode, a); }
#define VW2(func_name, opcode) void func_name(Op32 a, Op32 b)			{ emit_2_arg_vw(opcode, a, b); }
#define VW3(func_name, opcode) void func_name(Op32 a, Op32 b, Op32 c)	{ emit_3_arg_vw(opcode, a, b, c); }

//Fixed Width

//for ops that take no arguments
u32 emit_0_arg_fw(OpCode opcode)
{ return opcode << 24; }

//for ops that take 1 24-bit arg
u32 emit_1_arg_fw(OpCode opcode, Op24 addr)
{   addr.val &= 0xFFFFFF; return (((addr.mode << 7) | opcode) << 24) | addr.val;   }

//for ops that take 1 8-bit arg and 1 16-bit arg
u32 emit_2_arg_fw(OpCode opcode, Op8 dest, Op16 val)
{   return ( ((dest.mode << 7) | (val.mode << 6) | opcode) << 24) | (dest.val << 16) | (val.val);   }

//for ops that take 3 8-bit args
u32 emit_3_arg_fw(OpCode opcode, Op8 dest, Op8 lhs, Op8 rhs)
{   return (((dest.mode << 7) | (lhs.mode << 6) | (rhs.mode << 5)  | opcode) << 24) | (dest.val << 16) | (lhs.val << 8) | (rhs.val);   }

//for ops that take 1 8-bit arg, 1 11-bit arg, and 1 5-bit arg
u32 emit_shift_arg_fw(OpCode opcode, Op8 dest, Op11 lhs, Op5 rhs)
{   return (((dest.mode << 7) | (lhs.mode << 6) | (rhs.mode << 5)  | opcode) << 24) | (dest.val << 16) | (lhs.val << 5) | (rhs.val);   }


#define FW0(func_name, opcode) u32 func_name()                     { return     emit_0_arg_fw(opcode); }
#define FW1(func_name, opcode) u32 func_name(Op24 a)               { return     emit_1_arg_fw(opcode, a); }
#define FW2(func_name, opcode) u32 func_name(Op8 a, Op16 b)        { return     emit_2_arg_fw(opcode, a, b); }
#define FW3(func_name, opcode) u32 func_name(Op8 a, Op8  b, Op8 c) { return     emit_3_arg_fw(opcode, a, b, c); }
#define FWS(func_name, opcode) u32 func_name(Op8 a, Op11 b, Op5 c) { return emit_shift_arg_fw(opcode, a, b, c); }

#define emit_func(func_name, opcode, format) \
    format(func_name, opcode)

//EMITTERS
#define f emit_func

//Variable Width
//basics
f(nop_vw, OP_NOP, VW0);
f(inc_vw, OP_INC, VW1);
f(dec_vw, OP_DEC, VW1);
f(set_vw, OP_SET, VW2);
//arithmetic
f(add_vw, OP_ADD, VW3);
f(sub_vw, OP_SUB, VW3);
f(mul_vw, OP_MUL, VW3);
f(div_vw, OP_DIV, VW3);
//logic/bitwise
f(not_vw, OP_NOT, VW2);
f(and_vw, OP_AND, VW3);
f(ior_vw, OP_IOR, VW3);
f(xor_vw, OP_XOR, VW3);
f(lsl_vw, OP_LSL, VW3);
f(rsl_vw, OP_RSL, VW3);
//branch/jump
f(jmp_vw, OP_JMP, VW1);
f(jeq_vw, OP_JEQ, VW3);
f(jne_vw, OP_JNE, VW3);
f(jlt_vw, OP_JLT, VW3);
f(jgt_vw, OP_JGT, VW3);
//cpu state
f(halt_vw, OP_HALT, VW0);

//Fixed Width
//basics
f(nop_fw, OP_NOP, FW0);
f(inc_fw, OP_INC, FW1);
f(dec_fw, OP_DEC, FW1);
f(set_fw, OP_SET, FW2);
//arithmetic
f(add_fw, OP_ADD, FW3);
f(sub_fw, OP_SUB, FW3);
f(mul_fw, OP_MUL, FW3);
f(div_fw, OP_DIV, FW3);
//logic/bitwise
f(not_fw, OP_NOT, FW2);
f(and_fw, OP_AND, FW3);
f(ior_fw, OP_IOR, FW3);
f(xor_fw, OP_XOR, FW3);
f(lsl_fw, OP_LSL, FWS);
f(rsl_fw, OP_RSL, FWS);
//branch/jump
f(jmp_fw, OP_JMP, FW1);
f(jeq_fw, OP_JEQ, FW3);
f(jne_fw, OP_JNE, FW3);
f(jlt_fw, OP_JLT, FW3);
f(jgt_fw, OP_JGT, FW3);
//cpu state
f(halt_fw, OP_HALT, FW0);
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
	return -1;
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


