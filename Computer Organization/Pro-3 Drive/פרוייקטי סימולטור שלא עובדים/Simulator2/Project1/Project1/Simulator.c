#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

// a line is always smaller than 10 chars
#define MAX_LINE 10
// 65536 is long enough for most of the simulator's input memories.
// we didn't make it support infinite code because we didn't have tile to implement it
#define MAX_FILE 65536
// disk timer max - 1024 "disk is busy" clock cycles
#define diskTimerMax = 1024;

// global boolean to determine if in irq event
int isirqEvent = 0;
// global integer - length of irq2 trigger time array
int irq2count;

// this two global integers are related to disk operation
// disk timer - counts clock cycles with busy disk
int diskTimer = 0;

// gets irq2
int* parseirq2(FILE* irq2in) {
	// leave is irq2 file didn't open
	if (irq2in == NULL) {
		exit(1);
	}
	// the current row
	char current[MAX_LINE];
	// count the number of ints needed for the required memory
	irq2count = 0;
	while (!feof(irq2in)) {
		fgets(current, MAX_LINE, irq2in);
		irq2count++;
	}
	// return to the start for the actual reading
	fseek(irq2in, 0, SEEK_SET);
	// initialize array as dynamic memory. which will be released when the simulator halts
	int* irq2 = (int*)malloc(sizeof(int) * irq2count);
	// create index
	int index = 0;
	// fseek resets end of file
	while (!feof(irq2in)) {
		fgets(current, MAX_LINE, irq2in);
		// add to array
		irq2[index] = atoi(current);
		index = index + 1;
	}
	return irq2;
}

// basically updates the timer and trips IRQ0. a pointer to the input/output register array
// is the only input
void updateTimer(int* ioRege) {
	// reset irqstatus0 if timer did not tick
	ioRege[3] = 0;
	// check timer enable before updating
	if (ioRege[11] != 0 && ioRege[12] < ioRege[13]) {
		ioRege[12]++;
		// now we check if the timer event happend
		if (ioRege[12] == ioRege[13]) {
			ioRege[3] = 1;
			ioRege[12] = 0;
		}
	}
}

// function prints to the trace file. it gets the pc. memory line
// and register array and prints a row on the register values
Print_To_Trace(FILE* trace, int pc, char* line, int Reg_Array[]) {
	// hexval - hexadcimal value of current row
	char hexval[9], instruction[8], * inst = instruction;
	// write hexadecimal PC
	sprintf(hexval, "%08X", pc);
	// write hexadecimal pc in trace
	fputs(hexval, trace);
	// write a space to the file
	putc(' ', trace);
	// get instruction directly as was written in memin
	inst = strtok(line, "\n");
	// and write to file
	fputs(inst, trace);
	// space advance on trace file
	putc(' ', trace);
	// now write each register's value into the trace file
	for (int i = 0; i <= 15; i++) {
		sprintf(hexval, "%08x", Reg_Array[i]);
		fputs(hexval, trace);
		if (i != 15) {
			putc(' ', trace);
		}
	}
	// finish current row on trace file
	putc('\n', trace);
}

// updates leds and display files. gets clock cycle, updated HW register, the HW register array and the leds and display files
void updateLD(int cycle, int regNum, int* ioRege, FILE* leds, FILE* display) {
	char toWrite[100] = "";
	// start with cycle
	_itoa(cycle, toWrite, 10);
	// cibvert register value to hexa
	char regVal[MAX_LINE];
	sprintf(regVal, "%08x", ioRege[regNum]);
	// add the spacebar
	strcat(toWrite, " ");
	strcat(toWrite, regVal);
	// add the new line
	strcat(toWrite, "\n");
	// check if to use leds or display file and use the pointer to
	// point the correct file
	FILE* fileToUse;
	if (regNum == 9) {
		fileToUse = leds;
	}
	else {
		fileToUse = display;
	}
	// write to file
	fprintf(fileToUse, toWrite);
}

