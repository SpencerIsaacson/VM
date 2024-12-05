#include "macros"
#define data_address 50
#define box_location (screen_address+vm_width*vm_height/2+6)


set(imm(data_address), imm(screen_address));
label(start);
set(ind(data_address), imm(cyan));
inc(imm(data_address));
jlt(imm(g(start)), ind(data_address), imm(screen_address+(vm_width*vm_height/3)));
label(check_input);
	jgt(imm(g(respond_to_input)),ind(pad_address), imm(0));
	jmp(imm(g(check_input)));
label(respond_to_input);
	set(imm(box_location), imm(red));
label(end);
	jmp(imm(g(end)));