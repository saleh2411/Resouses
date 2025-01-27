# input matrices

.word 0x100 5
.word 0x101 3
.word 0x102 5
.word 0x103 1
.word 0x104 1
.word 0x105 5
.word 0x106 4
.word 0x107 5
.word 0x108 5
.word 0x109 1
.word 0x10A 5
.word 0x10B 5
.word 0x10C 5
.word 0x10D 3
.word 0x10E 4
.word 0x10F 4
.word 0x110 4
.word 0x111 1
.word 0x112 1
.word 0x113 2
.word 0x114 4
.word 0x115 4
.word 0x116 1
.word 0x117 5
.word 0x118 2
.word 0x119 1
.word 0x11A 5
.word 0x11B 1
.word 0x11C 4
.word 0x11D 2
.word 0x11E 4
.word 0x11F 3

	add $s0,$zero,$imm,0x100	# load address of input matrix 1 to s0
	add $s1,$zero,$imm,0x110	# load address of input matrix 2 to s1
	add $s2,$zero,$imm,0x120	# load address of output matrix to s2
	add $t0,$zero,$zero,0		# t0 will store the loop index. set to 0
	add $t3,$zero,$imm,16		# t3 will contain the value 16 and help stop the loop
LoopStart:
	lw $t1,$s0,$t0,0			# load $t0'th cell of first matrix to $t1
	lw $t2,$s1,$t0,0			# load $t0'th cell of second matrix to $t2
	add $t2,$t1,$t2,0			# perform sum operation and store it in $t2
	sw $t2,$s2,$t0,0			# save result in output matrix
	add $t0,$t0,$imm,1			# increase loop index by one
	blt $imm,$t0,$t3,LoopStart  # go back to "LoopStart" if the $t0 index did not reach 16 yet
	halt $zero, $zero, $zero, 0	# if the jump does not happen($t0 index did reach 16). stop the program