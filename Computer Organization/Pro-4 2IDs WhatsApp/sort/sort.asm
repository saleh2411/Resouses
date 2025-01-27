
    add $a0, $zero, $imm, 0x100     # save base address of the array into $a0
    add $a1, $zero, $imm, 16        # save the number of elements (16) into $a1

    # Initialize outer loop counter
    add $t0, $zero, $zero, 0       # $t0 = i = 0 (outer loop counter)
    sub $a1, $a1, $imm, 1          # $a1 = a1 - 1 (number of elements - 1)
    beq $imm, $zero, $zero, test1  # Jump to test1

outer_loop:
    add $t1, $zero, $zero, 0       # $t1 = j = 0 (inner loop counter)
    sub $s0, $a1, $t0, 0           # $s0 = a1 - t0 (number of remaining elements - 1)           
    beq $imm, $zero, $zero, test2  # Jump to test2

inner_loop:
    add $t2, $t1, $imm, 1          # $t2 = j + 1
    add $s2, $a0, $t1, 0           # $s2 = base address + j (address of current element)
    add $s1, $a0, $t2, 0           # $s1 = base address + j + 1 (address of next element)

    lw $a2, $s2, $zero, 0          # Load element at index j into $a2
    lw $a3, $s1, $zero, 0          # Load element at index j + 1 into $a3

    ble $imm, $a2, $a3, no_swap    # If arr[j] <= arr[j + 1], no swap needed

    # Swap elements
    sw $a3, $s2, $zero, 0          # Store arr[j + 1] at index j
    sw $a2, $s1, $zero, 0          # Store arr[j] at index j + 1

no_swap:
    add $t1, $t1, $imm, 1          # Increment inner loop counter

test2:
    blt $imm, $t1, $s0, inner_loop # Check if j < size - i - 1

    add $t0, $t0, $imm, 1          # Increment outer loop counter

test1:
    blt $imm, $t0, $a1, outer_loop # Check if i < number of elements - 1

    halt $zero, $zero, $imm, 0     # Halt the program

 # Array initialization with 16 numbers using .word
.word 0x100, 1
.word 0x101, 3
.word 0x102, 2
.word 0x103, 5
.word 0x104, 4
.word 0x105, 7
.word 0x106, 6
.word 0x107, 9
.word 0x108, 8
.word 0x109, 11
.word 0x10A, 10
.word 0x10B, 13
.word 0x10C, 12
.word 0x10D, 15
.word 0x10E, 14
.word 0x10F, 30