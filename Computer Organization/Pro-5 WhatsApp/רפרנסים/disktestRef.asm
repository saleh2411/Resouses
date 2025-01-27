	add $t0, $zero, $imm, 1024			# $t0 will be the iterator of sector0 which will be written to the buffer that start at the address 1024 in the main memory
	add $t1, $zero, $imm, 1152			# $t1 will be the iterator of sector1 which will be written to the buffer that start at the address 1152 in the main memory
	add $t2, $zero, $imm, 1280			# $t2 will be the iterator of sector2 which will be written to the buffer that start at the address 1280 in the main memory
	add $a0, $zero, $imm, 1408			# $a0 will be the iterator of sector3 which will be written to the buffer that start at the address 1408 in the main memory
	add $a1, $zero, $imm, 1536			# $a1 will be the iterator of sector4 which will be written to the buffer that start at the address 1536 in the main memory
	add $a2, $zero, $imm, 1664			# $a2 will be the iterator of sector5 which will be written to the buffer that start at the address 1664 in the main memory
	add $a3, $zero, $imm, 1792			# $a3 will be the iterator of sector6 which will be written to the buffer that start at the address 1792 in the main memory
	add $gp, $zero, $imm, 1920			# $gp will be the iterator of sector7 which will be written to the buffer that start at the address 1920 in the main memory
	
	# Reading sectors 0 - 7 from the hard disk to the main memory

	add $s0, $zero, $imm, 1				# $s0 = 1 
	out $s0, $zero, $imm, 1			  	# enable irq1
	add $s2, $zero, $imm, irq_handler 	# $s2 = address of irq_handler
	out $s2, $zero, $imm, 6				# set irqhandler as irq_handler 
	
	add $s0, $zero, $imm, 0 
	out $s0, $imm, $zero, 15			#setting disksector to 0
	out $t0, $imm, $zero, 16			#setting diskbuffer to 1024
	add $s1, $imm, $zero, 1				
	out $s1, $zero, $imm, 14			#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish 
	
	add $s0, $zero, $imm, 1 
	out $s0, $imm, $zero, 15			#setting disksector to 1
	out $t1, $imm, $zero, 16			#setting diskbuffer to 1152				
	out $s1, $zero, $imm, 14			#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish
	
	add $s0, $zero, $imm, 2 
	out $s0, $imm, $zero, 15			#setting disksector to 2
	out $t2, $imm, $zero, 16			#setting diskbuffer to 1280				
	out $s1, $zero, $imm, 14			#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish
	
	add $s0, $zero, $imm, 3 
	out $s0, $imm, $zero, 15			#setting disksector to 3
	out $a0, $imm, $zero, 16			#setting diskbuffer to 1408				
	out $s1, $zero, $imm, 14				#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish
	
	add $s0, $zero, $imm, 4 
	out $s0, $imm, $zero, 15			#setting disksector to 4
	out $a1, $imm, $zero, 16			#setting diskbuffer to 1536				
	out $s1, $zero, $imm, 14				#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish
	
	add $s0, $zero, $imm, 5 
	out $s0, $imm, $zero, 15			#setting disksector to 5
	out $a2, $imm, $zero, 16			#setting diskbuffer to 1664				
	out $s1, $zero, $imm, 14				#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish

	add $s0, $zero, $imm, 6 
	out $s0, $imm, $zero, 15			#setting disksector to 6
	out $a3, $imm, $zero, 16			#setting diskbuffer to 1792				
	out $s1, $zero, $imm, 14				#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish
	
	add $s0, $zero, $imm, 7 
	out $s0, $imm, $zero, 15			#setting disksector to 7
	out $gp, $imm, $zero, 16			#setting diskbuffer to 1920				
	out $s1, $zero, $imm, 14				#setting diskcmd to 1 (read mode) 
	jal $ra, $imm, $zero, wait_read	    #waiting to read operation to finish

	add $s0, $zero, $imm, 2048			#s0 will be the iterator of the result buffer that will be written to sector8, it will be kept unt the beffer that starts at the address 2048 of the main memory
	add $s1, $zero, $zero, 0			# i = 0
	add $v0, $zero, $imm, 128
	add $sp, $zero, $zero, 0
	
	# Summing up all the values

