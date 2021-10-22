; This is a commented line
;

.entry Calc
; $1 - Address to write to, 
; $2 - first initial value, $3 - second initial value, $4, $5 - used in program

Calc:	la INITIALS
		lw $0, 0, $2
		lw $0, 1, $3
		la MAX
		lw $0, 0, $5
		
		sw $1, 0 , $2
		addi  $1, 1, $1
		sw $1, 1,  $3
		addi  $1, 1, $1

LOOP:	lw $1, -2, $2
		lw $1, -1, $3
		add $2, $3, $4
		sw  $1, 0, $4
		addi  $1, 1, $1
		blt $4, $5, LOOP
		
INITIALS: .dw 0, 1
MAX: .dw 10000