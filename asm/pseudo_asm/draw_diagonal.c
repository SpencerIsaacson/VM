

char *draw_diagonal = "draw_diagonal";
char *loop = "loop";

char *e = alias("e", data(0));
char *c = alias("c", data(1));
char *pix = alias("pix", data(2));
char *draw_col = alias("draw_col", data(3));
char *len = alias("len", data(4));
char *screen_width = alias("screen_width", vm_width);
char *_white = alias("white", 0x00FFFFFF);
char *screen = alias("screen", screen_address);

set(a(draw_col), 0xFF0000, IMM, IMM);
set(a(len), 1, IMM, IMM);
label(draw_diagonal);
	set(a(e), a(len), IMM, IND);
	set(a(c), 0, IMM, IMM);
	label(loop);
		mult(a(pix), a(screen_width), a(c), IMM, IMM, IND);
		add(a(pix), a(c), a(pix), IMM, IND, IND);
		add(a(pix), a(screen), a(pix), IMM, IMM, IND);
		set(a(pix), a(draw_col), IND, IND);
		inc(a(c));
		jlt(get_jump_label(loop), a(c),a(e), IMM, IND, IND);
		inc(a(draw_col));
		inc(a(len));
		jmp(get_jump_label(draw_diagonal), IMM);
