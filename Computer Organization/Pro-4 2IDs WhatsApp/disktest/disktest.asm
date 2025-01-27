	
	add $a0,$zero,$imm,1024         # $a0=Future Mem address (MEM[1024]) for 128 loaded words from sector j
	add $a1,$zero,$zero,0			# set the sector we want to read from
	add $a2,$zero,$imm,1			# set the sector we want to read from
	out $zero, $zero, $imm, 1		# disable irq1 disk interrupt
	add $t1,$zero,$imm,8			# set end sector-1
	add $s0,$zero ,$imm ,-1			#j=0 set the offset for sum
	add $s2,$zero,$imm,8			#set end sector-1
LOOP:
	beq $imm, $a1, $t1, MID		    # check if finish to read
	in $t0, $zero, $imm, 17			# checking disk status 
	bne $imm, $t0, $zero, LOOP	    # if disk is free 
	out $a1, $imm, $zero, 15		# IOreg[15] = $a1 - disk sector to read from
	out $a0, $imm, $zero, 16		# IOreg[16] = $a0 - disk buffer 
	out $a2, $imm, $zero, 14		# IOreg[14] = $a2 - diskcmd
	add $a0, $imm, $a0, 128		# $a0 +=128
	add $a1, $imm, $a1  , 1			# set the next sector we want to read from
	beq $imm, $zero, $zero, LOOP
MID:
	add $a0,$zero,$imm,1024			#reset $a0
	add $a1,$zero,$imm,127			#set $a1 to 127
	beq $imm, $zero, $zero, LOOP1
LOOP1:
	beq $imm, $s0, $a1, DISK_OUT
	add $s0, $s0, $imm, 1			#offset +=1	
	add $s1,$zero ,$zero ,0			# temp sum = 0 
	add $t0,$zero,$zero,0			#set i=0 for inner loop= sector_num
LOOP2:
	beq $imm, $t0, $s2, WRITE_MEM
	mul $t1, $t0, $imm, 128			# $t1 = 128 * sector_number
	add $t1, $t1, $s0, 0			# $t1 = offset + 128 * sector_number
	lw  $t2, $a0, $t1, 0			# load word in offset + 128 * sector_number+1024 to $t2
	add $s1, $s1, $t2, 0			# temp_sum+= word in $t2
	add $t0, $t0, $imm, 1			# sector number +=1
	beq $imm, $zero, $zero, LOOP2

WRITE_MEM:
	sw  $s1, $a0, $s0, 0			# srote the sum in memory
	beq $imm, $zero, $zero, LOOP1
DISK_OUT:
	in $t0, $zero, $imm, 17			# checking disk status 
	bne $imm, $t0, $zero, DISK_OUT  # if disk is free 
	add $t2, $zero,$imm,  2			# reset $t2=2
	out $s2, $imm, $zero, 15		# IOreg[15] = 8 - disk sector to write
	out $a0, $imm, $zero, 16		# IOreg[16] = $a0 - disk buffer 
	out $t2, $imm, $zero, 14		# IOreg[14] = $t2 - diskcmd
WAIT:
	in $t0, $zero, $imm, 17			# checking disk status 
	bne $imm, $t0, $zero, WAIT      # if disk is free
	halt $zero, $zero, $imm, 0     # Halt the program

