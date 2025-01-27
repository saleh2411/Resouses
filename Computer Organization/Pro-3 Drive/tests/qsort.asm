#array to be sorted

.word 1024 246
.word 1025 352
.word 1026 27
.word 1027 33
.word 1028 87
.word 1029 2
.word 1030 543
.word 1031 4325
.word 1032 22
.word 1033 34
.word 1034 87
.word 1035 176
.word 1036 22
.word 1037 34
.word 1038 56
.word 1039 44

	add $sp, $zero, $imm, 1			# set $sp = 1
	sll $sp, $sp, $imm, 11			# set $sp = 1 << 11 = 2048. this prevents the negative address bug
	add $a0,$zero,$imm,1024			# array start index to a0. the top of the code is used as a main function
	add $a1,$zero,$imm,1039			# array end index  to a1
	jal $imm,$zero,$zero,QuickSort	# go to recoursive sorter function
	halt $zero,$zero,$zero,0		# when this is reached, stop the program
QuickSort:
	add $sp,$sp,$imm,-4				# adjust stack for 4 items
	sw $v0, $sp, $imm, 3			# save the partition index
	sw $ra, $sp, $imm, 2			# save return address
	sw $a0, $sp, $imm, 1			# save argument 0(start index)
	sw $a1, $sp, $imm, 0			# save argument 1(end index)
	bgt $imm,$a0,$a1,endIf1			# if the array ends before it starts. jump to end if 1(resume control to caller)
	jal $imm,$zero,$zero,Partition	# go to partition with the calling indexes of a0 and a1. it will set the value of v0 for later use and does not need any arguments as it doesn't change the registers above it
	add $a1,$v0,$imm,-1				# set end index for first half
	jal $imm,$zero,$zero,QuickSort	# run recoursive quicksort on first half
	lw $a1,$sp,$imm,0				# return original end to prepare to run on second half
	add $a0,$v0,$imm,1				# put start of second half in a0
	jal $imm,$zero,$zero,QuickSort	# run recoursive quicksort on second half
endIf1:
	lw $v0, $sp, $imm, 3			# reload original partition index
	lw $ra, $sp, $imm, 2			# reload original return address
	lw $a0, $sp, $imm, 1			# reload original start index
	lw $a1, $sp, $imm, 0			# reload original end index
	add $sp,$sp,$imm,4				# free memory from stack
	beq $ra, $zero, $zero, 0		# return to caller
Partition:
	add $sp,$sp,$imm,-1				# adjust stack for 1 items
	sw $ra, $sp, $imm, 0			# save return address
	lw $s0,$a1,$zero,0				# load pivot element to s0. it's the top of the array part sent
	sub $v0,$a0,$imm,1				# i = (low - 1) stored in v0. will be used for the swapping as a new index for elements smaller than pivot then returned to quicksort function to put borders for recoursive calls
	add $t0,$a0,$zero,0				# use t0 as a for loop index
Loop:
	bge $imm, $t0, $a1, LoopEnd		# check if t0 wen too high otherwise exit loop
	lw $t2, $t0, $zero,0			# load $t0'th array element to $t2
	bge $imm,$t2,$s0,endIf2			# an if to check if t2 < s0(pivot) otherwise go to endif2
	add $v0,$v0,$imm,1				# move i(v0) index by 1
	lw $t3, $v0, $zero,0			# load $v0'th or i'th element to t3
	sw $t3,$t0,$zero,0				# swap array indexes by sending the registers they were saved in to the other address
	sw $t2,$v0,$zero,0
endIf2:
	add $t0, $t0, $imm, 1			# Advance loop index
	beq $imm, $zero, $zero, Loop	# jump to Loop
LoopEnd:
	add $v0,$v0,$imm,1				# move i index by 1 one last time
	lw $t3, $v0, $zero,0			# load $v0'th or i'th element to t3
	sw $s0, $v0, $zero,0			# do the final swap to put pivot in i
	sw $t3, $a1, $zero,0			# and element at i+1 in pivot
	lw $ra, $sp, $imm, 0			# restore $ra
	add $sp,$sp,$imm,1				# free memory from stack
	beq $ra, $zero, $zero, 0		# return to caller