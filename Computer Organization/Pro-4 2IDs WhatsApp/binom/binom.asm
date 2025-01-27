add $t2, $zero, $imm, 1				
sll $sp, $t2, $imm, 11

lw $a0, $zero, $imm,256  # Load n from memory address 0x100 to $a0 
lw $a1, $zero, $imm,257  # Load k from memory address 0x101 to $a0

jal $ra, $imm, $zero, binom
sw $v0, $zero, $imm,258  # Store the result at memory address 0x102
halt $zero, $zero,$imm,0 


binom:

	add $sp, $sp,$imm, -4				 # make room on stack for 3 registers
	sw $s0,$sp,$imm 3 
	sw $ra,$sp,$imm,2					 # Save return address on stack
	sw $a0,$sp,$imm,1                    # Save $a0 (n) on stack
	sw $a1,$sp,$imm,0                    # Save $a1 (k) on stack

	beq $imm,$a1, $zero, BASE            # if (k == 0) jump to BASE
	beq $imm,$a0, $a1, BASE              # if (n == k) jump to BASE

	sub $a0, $a0,$imm, 1                 # n = n - 1 
	sub $a1, $a1,$imm,1                  # k = k - 1 
	jal $ra,$imm,$zero,binom			 # call binom(n-1, k-1) and save return address in $ra
	add $s0, $v0, $zero,0                # Save result of binom(n-1, k-1) in $s0 
	lw $a0,$sp,$imm,1                    # Restore original n from stack
	lw $a1,$sp,$imm,0                    # Restore original k from stack
	sub $a0, $a0,$imm, 1				 # n = n - 1 
	jal $ra,$imm,$zero,binom			 # call binom(n-1, k) and save return address in $ra
	add $v0, $v0, $s0,0                  # result= added results of both recursive calls
	beq $imm, $zero, $zero, END		   	 #jump to END

BASE:
	add $v0,$zero,$imm,1				 #returned result set to 1

	END:
	lw $a1,$sp,$imm,0					 # Restore k from stack
	lw $a0,$sp,$imm,1					 # Restore n from stack
	lw $ra,$sp,$imm,2					 # Restore return address from stack 
	lw $s0,$sp,$imm 3
	add $sp, $sp, $imm, 4				 # Readjust stack pointer
	beq $ra,$zero,$zero,0			     # Return to caller
	.word 0x100, 4
	.word 0x101, 3