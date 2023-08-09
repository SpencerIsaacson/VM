int next_word = start_address;

/*
	A brief note: this system currently supports a fixed width and variable width encoding until further notice.
	The kinks are still being worked out. The fixed width (fw) and variable width (vw) emitters work very differently.
	The vw emitters assemble the instructions *into* the console memory and advance the "next_word" pointer.
	The fw emitters just return a u32 that represents the full instruction, and are in that way far more flexible.
*/

//INSTRUCTION FORMATS
//todo guarantee args are in proper ranges

//Variable Width

//todo

//Fixed Width

//for ops that take 1 24-bit arg
u32 emit_1_arg_fw(OpCode opcode, AddrMode addr_m, u32 addr)
{    return (((addr_m << 7) | opcode) << 24) | addr;    }

//for ops that take 1 8-bit arg and 1 16-bit arg
u32 emit_2_arg_fw(OpCode opcode, AddrMode dest_m, u8 dest, AddrMode val_m, u32 val) 
{    return ( ((dest_m << 7) | (val_m << 6) | opcode) << 24) | (dest << 16) | (val);    }

//for ops that take 3 8-bit args
u32 emit_3_arg_fw(OpCode opcode, AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return (((dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | opcode) << 24) | (dest << 16) | (lhs << 8) | (rhs);    }

//for ops that take 1 8-bit arg, 1 11-bit arg, and 1 5-bit arg
u32 emit_shift_arg_fw(OpCode opcode, AddrMode dest_m, u8 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u8 rhs)
{    return (((dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | opcode) << 24) | (dest << 16) | (lhs << 5) | (rhs);    }

//EMITTERS

//Variable Width

//basics
void nop_vw()
{    RAM[next_word++] = OP_NOP;    }

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
	u32 i = (dest_m << 7) | (lh_m << 6) | (rh_m << 5)  | OP_MULT;
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
{    /*todo*/    }

void and_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{    /*todo*/    }

void or_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{    /*todo*/    }

void xor_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{    /*todo*/    }

void lsl_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{    /*todo*/    }

void rsl_vw(AddrMode dest_m, u32 dest, AddrMode lh_m, u32 lhs, AddrMode rh_m, u32 rhs)
{    /*todo*/    }

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
{    RAM[next_word++] = OP_HALT;    }

//Fixed Width

//basics
u32 nop_fw()
{    return 0;    }

u32 inc_fw(AddrMode addr_m, u32 addr)
{    return emit_1_arg_fw(OP_INC, addr_m, addr);    }

u32 dec_fw(AddrMode addr_m, u32 addr)
{    return emit_1_arg_fw(OP_DEC, addr_m, addr);    }

u32 set_fw(AddrMode dest_m, u8 dest, AddrMode val_m, u32 val) 
{    return emit_2_arg_fw(OP_SET, dest_m, dest, val_m, val);    }

//arithmetic
u32 add_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_ADD, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 sub_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_SUB, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 mult_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_MULT, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 div_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_DIV, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

//logic/bitwise
u32 not_fw(AddrMode dest_m, u8 dest, AddrMode val_m, u32 val)
{    return emit_2_arg_fw(OP_NOT, dest_m, dest, val_m, val);    }

u32 and_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_AND, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 or_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_OR, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 xor_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_XOR, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 lsl_fw(AddrMode dest_m, u32 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_shift_arg_fw(OP_LSL, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 rsl_fw(AddrMode dest_m, u32 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_shift_arg_fw(OP_RSL, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

//branch/jump
u32 jmp_fw(AddrMode dest_m, u32 dest)
{    return emit_1_arg_fw(OP_JMP, dest_m, dest);    }

u32 jeq_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_JEQ, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 jne_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_JNE, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 jlt_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_JLT, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

u32 jgt_fw(AddrMode dest_m, u8 dest, AddrMode lh_m, u8 lhs, AddrMode rh_m, u8 rhs)
{    return emit_3_arg_fw(OP_JGT, dest_m, dest, lh_m, lhs, rh_m, rhs);    }

//cpu state
u32 halt_fw()
{    return OP_HALT << 24;    }

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
		printf("RESOLVE! %s %d %d\n",unresolved_labels.keys[i], unresolved_labels.keys[i], unresolved_labels.vals[i]);
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


