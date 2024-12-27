    add $t0, $zero, $imm, 0x100   # $t0 is the base address of FIBO
	add $t1, $zero, $imm, 1   
	sw $t1, $t0, $zero ,0    # FIBO[$t0+$zero] = $t1 -> FIBO[0] = 1 
	sw $t1, $t0, $imm, 1   # FIBO[$t0+$imm] = $t1 -> FIBO[1] = 1
	add $t2, $zero , $imm, 2   # Use $t2 as "i" and set to 2
	add $t1, $zero, $imm, 110 # the maximal value
R:      beq $imm, $t1, $t2, END  # Is i <= max terations? 
        add $t2, $t2, $imm, 1   # i++
        add $a1, $t0, $t2 ,0      # $a1 = address of FIBO[i] after incrementing i
        lw $a2, $a1, $imm, -1    # $a2 = FIBO[i-1] 
        lw $a3, $a1, $imm, -2    # $a3 = FIBO[i-2]
		add $a0, $a2, $a3 ,0 # FIBO[i-1]+FIBO[i-2]
        sw $a0, $a1, $zero ,0    # FIBO[i] = FIBO[i-1]+FIBO[i-2]
        jal $v0, $imm, $zero,R # jump back to the label
END:
	halt $zero, $zero, $zero, 0			# halt