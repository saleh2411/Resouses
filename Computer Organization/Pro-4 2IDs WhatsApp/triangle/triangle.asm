LOAD:
	add $s0,$zero,$imm,255	   #load the color
	out $s0,$zero,$imm,21		
	lw  $s0,$zero,$imm,256     #load A from address 0x100 to $s0
	lw  $s1,$zero,$imm,257     #load B from address 0x101 to $s1
	lw  $s2,$zero,$imm,258     #load C from address 0x102 to $s2
	

X:
	and $a0,$s0,$imm,0x000ff   # A.x = B.x = A mod 256
	and $a1,$s2,$imm,0x000ff   # C.x = A mod 256

Y:
	and $a2,$s0,$imm,0xfff00  # A.y = A/256
	srl $a2,$a2,$imm,8  
	and $a3,$s1,$imm,0xfff00  # B.y =C.y = B/256
	srl $a3,$a3,$imm,8      
	
	sub $s1, $a1, $a0,0		# save bc
	sub $s0, $a3, $a2,0		# save ab
	add $a1, $zero, $imm, -1 #start the line of the loop
LOOP:
	bgt $imm, $a1, $s0, OUT
	add $a1, $a1, $imm, 1			#line +=1	
	mul $t1, $a1, $s1,  0			#calculate y*BC for the ratio
	add $t2,$zero,$zero,0			#set i=0 for inner loop= sector_num
	add $t0, $zero, $zero, 0		#reset $t0
	beq $imm, $zero, $zero, RATIO   #calculate the ratio y*bc/ab	
LOOP2:
	bgt $imm, $t2, $t0, LOOP		#
	add $a3, $a1, $a2, 0			# calculate the line
	mul $a3, $a3, $imm, 256			# calculate the location of the line
	add $a3, $a3, $a0, 0			#add the x offset
	add $a3, $a3, $t2, 0			#add the x offset
	out $a3, $imm, $zero, 20		#set the monitor address
	add $a3, $imm, $zero, 1			#reset the monitor cmd
	out $a3, $imm, $zero, 22			
	add $t2, $t2, $imm, 1			# number +=1
	beq $imm, $zero, $zero, LOOP2
OUT:
	halt $zero, $zero, $imm, 0     # Halt the program
	
RATIO:
	blt $imm,$t1, $s0, LOOP2		# if remainder < divisor, go to LOOP
	sub $t1, $t1, $s0, 0			# remainder -= divisor
	add $t0, $t0, $imm, 1			# quotient += 1
	beq $imm,$zero, $zero, RATIO	# unconditional jump to DIV_LOOP



.word 256 0
.word 257 51200
.word 258 51400