	add $a0, $zero, $zero, 0           		 # x = 0
	add $s1, $zero, $imm, 256          		 # initialize save address to 256
	add $s2, $zero, $imm, 30           		 # set max num
while:
	add $t2, $zero, $imm, 1				# $t2 = 1
	sll $sp, $t2, $imm, 11				# set $sp = 1 << 11 = 2048
	jal $ra, $imm, $zero, fib			# calc $v0 = fib(x)
	sw $v0, $zero, $s1, 0               	# store fib(x) in save address
	add $a0, $a0, $imm, 1               	# x=x+1
	add $s1, $s1, $imm, 1               	# save address = save address + 1
	blt $imm, $a0, $s2, while           	# if x < max num return to while
	halt $zero, $zero, $zero, 0			# halt
fib:
	add $sp, $sp, $imm, -3				# adjust stack for 3 items
	sw $s0, $sp, $imm, 2				# save $s0
	sw $ra, $sp, $imm, 1				# save return address
	sw $a0, $sp, $imm, 0				# save argument
	add $t2, $zero, $imm, 1				# $t2 = 1
	bgt $imm, $a0, $t2, L1				# jump to L1 if x > 1
	add $v0, $a0, $zero, 0				# otherwise, fib(x) = x, copy input
	beq $imm, $zero, $zero, L2			# jump to L2
L1:
	add $a0, $a0, $imm, 254				# calculate xx - 1
	lw $v0, $a0, $imm, 0				# calc $v0=fib(x-1)
	add $s0, $v0, $zero, 0				# $s0 = fib(x-1)
	lw $a0, $sp, $imm, 0				# restore $a0 = x
	add $a0, $a0, $imm, 255				# calculate x - 2
	lw $v0, $a0, $imm, 0				# calc fib(x-2)
	add $v0, $v0, $s0, 0				# $v0 = fib(x-2) + fib(x-1)
	lw $a0, $sp, $imm, 0				# restore $a0
	lw $ra, $sp, $imm, 1				# restore $ra
	lw $s0, $sp, $imm, 2				# restore $s0
L2:
	add $sp, $sp, $imm, 3				# pop 3 items from stack
	beq $ra, $zero, $zero, 0			# and return