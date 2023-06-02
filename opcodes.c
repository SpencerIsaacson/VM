#define RAM stronkbox.memory.RAM
//immediate value into address dest
void mov(u32 val, u32 dest) { RAM[dest] = val; }

void mult(u32 a, u32 b, u32 dest) { RAM[dest] = RAM[a] * RAM[b]; }

void add(u32 a, u32 b, u32 dest) { RAM[dest] = RAM[a] + RAM[b]; }

inc(u32 dest) { RAM[dest]++; }

mov_ind(u32 imm, u32 dest) { RAM[RAM[dest]] = imm; }

jlt(u32 a, u32 b, char *label) { if(RAM[a] < RAM[b]) stronkbox.CPU.PC = get_jump_label(label); }

#undef RAM