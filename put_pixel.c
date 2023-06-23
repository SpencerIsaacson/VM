//todo add pass to resolve jump labels so you can call labels before they're created

char *_main = "main";
char *put_pixel = "put_pixel";

//variable addresses
char *x = alias("x", data(0));
char *y = alias("y", data(1));
char *color = alias("color", data(2));
char *pixel = alias("pixel", data(3));
char *sp =  alias("sp", data(4));
char *stack = alias("stack", data(5));

label(_main);
	set(a(color), 0xFFFF00, IMM, IMM);
	set(a(x), 12, IMM, IMM);
	set(a(y), 34, IMM, IMM);
	set(a(stack), start_address+((3*4)+2), IMM, IMM);
	jmp(get_jump_label(put_pixel),IMM);
	set(a(color), 0x00FFFF, IMM, IMM);
	set(a(x), 58, IMM, IMM);
	set(a(y), 16, IMM, IMM);
	set(a(stack), start_address+2*((3*4)+2), IMM, IMM);
	jmp(get_jump_label(put_pixel),IMM);
	halt();
label(put_pixel);
	set(a(pixel), vm_width, IMM, IMM);
	mult(a(pixel), a(y), a(pixel), IMM, IND, IND);
	add(a(pixel), a(x), a(pixel), IMM, IND, IND);
	add(a(pixel), screen_address, a(pixel), IMM, IMM, IND);
	set(a(pixel),a(color), IND,IND);
	jmp(a(stack), IND);

