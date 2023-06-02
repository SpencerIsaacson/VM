#define e (start_address+200)
#define c (start_address+201)
#define pix 202
#define width vm_width
#define screen 200

mov(14,e);
mov(0,c);
label("loop");
mov(c, pix);
mult(c, width, pix);
add(pix, c, pix);
add(pix, screen, pix);
mov_ind(white, pix);
inc(c);
jlt(c,e,"loop");