// updates the hwRegTrace file
// gets current cycle, action(0 write 1 read), register being used and HW register array pointer, as well as a pointer to the file
void updatehwRegTrace(int cycle, int action, int reg, int* ioRege, FILE* hwregTrace) {
	// big string for current line, slowly strings will be added to it then it will be printed
	// to the file after the null wrap
	char toWrite[2000] = "";
	// start with cycle
	_itoa(cycle, toWrite, 10);
	// 1 - read, 0 - write
	if (action == 1) {
		strcat(toWrite, " READ");
	}
	else {
		strcat(toWrite, " WRITE");
	}
	// the names of the IO registers
	char names[18][50] = { " irq0enable "," irq1enable "," irq2enable "," irq0status "," irq1status "," irq2status "," irqhandler "," irqreturn "," clks "," leds ",
		" display "," timerEnable "," timerCurrent "," timerMax "," diskcmd "," disksector "," diskbuffer "," diskstatus " };
	// write the register's name
	strcat(toWrite, names[reg]);
	// now convert the register value. the bitmask is an extention to 8 bits
	// string will hold register value as string
	char regVal[MAX_LINE];
	sprintf(regVal, "%08x", ioRege[reg]);
	strcat(toWrite, regVal);
	// add next line
	strcat(toWrite, "\n");
	// finally, write to file
	fprintf(hwregTrace, toWrite);
}

// performs disk read and write, gets ioRege - pointer to IO registers, out - pointer to memory and disk - pointer to disk
void diskOperation(int* ioRege, char out[][9], char disk[][9]) {
	// only do if disk isn't busy
	if (ioRege[17] != 0)
		return;
	// translate sector number to line number in file/array
	int sectorStart = ioRege[15] * 128;
	// Reading loop
	if (ioRege[14] == 1) {
		// 128 iterations because the sector contains 128 rows
		for (int i = 0; i < 128; i++) {
			// copy disk sector into memory buffer
			strcpy(out[i + ioRege[16]], disk[i + sectorStart]);
		}
	}
	else {
		for (int i = 0; i < 128; i++) {
			// copy memory buffer into disk sector
			strcpy(disk[i + sectorStart], out[i + ioRege[16]]);
		}
	}
	// disk is now busy. so reset the timer
	ioRege[17] = 1;
	diskTimer = 0;
}

// each opcode has it's own function to shorten the length of functions

// each opcode function takes integers of the instruction parameters, the current pc and register array(rege). and returns the pc after.
// arithmetic functions take no more than that

// this executes "add" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int add(int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 0) { //ADD instruction
		rege[rd] = rege[rs] + rege[rt];
		pc += 1;
	}
	return pc;
}

// this executes "sub" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sub(int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 1) { //SUB instruction
		rege[rd] = rege[rs] - rege[rt];
		pc += 1;
	}
	return pc;
}

// this executes "and" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int and (int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 2) {//and instruction
		rege[rd] = (rege[rs] & rege[rt]);
		pc += 1;
	}
	return pc;
}

// this executes "or" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int or (int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 3) { //or instruction
		rege[rd] = (rege[rs] | rege[rt]);
		pc += 1;
	}
	return pc;
}

// this executes "sll" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sll(int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 4) { //sll instruction
		rege[rd] = (rege[rs] << rege[rt]);
		pc += 1;
	}
	return pc;
}

// this executes "sra" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sra(int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 5) { //sra instruction. uses the normal >> because
	// all values are signed
		rege[rd] = (rege[rs] >> rege[rt]);
		pc += 1;
	}
	return pc;
}

// this executes "srl" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int srl(int opcode, int rd, int rs, int rt, long pc, int* rege) {
	if (opcode == 6) { //srl instruction, uses a special code
		int size = sizeof(int);
		// shift
		// deal with the zero case
		if (rege[rs] == 0)
			rege[rd] = rege[rs];
		else /// else use modified >> operator. it uses a bitmask that only lets the original bits pass to zero any newly created bit
			rege[rd] = (rege[rs] >> rege[rt]) & ~(((rege[rs] >> (size << 3) - 1) << (size << 3) - 1)) >> (rege[rt] - 1);
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	return pc;
}

// the functions for branch and jump instructions take "file" the pointer to memin they need to move to the correct location
// so the line at pc will be read

