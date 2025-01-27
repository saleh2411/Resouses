	add $t2, $zero, $imm, 1 #set $t2=1 (counter)
	out $t2, $zero, $imm, 9 # light up led zero
	add $t0, $zero, $imm, 251 # set t0 to 250. special case so first lamp will be on exactly 256 cycles
	out $t0, $zero, $imm, 13 # set timerMax to 250. so first tick exactly one second after program starts
	out $t2, $zero, $imm, 11 # enable timer
	add $t3, $zero, $imm, 33 # set $t3=32, for loop	
	out $imm, $zero, $zero, 1 # enable irq0
	add $t0, $zero, $imm, 6		# point t0 to 6 for irqhandler
	out $imm, $t0, $zero, L2 #set irqhandler as L2
L1:
	blt $imm, $t2,$t3,L1 # don't go if we aren't done yet
	halt $zero, $zero, $zero, 0 # if we finished the loop. we stop the program
L2:
	in $t1, $zero, $imm, 9 # load leds registor into $t1
	sll $t1, $t1, $imm, 1 #shift to the next led(left)
	out $t1, $zero, $imm, 9 # light up the correct led
	out $zero, $zero, $imm, 3 #clear irq0 status
	addi $t2, $t2, $imm, 1 # $t2+=1
	add $t0, $zero, $imm, 256 # set t0 to 256
	out $t0, $zero, $imm, 13 # set timerMax to 256.
	reti $zero, $zero, $zero,0 #return from the led handler