
//todo move code to "start address?"
#define _code (next_word++)
#define RAM stronkbox.memory.RAM
int data_start = 200;
#define data(i) (start_address+data_start+i)

#define color  RAM[data(0)]
#define x      RAM[data(1)]
#define y      RAM[data(2)]
#define width  RAM[data(3)]
#define height RAM[data(4)]
#define _x     RAM[data(5)]
#define _y     RAM[data(6)]

char start_row[9] = "start_row";
char put_pixel[9] = "put_pixel";

RAM[_code] = LOAD|d_A|s_imm;
RAM[_code] = 0;

RAM[_code] = STORE|d_A|s_imm;
RAM[_code] = _y;
label(start_row);
RAM[_code] = LOAD|d_A|s_imm;
RAM[_code] = 0;

RAM[_code] = STORE|d_A|s_imm;
RAM[_code] = _x;
label(put_pixel);
RAM[_code] = LOAD|d_A|s_imm;
RAM[_code] = 0;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = y;

RAM[_code] = ADD;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = _y;

RAM[_code] = ADD;

RAM[_code] = LOAD|d_B|s_imm;
RAM[_code] = 640;

RAM[_code] = MULT_IMM;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = x;

RAM[_code] = ADD;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = _x;

RAM[_code] = ADD;

RAM[_code] = LOAD|d_B|s_imm;
RAM[_code] = screen_address;

RAM[_code] = ADD_IMM;

RAM[_code] = TAB;

RAM[_code] = LOAD|d_A|s_mem;
RAM[_code] = color;

RAM[_code] = STORE|d_A|s_B;

RAM[_code] = LOAD|d_A|s_mem;
RAM[_code] = _x;

RAM[_code] = INC;

RAM[_code] = STORE|d_A|s_imm;
RAM[_code] = _x;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = width;

RAM[_code] = JLT;
RAM[_code] = jump_label(put_pixel);

RAM[_code] = LOAD|d_A|s_mem;
RAM[_code] = _y;

RAM[_code] = INC;

RAM[_code] = LOAD|d_B|s_mem;
RAM[_code] = height;

RAM[_code] = JLT;
RAM[_code] = jump_label(start_row);