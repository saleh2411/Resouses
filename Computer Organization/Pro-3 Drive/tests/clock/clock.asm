.word 0x100 0x00195955 # start
.word 0X200 0x00200005 # finish
.word 0x101 0x9 # start of change
.word 0x102 0x59
.word 0x103 0x959
.word 0x104 0x5959
.word 0x105 0x95959
.word 0x106 0x235959
.word 0x201 0x10 # end of change
.word 0x202 0x100
.word 0x203 0x1000
.word 0x204 0x10000
.word 0x205 0x100000

add $t1, $zero, $imm, 0x100 # contains initial clock
lw $s0, $t1, $imm, 0 # $s0 contains the current clock
out $s0, $zero, $imm, 10 # display initial clock value
add $t2, $zero, $imm, 1 # set counter to 1
add $t1, $zero, $imm, 0X200
lw $s1, $t1, $imm,0 #$s1 contain the final clock
add $t0, $zero, $imm, 244 # set t0 to 256 
out $t0, $zero, $imm, 13 # set timerMax to 250 for first round
out $t2, $zero, $imm, 11 # enable timer
out $imm, $zero, $zero, 1 # enable irq0 to stop timer
add $t0, $zero, $imm, 6	# change t0 to 6 for irqhandler
out $imm, $t0, $zero, L2 #set irqhandler as L2
L1:
	bne $imm, $s0,$s1,L1 # don't go if we aren't done yet
	halt $zero, $zero, $zero, 0 # if we finished the loop. we stop the program
L2:
	add $t0, $zero, $imm, 256 # set t0 to 256
	out $t0, $zero, $imm, 13 # set timerMax to 256.
	lw $t3, $zero, $imm, 0x101 # hex 9
    and $t2, $t3, $s0, 0 # check if s0 has 9 at first digit now.
    beq $imm, $t3,$t2,L3 # change next digit
    add $s0, $s0, $imm, 1 # add 1 to clock
    out $s0, $zero, $imm, 10 # display current clock value
    reti $zero, $zero, $zero,0 #return from the handler
L3:
	lw $t3, $zero, $imm, 0x102 # to check for change in minutes first digit.
    and $t2, $t3, $s0, 0 # check if s0 has 59 at seconds now.
    beq $imm, $t3,$t2,L4
	lw $t2, $zero, $imm, 0x101 # load the value for hex 9
	lw $t3, $zero, $imm, 0x201 # load the values for hex 10
	sub $t3, $t3, $t2, 0 # create jump distance
    add $s0, $s0, $t3, 0 # perform jump
    out $s0, $zero, $imm, 10 # display on clock
    reti $zero, $zero, $zero,0 #return from the handler
L4:
	lw  $t3, $zero, $imm, 0x103 # the hex value 959 is contained in the 0x101'th memory cell
    and $t2, $t3, $s0, 0 # check if s0 has 959 now.
    beq $imm, $t3,$t2,L5 # change next digit
	lw $t2, $zero, $imm, 0x102 # load the value for hex 59
	lw $t3, $zero, $imm, 0x202 # load the values for hex 100
	sub $t3, $t3, $t2, 0 # create jump distance
    add $s0, $s0, $t3, 0 # perform jump
	out $s0, $zero, $imm, 10 # display on clock
    reti $zero, $zero, $zero,0 #return from the handler
L5:
	lw  $t3, $zero, $imm, 0x104 % the hex value of 5959
    and $t2, $t3, $s0, 0 # check if s0 has 5959 now.
    beq $imm, $t3,$t2,L6
	lw $t2, $zero, $imm, 0x103 # load the value for hex 959
	lw $t3, $zero, $imm, 0x203 # load the values for hex 1000
	sub $t3, $t3, $t2, 0 # create jump distance
    add $s0, $s0, $t3, 0 # perform jump
	out $s0, $zero, $imm, 10 # display on clock
    reti $zero, $zero, $zero,0 #return from the handler
L6:
	lw  $t3, $zero, $imm, 0x105 % the hex value of 95959
    and $t2, $t3, $s0, 0 # check if s0 has 95959 now.
    beq $imm, $t3, $t2, L7
	lw  $t3, $zero, $imm, 0x106 % the hex value of 235959. special midnight case
	and $t2, $t3, $s0, 0 # check if s0 has 235959 now.
    beq $imm, $t3, $t2, L8 # jump to midnight
	lw $t2, $zero, $imm, 0x104 # load the value for hex 5959
	lw $t3, $zero, $imm, 0x204 # load the values for hex 10000
	sub $t3, $t3, $t2, 0 # create jump distance
    add $s0, $s0, $t3, 0 # perform jump
	out $s0, $zero, $imm, 10 # display on clock
    reti $zero, $zero, $zero,0 #return from the handler
L7:
	lw $t2, $zero, $imm, 0x105 # load the value for hex 95959
	lw $t3, $zero, $imm, 0x205 # load the values for hex 100000
	sub $t3, $t3, $t2, 0 # create jump distance
    add $s0, $s0, $t3, 0 # perform jump
	out $s0, $zero, $imm, 10 # display on clock
    reti $zero, $zero, $zero,0 #return from the handler
L8:
	add $s0, $zero, $zero, 0 # reset s0 back to zero
	out $s0, $zero, $imm, 10
    reti $zero, $zero, $zero,0 #return from the handler