add_loop:
	lw $s2, $t0, $s1, 0				# $s2=MEM[addr of sector0 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $t1, $s1, 0				# $s2=MEM[addr of sector1 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $t2, $s1, 0				# $s2=MEM[addr of sector2 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $a0, $s1, 0				# $s2=MEM[addr of sector3 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $a1, $s1, 0				# $s2=MEM[addr of sector4 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $a2, $s1, 0				# $s2=MEM[addr of sector5 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $a3, $s1, 0				# $s2=MEM[addr of sector6 + i]
	add $sp, $sp, $s2, 0
	lw $s2, $gp, $s1, 0				# $s2=MEM[addr of sector7 + i]
	add $sp, $sp, $s2, 0
	sw $sp, $s0, $s1, 0				# $s0 = sector0[i] + sector1[i] + sector2[i] + sector3[i] + sector4[i] + sector5[i] + sector6[i] + sector7[i]
	add $s1, $s1, $imm, 1			# i++ 
	add $sp, $zero, $zero, 0
	blt $imm, $s1, $v0, add_loop	# if i < 128 return to loop
	
	# Writing the results from the main memory to sector8
	
	add $s1, $zero, $imm, 8
	add $s2, $zero, $imm, 2
	out $s1, $imm, $zero, 15			#setting disksector to 8
	out $s0, $imm, $zero, 16			#setting diskbuffer to 2048				
	out $s2, $zero, $imm, 14			#setting diskcmd to 2 (write mode) 
	jal $ra, $imm, $zero, wait_write	#waiting to write operation to finish	
	halt $zero,$zero,$zero, 0		   	#end of program
	
wait_read:  
	add $s2,$zero,$imm,1
	in $sp,$imm,$zero,14    			# take diskcmd
	beq $imm, $sp, $s2, wait_read   	# if(diskcmd==1) j to wait_read, when coming back from interrupt diskcmd should be zero
	jal $s2 , $ra , $zero , 0
	
wait_write:  
	add $s2,$zero,$imm,2
	in $sp,$imm,$zero,14    			# take diskcmd
	beq $imm, $sp, $s2, wait_write   	# if(diskcmd==2) j to wait_write, when coming back from interrupt diskcmd should be zero
	jal $s2 , $ra , $zero , 0


irq_handler: # interrupt handler 
	out $zero, $zero, $imm, 4			# clear irq1 status
	reti $zero, $zero, $zero, 0			# return from interrupt








	#############################
	
	add $a0, $zero, $imm, 896				#a0 starting buffer
	add $a1, $zero, $imm, 8					#stoping parameter
	add $v0, $zero, $imm, 128				
		
	add $s0, $zero, $imm, 0					#sector counter

L1:
	add $a0, $a0, $imm, 128					#looping every 128
	out $s0, $zero, $imm, 15				#setting disksector to s0
	out $a0, $zero, $imm, 16				#setting diskbuffer to 1024
	add $t1, $zero, $imm, 1			
	out $t1, $zero, $imm, 14				#setting READ
	jal $ra, $imm, $zero, PAUSE				#waiting for disk to finish
	add $s0, $s0, $imm, 1					#incr sector count
	blt $imm, $s0, $a1, L1					#loop if didnt do 7 sectores

L2:
	add $t0, $zero, $imm, 0					#sttarting counter
	add $a0, $zero, $imm, 896				#a0 starting buffer
	add $s0, $zero, $imm, 2048				#saving addres
	add $s2, $zero, $imm, 0					#sum
	add $sp, $zero, $zero, 0				#

L_IN:

	add $a0, $a0, $imm, 128					#looping every 128
	lw $s2, $a0, $t0						#$s2=MEM[sector $a0 +$t0]
	add $sp, $sp, $s2, 0					#su
	blt $imm, $a0, $s0, L_IN				#load all sectors

	sw $sp, $s0, $t0, 0						#saving sum to memory
	add $t0, $t0, $imm, 1					#incr counter
	blt $imm, $t0, $v0, L2					#if t0<128 start sum loop(L2)



	#writing back the results to the disk

	add $t2, $zero, $imm, 2				
	out $a1, $zero, $imm, 15				#sector 8
	out $s0, $zero, $imm, 16				#diskbuffer 2048
	out $t2, $zero, $imm, 14				#setting WRITE
	jal $ra, $imm, $zero, PAUSE				#waiting for disk to finish
	halt $zero, $zero, $zero, 0				#end