// this executes "beq" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int beq(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 7) {//beq instruction
	// do in case of equality
		if (rege[rs] == rege[rt]) {
			// jump pc to immediate value
			pc = rege[rd];
			// use pc_help to move the pointer
			pc_help = pc;
			// move file pointer back to the start
			fseek(file, 0, SEEK_SET);
			// and from there to the correct pc value
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}// otherwise continue to the next row
		else
			pc += 1;
	}
	return pc;
}
// this executes "bne" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bne(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 8) {
		if (rege[rs] != rege[rt]) {
			pc = rege[rd];
			pc_help = pc;
			fseek(file, 0, SEEK_SET);
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "blt" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int blt(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 9) { //blt instruction
		if (rege[rs] < rege[rt]) {
			pc = rege[rd];
			pc_help = pc;
			fseek(file, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "bgt" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bgt(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 10) { //bgt instruction
		if (rege[rs] > rege[rt]) {
			pc = rege[rd];
			pc_help = pc;
			fseek(file, 0, SEEK_SET);
			while (pc_help > 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "ble" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int ble(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 11) { //ble instruction
		if (rege[rs] <= rege[rt]) {
			pc = rege[rd];
			pc_help = pc;
			fseek(file, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "bge" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bge(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 12) { // bge instruction
		if (rege[rs] >= rege[rt]) {
			pc = rege[rd];
			pc_help = pc;
			fseek(file, 0, SEEK_SET);
			while (pc_help >= 0) {
				fgets(line2, MAX_LINE, file);
				pc_help -= 1;
			}
		}
		else
			pc += 1;
	}
	return pc;
}

// this executes "jal" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int jal(int opcode, int rd, int rs, int rt, long pc, int* rege, FILE* file) {
	// pc_help is used to move the file pointer to the the pc'd row
	int pc_help;
	// this string and the pointer to it are used to move the file pointer by reading lines
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 13) {	//jal instruction
	// put original pc(+1) in return address register
		rege[15] = pc + 1;
		//  move pc similar to the branch instructions
		pc = rege[rd];
		pc_help = pc;
		fseek(file, 0, SEEK_SET);
		while (pc_help > 0) {
			fgets(line2, MAX_LINE, file);
			pc_help -= 1;
		}
	}
	return pc;
}

// this executes "lw" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// output - current memory as an array
// returns pc after execution
int lw(int opcode, int rd, int rs, int rt, long pc, int* rege, char output[][9]) {
	// stores the memory world we load as an int
	int MEM;
	// and as a string
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 14) {	//lw instruction
	// address of word
		int lines = rege[rs] + rege[rt];
		// get the hexadecimal word
		strcpy(line2, output[lines]);
		// convert to an integer
		MEM = (int)strtol(line2, NULL, 16);
		// put in register
		rege[rd] = MEM;
		// zero lock(again)
		if (rd == 0)
			rege[rd] = 0;
		pc += 1;
	}
	return pc;
}

// this executes "sw" instructions
// opcode, rd, rs and rt - instruction variables
// pc - current pc
// rege - register array
// output - current memory as an array
// returns pc after execution
int sw(int opcode, int rd, int rs, int rt, long pc, int* rege, char output[][9]) {
	// and as a string
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (opcode == 15) { //sw instruction
		char hexval[9];
		// get line index ffor the store
		int lines = rege[rs] + rege[rt];
		// convert rd value to hexacdecimal
		sprintf(hexval, "%08X", rege[rd]);
		// copy each char in the word to a diffrent bit for the output
		// which will be later written to memout
		output[lines][0] = hexval[0];
		output[lines][1] = hexval[1];
		output[lines][2] = hexval[2];
		output[lines][3] = hexval[3];
		output[lines][4] = hexval[4];
		output[lines][5] = hexval[5];
		output[lines][6] = hexval[6];
		output[lines][7] = hexval[7];
		output[lines][8] = hexval[8];
		pc += 1;
	}
	return pc;
}

// instructions dealing with input/output start here:

// reti instruction.
// this executes "lw" instructions
// opcode - instruction opcode
// pc - current pc
// ioRege - HW register array
// returns pc after execution
//but gets an array pointer to the hardware registers(ioRege)
int reti(int opcode, int* ioRege, long pc) {
	if (opcode == 16) {
		// reti command, very simple
		pc = ioRege[7];
		isirqEvent = 0;
	}
	return pc;
}

// in operation. gets
// opcode, rd, rs and rt - like other functions
// rege - pointer to register array
// ioRege - pointer to HW register array
// cycle - current clock cycle
// hwregTrace - pointer to hw register array
// pc - current pc
// returns pc after execution
int in(int opcode, int rd, int rs, int rt, int* rege, int* ioRege, int cycle, FILE* hwregTrace, long pc) {
	if (opcode == 17) {
		// in command. as specified in instructions
		rege[rd] = ioRege[rege[rs] + rege[rt]];
		updatehwRegTrace(cycle, 1, rege[rs] + rege[rt], ioRege, hwregTrace);
		pc = pc + 1;
	}
	return pc;
}

// out operation. gets
// opcode, rd, rs and rt - like other functions
// rege - pointer to register array
// ioRege - pointer to HW register array
// cycle - current clock cycle
// hwregTrace - pointer to hw register array
// pc - current pc
// output - the current memory as a big array
// disk - the current disk as a big array
// leds - leds file pointer
// display - display file pointer
// returns pc after execution
int out(int opcode, int rd, int rs, int rt, int* rege, int* ioRege, int cycle, FILE* hwregTrace, long pc, char output[][9], char disk[][9], FILE* leds, FILE* display) {
	if (opcode == 18) {
		// update ioRege and hwregtrace file like in in
		ioRege[rege[rs] + rege[rt]] = rege[rd];
		updatehwRegTrace(cycle, 0, rege[rs] + rege[rt], ioRege, hwregTrace);
		// update leds and display files
		if ((rege[rs] + rege[rt] == 9) || (rege[rs] + rege[rt] == 10)) {
			updateLD(cycle, rege[rs] + rege[rt], ioRege, leds, display);
		}
		// check if disk command register was updated. if so execute the operation
		if ((rege[rs] + rege[rt] == 14)) {
			diskOperation(ioRege, output, disk);
		}
		pc = pc + 1;
	}
	return pc;
}

// performs command as opcode specifies. and returns the pc after it's execution
// opcode, rd, rs, rt, immediate_char are the command variables
// rege - register array pointer, ioRege - HW register array pointer
// pc - the current pc
// file - memory input file
// output - memory output array
// hwregtrace - hardware register trace file pointer
// cycle - current clock cycle
// leds and display - file pointers to them
// disk - disk content array
int Opcode_Operation(int opcode, int rd, int rs, int rt, char immediate_char[], int* rege, int* ioRege, long pc, FILE* file, char output[][9], FILE* hwregTrace, int cycle, FILE* leds, FILE* display, char disk[][9]) {
	// while MEM is used to convert hexadecimal string to integer in load
	// follow each opcode and do what it was meant to do. returning the "after" pc
	pc = add(opcode, rd, rs, rt, pc, rege);
	pc = sub(opcode, rd, rs, rt, pc, rege);
	pc = and (opcode, rd, rs, rt, pc, rege);
	pc = or (opcode, rd, rs, rt, pc, rege);
	pc = sll(opcode, rd, rs, rt, pc, rege);
	pc = sra(opcode, rd, rs, rt, pc, rege);
	pc = srl(opcode, rd, rs, rt, pc, rege);
	pc = beq(opcode, rd, rs, rt, pc, rege, file);
	pc = bne(opcode, rd, rs, rt, pc, rege, file);
	pc = blt(opcode, rd, rs, rt, pc, rege, file);
	pc = bgt(opcode, rd, rs, rt, pc, rege, file);
	pc = ble(opcode, rd, rs, rt, pc, rege, file);
	pc = bge(opcode, rd, rs, rt, pc, rege, file);
	pc = jal(opcode, rd, rs, rt, pc, rege, file);
	pc = lw(opcode, rd, rs, rt, pc, rege, output);
	pc = sw(opcode, rd, rs, rt, pc, rege, output);
	pc = reti(opcode, ioRege, pc);
	pc = in(opcode, rd, rs, rt, rege, ioRege, cycle, hwregTrace, pc);
	pc = out(opcode, rd, rs, rt, rege, ioRege, cycle, hwregTrace, pc, output, disk, leds, display);

	// prevent changing zero
	if (rd == 0)
		rege[rd] = 0;

	return pc;
}

// updates the "busy disk" timer and resets it on 1024 clock cycles
// gets ioRege - HW register array
void updateDiskTimer(int* ioRege) {
	// set irq1 status to zero if we can't do anything
	ioRege[4] = 0;
	if (ioRege[17]) {
		diskTimer++;
		// reached 1024 cycles
		if (diskTimer >= 1024) {
			// reset disk command
			ioRege[14] = 0;
			// and disk status
			ioRege[17] = 0;
			// and trips irq1status
			ioRege[4] = 1;
		}
	}
}

// prints all file outputs. gets pointers to all output files. the memout content as an array. the clock cycle count, the register values at the end and the diskout content as an array
Print_To_Files(FILE* mem_out, FILE* regout, FILE* trace, FILE* cycles, char output[][9], int count, int Reg_Array[], char disk[][9], FILE* diskout) {
	// i is where the memory file ends. starts at 65532(very big and moves to end of file
	// and j is the index of the current row
	int i = MAX_FILE - 2, j = 0;
	// 
	char regoutchar[9];
	// move memout end pointer to correct place
	while (strcmp(output[i], "00000000") == 0)
		i -= 1;
	// write memout. with a \n between two subsequent 8 char rows
	while (j <= i) {
		fputs(output[j], mem_out);
		putc('\n', mem_out);
		j += 1;
	}
	// write disk out same way as memout
	while (strcmp(disk[i], "00000000") == 0)
		i -= 1;
	while (j <= i) {
		fputs(disk[j], diskout);
		putc('\n', diskout);
		j += 1;
	}
	// write the amount of cycles the program ran
	fprintf(cycles, "%d", count);

	// finally. write the register values to regout. not including zero and imm
	for (i = 2; i <= 15; i++) {
		sprintf(regoutchar, "%08X", Reg_Array[i]);
		fputs(regoutchar, regout);
		putc('\n', regout);
	}
}
// single char to int converter for register indexes
int StrTol(char c) {
	if (c >= '0' && c <= '9')
		return (int)c - 48;
	if (c == 'a' || c == 'A')
		return 10;
	if (c == 'b' || c == 'B')
		return 11;
	if (c == 'c' || c == 'C')
		return 12;
	if (c == 'd' || c == 'D')
		return 13;
	if (c == 'e' || c == 'E')
		return 14;
	if (c == 'f' || c == 'F')
		return 15;
	return 0;
}

// checks for wheter an irq event has happend. moves pc and trips irqevent "boolean"
// and a pointer to memin
int checkirq(int* ioRege, int pc, FILE* file) {
	// in case of irq event return current pc
	if (isirqEvent) {
		return pc;
	}
	// the irq boolean as specified
	if ((ioRege[0] && ioRege[3]) || (ioRege[1] && ioRege[4]) || (ioRege[2] && ioRege[5])) {
		// return address
		ioRege[7] = pc;
		isirqEvent = 1;
		// put memin pointer at correct place
		int pc_help = ioRege[6];
		fseek(file, 0, SEEK_SET);
		char line2[MAX_LINE];
		while (pc_help > 0) {
			fgets(line2, MAX_LINE, file);
			pc_help -= 1;
		}
		return ioRege[6];
	}
	// default case - keep current pc
	return pc;
}
// trips irq2 status at the currect clock cycles
// gets pointer to HW registers, clock cycle count and
// the irq2 trigger time array
void checkirq2(int* ioRege, int count, int* irq2Times) {
	// reset irq2
	ioRege[5] = 0;
	// loop trough the irq2 to see if event happens
	for (int i = 0; i < irq2count; i++) {
		if (count == irq2Times[i] + 1) {
			// trip irq2status
			ioRege[5] = 1;
			break;
		}
	}
}
// reads memory and disk input files(memin) and puts them in array(output)
// the array will be written to memout or diskout eventually
char* memRead(char output[65536][9], FILE* memin) {
	char* out = output;
	// a string for the line being read to output
	char* line, Lines[MAX_LINE];
	// the current memory line
	line = Lines;
	int i = 0;
	while (!feof(memin)) { //place input file lines into array
		fgets(line, MAX_LINE, memin);
		// null terminate
		strcpy(output[i], line);
		output[i][8] =
			i += 1;
	}
	i += 1;
	// place zeros in output until limit
	while (i < MAX_FILE) { // continuing placing lines until limit
		strcpy(output[i - 1], "00000000\0");
		i += 1;
	}
	// reset input file position without closing file
	fseek(memin, 0, SEEK_SET);
	return out;
}

// opens files safely
// out - pointer to file that is being opened
// name - file name
// type - r for read w for write
FILE* openFile(char* name[], char type) {
	FILE* out = fopen(name, type);
	// if file didn't open leave
	if (out == NULL) {
		exit(1);
	}
	return out;
}
// gets all files and irq2in array and closes/frees them
void closeAndFree(FILE* memin, FILE* mem_out, FILE* regout, FILE* trace, FILE* hwregtrace, FILE* cycles, FILE* leds, FILE* display, FILE* diskin, FILE* irq2in, FILE* diskout, int* irq2Times) {
	// close the files
	fclose(memin);
	fclose(diskin);
	fclose(mem_out);
	fclose(regout);
	fclose(trace);
	fclose(cycles);
	fclose(hwregtrace);
	fclose(irq2in);
	fclose(leds);
	fclose(display);
	fclose(diskout);
	// destroy the irqtimes array since it was malloced
	free(irq2Times);
}



// the main function
// argc - number of command arguments(always 12)
// argv - command arguments(names of all the files)
int main(int argc, char* argv[]) {
	// Reg_Array - array for regular registers, IOReg_array - array for Hardware registers
	int Reg_Array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, IOReg_Array[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	// Rege - pointer to register array, rd, rs, rt - instruction variables, pc - current pc, count - current clock cycle, ioRege - pointer to HW register array, i - 
	int* rege = Reg_Array, rd, rs, rt, pc_help, pc = 0, count = 0, ioRege = IOReg_Array, i = 0;
	// pointers to all the files the simulator is working with
	FILE* memin, * mem_out, * regout, * trace, * hwregtrace,
		* cycles, * leds, * display, * diskin, * irq2in, * diskout;
	// open irq2
	irq2in = openFile(argv[3], "r");
	int* irq2Times = parseirq2(irq2in);
	// line and lines are the current code line.
	// output is an array of all lines for the memout
	// immediate char is used for the immediate value
	char opcode[3], immediate_char[4], output[65536][9], (*out)[9];
	// diskArr contains the disk's memory and i read like the input file
	// the pointer is a portable array pointer for it
	char diskArr[65536][9], (*disk)[9] = diskArr;
	memin = openFile(argv[1], "r"); //input file opening
	trace = openFile(argv[6], "w");//trace file opening
	leds = openFile(argv[9], "w");//leds file opening
	display = openFile(argv[10], "w");//display file opening
	// open the disk input file
	diskin = openFile(argv[3], "r");
	// open the hwregtrace file and exit on fail
	hwregtrace = openFile(argv[7], "w");
	// read the memory
	out = memRead(output, memin);
	// read the disk using same function
	disk = memRead(diskArr, diskin);
	// lines is a string for the current memory line at pc
	char* line, Lines[MAX_LINE];
	// and line is a portable pointer to it
	line = Lines;
	while (!feof(memin)) { // start reading input file line by line and process it 
		// update the disk timer. this wil deal with irq1
		updateDiskTimer(ioRege);
		// deal with irq2
		checkirq2(ioRege, count, irq2Times);
		// move pc in case of an irq event
		pc = checkirq(ioRege, pc, memin);
		// get the current memory line
		fgets(line, MAX_LINE, memin);
		// get the two char opcode
		strncpy(opcode, line, 2);
		opcode[2] = '\0';
		// rd rs and rt using the single char converter
		rd = (int)StrTol(line[2]);
		rs = (int)StrTol(line[3]);
		rt = (int)StrTol(line[4]);
		// copy immediate value to string
		strncpy(immediate_char, line + 5, 3);
		// null terminate
		immediate_char[3] = '\0';
		// sign extend immediate properly - using arithmetic shifts
		int imm_val = (int)strtol(immediate_char, NULL, 16);
		imm_val = imm_val << 20;
		imm_val = imm_val >> 20;
		// put immediate value in register 1
		Reg_Array[1] = imm_val;
		Print_To_Trace(trace, pc, line, Reg_Array);//print to trace
		// check for halt
		if (strcmp(opcode, "13")) {
			// perform opcode operation
			pc = Opcode_Operation((int)strtol(opcode, NULL, 16), rd, rs, rt, immediate_char, rege, ioRege, pc, memin, out, hwregtrace, count, leds, display, disk); //instruction execution function
			fseek(memin, 0, SEEK_SET); //next five lines made for reading the correct line for next instruction
			pc_help = pc;
			while (pc_help > 0) {
				fgets(line, MAX_LINE, memin);
				pc_help -= 1;
			}
			count += 1; //instructions/clock cycles count to be printed into "cycles.txt"
			// update the timer
			updateTimer(ioRege);
		}
		else {
			// in case of a halt
			count += 1;
			break;
		}
	}
	//opening output files that were not written during run
	mem_out = openFile(argv[4], "w");
	regout = openFile(argv[5], "w");
	cycles = openFile(argv[8], "w");
	diskout = openFile(argv[11], "w");
	Print_To_Files(mem_out, regout, trace, cycles, output, count, Reg_Array, disk, diskout); //print to files not written during run
	// close all files
	closeAndFree(memin, mem_out, regout, trace, hwregtrace, cycles, leds, display, diskin, irq2in, diskout, irq2Times);
}
