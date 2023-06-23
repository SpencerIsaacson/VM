alias e 151858
alias c 151859
alias pix 151860

draw_diagonal:
	mov 14 e
	mov 0 c
loop:
	mult @c width pix
	add @pix @c pix
	add @pix screen pix
	mov white @pix
	inc c
	jlt @c @e loop