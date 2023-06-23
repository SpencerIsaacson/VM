//constants
alias screen_width 480
alias screen_address 22058
alias yellow    0xFFFF00
alias cyan		0x00FFFF
//variable addresses
alias x     151858
alias y     151859
alias color 151860
alias pixel 151861
alias sp	151862
//todo increment call stack to allow more than one level of calls
alias stack 151863

main:
	set color yellow
	set x 12
	set y 32
//todo relocatability (relative jump?)
	set stack 151672
	jmp put_pixel
	set color cyan
	set x 58
	set y 16
	set stack 151686
	jmp put_pixel	
	halt 
put_pixel:
	set pixel screen_width
	mult pixel @pixel @y 
	add pixel @pixel @x
	add pixel @pixel screen_address
	set @pixel @color
	jmp @stack