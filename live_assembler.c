int data_start = 200;
#define data(i) (start_address+data_start+i)
int next_word = start_address;


#define a_mode(op) ((op & 0x80) >> 7)
#define b_mode(op) ((op & 0x40) >> 6)
#define c_mode(op) ((op & 0x20) >> 5)

//immediate value into address dest
u32 set(u32 dest, u32 val, AddressingMode dest_m, AddressingMode val_m) 
{ 
	printf("next word:%d\n", next_word);
	printf("set %u %u\n", dest, val);
	u32 i = (dest_m << 7) | (val_m << 6) | OP_SET;
	RAM[next_word] = i;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
	RAM[next_word] = val;
	next_word++;

	return i;
}

mult(u32 dest, u32 lhs, u32 rhs, AddressingMode dest_m, AddressingMode lh_m, AddressingMode rh_m)
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

add(u32 dest, u32 lhs, u32 rhs, AddressingMode dest_m, AddressingMode lh_m, AddressingMode rh_m)
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

jlt(u32 dest, u32 lhs, u32 rhs, AddressingMode dest_m, AddressingMode lh_m, AddressingMode rh_m)
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

inc(u32 addr)
{
	RAM[next_word] = OP_INC;
	next_word++;
	RAM[next_word] = addr;
	next_word++;
}

u32 halt()
{
	RAM[next_word++] = OP_HALT;
	return OP_HALT;
}

jmp(u32 dest, AddressingMode dest_m)
{
	RAM[next_word] = (dest_m << 7)| OP_JMP;
	next_word++;
	RAM[next_word] = dest;
	next_word++;
}

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