PAUSE:
	in $t0, $imm, $zero, 17					#t0=diskstatus
	bne $imm, $t0, $zero, PAUSE				#loop antil ready, (1024 max)
	beq $ra, $zero, $zero					#if ready, continue














		###the runing timeis to long we will do "Hard Coding"
	#init sector memory pointers
	add $s0, $zero, $imm, 1024			# $s0 sector 0 index
	add $s1, $zero, $imm, 1152			# $s1  sector 1 index
	add $s2, $zero, $imm, 1280			# $s2  sector 2 index
	add $a0, $zero, $imm, 1408			# $a0 sector 3 index
	add $a1, $zero, $imm, 1536			# $a1 sector 4 index
	add $a2, $zero, $imm, 1664			# $a2  sector 5 index
	add $a3, $zero, $imm, 1792			# $a3 sector 6 index
	add $gp, $zero, $imm, 1920			# $gp sector 7 index

	## read each sector

	add $t0, $zero, $imm, 1				#t0 = 1 
	out $t0, $zero, $imm, 1			  	#enable irq1
	add $t2, $zero, $imm, IRQ		 	#t2 = address of IRQ
	out $t2, $zero, $imm, 6				#irqhandler = IRQ 

	add $t0, $zero, $imm, 0 
	out $t0, $imm, $zero, 15			#disksector=0
	out $s0, $imm, $zero, 16			#diskbuffer=1024
	add $t1, $imm, $zero, 1				
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish
	
	add $t0, $zero, $imm, 1 
	out $t0, $imm, $zero, 15			#disksector=1
	out $s1, $imm, $zero, 16			#diskbuffer=1152			
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 2 
	out $t0, $imm, $zero, 15			#disksector=2
	out $s2, $imm, $zero, 16			#diskbuffer=1280			
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 3 
	out $t0, $imm, $zero, 15			#disksector=3
	out $a0, $imm, $zero, 16			#diskbuffer=1408			
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 4 
	out $t0, $imm, $zero, 15			#disksector=4
	out $a1, $imm, $zero, 16			#diskbuffer=1536		
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 5 
	out $t0, $imm, $zero, 15			#disksector=5
	out $a2, $imm, $zero, 16			#diskbuffer=1664		
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 6 
	out $t0, $imm, $zero, 15			#disksector=6
	out $a3, $imm, $zero, 16			#diskbuffer=1792
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish

	add $t0, $zero, $imm, 7 
	out $t0, $imm, $zero, 15			#disksector=7
	out $gp, $imm, $zero, 16			#diskbuffer=1920
	add $t1, $imm, $zero, 1				
	out $t1, $zero, $imm, 14			#diskcmd READ 
	jal $ra, $imm, $zero, PAUSE			#waiting to read operation to finish


	## init var for summing vals

	add $t0, $zero, $imm, 2048			#sector 8 buffer
	add $t1, $zero, $zero, 0			#counter
	add $t2, $zero, $imm, 128			#stop val
	add $v0, $zero, $zero, 0			#sum
L:

	lw $sp, $s0, $t1, 0					#load val sector 0 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $s1, $t1, 0					#load val sector 1 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $s2, $t1, 0					#load val sector 2 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $a0, $t1, 0					#load val sector 3 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $a1, $t1, 0					#load val sector 4 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $a2, $t1, 0					#load val sector 5 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $a3, $t1, 0					#load val sector 6 index t1 
	add $v0, $v0, $sp, 0				#add to sum
	lw $sp, $gp, $t1, 0					#load val sector 7 index t1 
	add $v0, $v0, $sp, 0				#add to sum

	sw $v0, $t0, $t1, 0					# saving to mem the sum in the corect index
	add $t1, $t1, $imm, 1				#incr index
	add $v0, $zero, $zero, 0			#reset sum for nex iteration
	blt $imm, $t1, $t2, L				#loop if not 128

	## write to disk
		

	add $s0, $zero, $imm, 8 
	out $s0, $imm, $zero, 15				#disksector=8
	out $t0, $imm, $zero, 16				#diskbuffer=2048
	add $t1, $imm, $zero, 2				
	out $t1, $zero, $imm, 14				#diskcmd WRITE
	jal $ra, $imm, $zero, PAUSE				#waiting to write operation to finish
	halt $zero, $zero, $zero, 0				#exit

	
PAUSE:
	in $t0, $imm, $zero, 17					#t0=diskstatus
	bne $imm, $t0, $zero, PAUSE				#loop antil ready, (1024 max)
	beq $ra, $zero, $zero					#if ready, continue


	IRQ:  
	out $zero, $imm, $zero, 4			#clear irq
	reti $zero, $zero, $zero, 0			#return