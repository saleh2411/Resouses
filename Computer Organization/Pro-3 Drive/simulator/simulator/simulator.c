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
// is the only input.
void updateTimer(unsigned int* ioRege) {
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
void updateLD(unsigned int cycle, int regNum, unsigned int* ioRege, FILE* leds, FILE* display) {
	char toWrite[100] = "";
	// start with cycle
	_ultoa(cycle, toWrite, 10);
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
void updatehwRegTrace(unsigned int cycle, int action, int reg, unsigned int* ioRege, FILE* hwregTrace) {
	// big string for current line, slowly strings will be added to it then it will be printed
	// to the file after the null wrap
	char toWrite[2000] = "";
	// start with cycle
	_ultoa(cycle, toWrite, 10);
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
void diskOperation(unsigned int* ioRege, char out[][9], char disk[][9]) {
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
	else { // writing loop
		for (int i = 0; i < 128; i++) {
			// copy memory buffer into disk sector
			strcpy(disk[i + sectorStart], out[i + ioRege[16]]);
		}
	}
	// disk is now busy. so reset the timer
	ioRege[17] = 1;
	diskTimer = 0;
}

// a utility function to move the file pointer to the correct pc
void moveFP(FILE *memin, long pc) {
	char line[MAX_LINE];
	fseek(memin, 0, SEEK_SET); //next five lines made for reading the correct line for next instruction
	int pc_help = pc;
	while (pc_help > 0) {
		fgets(line, MAX_LINE, memin);
		pc_help -= 1;
	}
}

// this struct will be used to save the memory lines, similar to one in assembler but not a list
typedef struct MemoryLine {
	// the opcode of the command
	int opcode;
	// the registers
	int rd;
	int rs;
	int rt;
	// immediate value
	int imm;
	// a copy of the line string
	char line[MAX_LINE];
}MemoryLine;

// each opcode has it's own function to shorten the length of functions

// each opcode function takes integers of the instruction parameters, the current pc and line data struct. and returns the pc after.
// arithmetic functions take no more than that

// this executes "add" instructions
// current - all instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int add(MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 0) { //ADD instruction
		rege[current->rd] = rege[current->rs] + rege[current->rt];
		pc += 1;
	}
	return pc;
}

// this executes "sub" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sub(MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 1) { //SUB instruction
		rege[current->rd] = rege[current->rs] - rege[current->rt];
		pc += 1;
	}
	return pc;
}

// this executes "and" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int and (MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 2) {//and instruction
		rege[current->rd] = (rege[current->rs] & rege[current->rt]);
		pc += 1;
	}
	return pc;
}

// this executes "or" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int or (MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 3) { //or instruction
		rege[current->rd] = (rege[current->rs] | rege[current->rt]);
		pc += 1;
	}
	return pc;
}

// this executes "sll" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sll(MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 4) { //sll instruction
		rege[current->rd] = (rege[current->rs] << rege[current->rt]);
		pc += 1;
	}
	return pc;
}

// this executes "sra" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int sra(MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 5) { //sra instruction. uses the normal >> because
	// all values are signed
		rege[current->rd] = (rege[current->rs] >> rege[current->rt]);
		pc += 1;
	}
	return pc;
}

// this executes "srl" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// returns pc after execution
int srl(MemoryLine* current, long pc, int* rege) {
	if (current->opcode == 6) { //srl instruction, uses a special code
		int size = sizeof(int);
		// shift
		// deal with the zero case
		if (rege[current->rs] == 0)
			rege[current->rd] = rege[current->rs];
		else /// else use modified >> operator. it uses a bitmask that only lets the original bits pass to zero any newly created bit
			rege[current->rd] = (rege[current->rs] >> rege[current->rt]) & ~(((rege[current->rs] >> (size << 3) - 1) << (size << 3) - 1)) >> (rege[current->rt] - 1);
		if (current->rd == 0)
			rege[current->rd] = 0;
		pc += 1;
	}
	return pc;
}

// the functions for branch and jump instructions take "file" the pointer to memin they need to move to the correct location
// so the line at pc will be read

// this executes "beq" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int beq(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 7) {//beq instruction
	// do in case of equality
		if (rege[current->rs] == rege[current->rt]) {
			// jump pc to immediate value
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}// otherwise continue to the next row
		else
			pc += 1;
	}
	return pc;
}
// this executes "bne" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bne(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 8) {
		if (rege[current->rs] != rege[current->rt]) {
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "blt" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int blt(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 9) { //blt instruction
		if (rege[current->rs] < rege[current->rt]) {
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "bgt" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bgt(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 10) { //bgt instruction
		if (rege[current->rs] > rege[current->rt]) {
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "ble" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int ble(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 11) { //ble instruction
		if (rege[current->rs] <= rege[current->rt]) {
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}
		else
			pc += 1;
	}
	return pc;
}
// this executes "bge" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int bge(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 12) { // bge instruction
		if (rege[current->rs] >= rege[current->rt]) {
			pc = rege[current->rd];
			// move the pointer
			moveFP(file, pc);
		}
		else
			pc += 1;
	}
	return pc;
}

// this executes "jal" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// file - pointer to memin
// returns pc after execution
int jal(MemoryLine* current, long pc, int* rege, FILE* file) {
	if (current->opcode == 13) {	//jal instruction
	// put original pc(+1) in return address register
		rege[15] = pc + 1;
		//  move pc similar to the branch instructions
		pc = rege[current->rd];
		// move the pointer
		moveFP(file, pc);
	}
	return pc;
}

// this executes "lw" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// output - current memory as an array
// returns pc after execution
int lw(MemoryLine* current, long pc, int* rege, char output[][9]) {
	// stores the memory world we load as an int
	int MEM;
	// and as a string
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (current->opcode == 14) {	//lw instruction
	// address of word
		int lines = rege[current->rs] + rege[current->rt];
		// get the hexadecimal word
		strcpy(line2, output[lines]);
		// convert to an integer
		MEM = (int)strtol(line2, NULL, 16);
		// put in register
		rege[current->rd] = MEM;
		// zero lock(again)
		if (current->rd == 0)
			rege[current->rd] = 0;
		pc += 1;
	}
	return pc;
}

// this executes "sw" instructions
// current - instruction variables
// pc - current pc
// rege - register array
// output - current memory as an array
// returns pc after execution
int sw(MemoryLine* current, long pc, int* rege, char output[][9]) {
	// and as a string
	char* line2, LINES[MAX_LINE];
	line2 = LINES;
	if (current->opcode == 15) { //sw instruction
		char hexval[9];
		// get line index ffor the store
		int lines = rege[current->rs] + rege[current->rt];
		// convert rd value to hexacdecimal
		sprintf(hexval, "%08X", rege[current->rd]);
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
int reti(int opcode, unsigned int* ioRege, long pc) {
	if (opcode == 16) {
		// reti command, very simple
		pc = ioRege[7];
		isirqEvent = 0;
	}
	return pc;
}

// in operation. gets
// current - memory line struct of instruction variables(registers and immediate and opcode)
// rege - pointer to register array
// ioRege - pointer to HW register array
// cycle - current clock cycle
// hwregTrace - pointer to hw register array
// pc - current pc
// returns pc after execution
int in(MemoryLine* current, int* rege, unsigned int* ioRege, unsigned int cycle, FILE* hwregTrace, long pc) {
	if (current->opcode == 17) {
		// in command. as specified in instructions
		rege[current->rd] = ioRege[rege[current->rs] + rege[current->rt]];
		updatehwRegTrace(cycle, 1, rege[current->rs] + rege[current->rt], ioRege, hwregTrace);
		pc = pc + 1;
	}
	return pc;
}

// out operation. gets
// current - memory line struct of instruction variables(registers and immediate and opcode)
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
int out(MemoryLine* current, int* rege, unsigned int* ioRege, unsigned int cycle, FILE* hwregTrace, long pc, char output[][9], char disk[][9], FILE* leds, FILE* display) {
	if (current->opcode == 18) {
		// update ioRege and hwregtrace file like in in
		ioRege[rege[current->rs] + rege[current->rt]] = rege[current->rd];
		updatehwRegTrace(cycle, 0, rege[current->rs] + rege[current->rt], ioRege, hwregTrace);
		// update leds and display files
		if ((rege[current->rs] + rege[current->rt] == 9) || (rege[current->rs] + rege[current->rt] == 10)) {
			updateLD(cycle, rege[current->rs] + rege[current->rt], ioRege, leds, display);
		}
		// check if disk command register was updated. if so execute the operation
		if ((rege[current->rs] + rege[current->rt] == 14)) {
			diskOperation(ioRege, output, disk);
		}
		pc = pc + 1;
	}
	return pc;
}

// performs command as opcode specifies. and returns the pc after it's execution
// current - struct of the line being read
// rege - register array pointer, ioRege - HW register array pointer
// pc - the current pc
// file - memory input file
// output - memory output array
// hwregtrace - hardware register trace file pointer
// cycle - current clock cycle
// leds and display - file pointers to them
// disk - disk content array
int Opcode_Operation(MemoryLine* current, int* rege, unsigned int* ioRege, long pc, FILE* file, char output[][9], FILE* hwregTrace, unsigned int cycle, FILE* leds, FILE* display, char disk[][9]) {
	// while MEM is used to convert hexadecimal string to integer in load
	// actually. only one of those functions will execute because an if will take it out
	pc = add(current, pc, rege);
	pc = sub(current, pc, rege);
	pc = and (current, pc, rege);
	pc = or (current, pc, rege);
	pc = sll(current, pc, rege);
	pc = sra(current, pc, rege);
	pc = srl(current, pc, rege);
	pc = beq(current, pc, rege, file);
	pc = bne(current, pc, rege, file);
	pc = blt(current, pc, rege, file);
	pc = bgt(current, pc, rege, file);
	pc = ble(current, pc, rege, file);
	pc = bge(current, pc, rege, file);
	pc = jal(current, pc, rege, file);
	pc = lw(current, pc, rege, output);
	pc = sw(current, pc, rege, output);
	pc = reti(current->opcode, ioRege, pc);
	pc = in(current, rege, ioRege, cycle, hwregTrace, pc);
	pc = out(current, rege, ioRege, cycle, hwregTrace, pc, output, disk, leds, display);

	// prevent changing zero
	if (current->rd == 0)
		rege[current->rd] = 0;

	return pc;
}

// updates the "busy disk" timer and resets it on 1024 clock cycles
// gets ioRege - HW register array
void updateDiskTimer(unsigned int* ioRege) {
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
Print_To_Files(FILE* mem_out, FILE* regout, FILE* trace, FILE* cycles, char output[][9], unsigned int count, int Reg_Array[], char disk[][9], FILE* diskout) {
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
	i = MAX_FILE - 2; // reset i and j to write disk out
	j = 0;
	// write disk out same way as memout
	while (strcmp(disk[i], "00000000") == 0)
		i -= 1;
	while (j <= i) {
		fputs(disk[j], diskout);
		putc('\n', diskout);
		j += 1;
	}
	// write the amount of cycles the program ran
	fprintf(cycles, "%u", count);

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
int checkirq(unsigned int* ioRege, int pc, FILE* file) {
	// in case of irq event return current pc
	if (isirqEvent) {
		return pc;
	}
	// the irq boolean as specified
	if ((ioRege[0] && ioRege[3]) || (ioRege[1] && ioRege[4]) || (ioRege[2] && ioRege[5])) {
		// return address
		ioRege[7] = pc;
		isirqEvent = 1;
		moveFP(file, ioRege[6]); // move memin file pointer to irq handler
		return ioRege[6];
	}
	// default case - keep current pc
	return pc;
}
// trips irq2 status at the currect clock cycles
// gets pointer to HW registers, clock cycle count and
// the irq2 trigger time array
void checkirq2(unsigned int* ioRege, unsigned int count, int* irq2Times) {
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
		output[i][8] = '\0';
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
FILE* openFile(char* name[], char *type) {
	FILE* out = fopen(name, type);
	// if file didn't open leave
	if (out == NULL) {
		exit(1);
	}
	return out;
}

// gets all files and irq2in array and memory line struct "current" and closes/frees them
void closeAndFree(FILE* memin, FILE* mem_out, FILE* regout, FILE* trace, FILE* hwregtrace, FILE* cycles, FILE* leds, FILE* display, FILE* diskin, FILE* irq2in, FILE* diskout, int* irq2Times, MemoryLine* current) {
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
	// same with memory line struct
	free(current);
}

// copies to memory line "out" from a line in memin.txt
MemoryLine* Create(FILE *memin, MemoryLine *out) {
	// two strings for opcode and immediate
	char opcode[3], immediate_char[4];
	// get the current memory line
	fgets(out->line, MAX_LINE, memin);
	// get the two char opcode
	strncpy(opcode, out->line, 2);
	opcode[2] = '\0';
	// sting int converter
	out->opcode = (int)strtol(opcode, NULL, 16);
	// rd rs and rt using the single char converter
	out->rd = (int)StrTol(out->line[2]);
	out->rs = (int)StrTol(out->line[3]);
	out->rt = (int)StrTol(out->line[4]);
	// copy immediate value to string
	strncpy(immediate_char, out->line + 5, 3);
	// null terminate
	immediate_char[3] = '\0';
	// sign extend immediate properly - using arithmetic shifts
	out->imm = (int)strtol(immediate_char, NULL, 16);
	out->imm = out->imm << 20;
	out->imm = out->imm >> 20;
}

// updates the clock cycle count and the clks IO register
// count - current clock cycle
// ioRege - hw register
int updateclks(unsigned unsigned int count, unsigned int *ioRege) {
	count++; // update clock cycle count
	ioRege[8] = count;// copy to io Register
	return count;
}

// the main function
// argc - number of command arguments(always 12)
// argv - command arguments(names of all the files)
int main(int argc, char* argv[]) {
	int Reg_Array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Reg_Array - array for regular registers
	int* rege = Reg_Array, pc = 0; // Rege - pointer to register array, pc - current pc, count - current clock cycle, ioRege - pointer to HW register array
	unsigned unsigned int count = 0, IOReg_Array[18] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, *ioRege = IOReg_Array; // count - clock cycle count, IOReg_array - array for Hardware registers, ioRege - pointer to it
	FILE* memin, * mem_out, * regout, * trace, * hwregtrace, * cycles, * leds, * display, * diskin, * irq2in, * diskout; // pointers to all the files the simulator is working with
	irq2in = openFile(argv[3], "r"); // open irq2
	int* irq2Times = parseirq2(irq2in);
	char output[65536][9], (*out)[9]; // output is an array of all lines for the memout. and out is a pointer to it
	char diskArr[65536][9], (*disk)[9] = diskArr; // diskArr contains the disk's memory and i read like the input file the pointer is a portable array pointer for it
	memin = openFile(argv[1], "r"); //input file opening
	trace = openFile(argv[6], "w");//trace file opening
	leds = openFile(argv[9], "w");//leds file opening
	display = openFile(argv[10], "w");//display file opening
	diskin = openFile(argv[2], "r");// open the disk input file
	hwregtrace = openFile(argv[7], "w");// open the hwregtrace file and exit on fail
	out = memRead(output, memin); // read the memory
	disk = memRead(diskArr, diskin); // read the disk using same function
	MemoryLine* current = malloc(sizeof(MemoryLine)); // current line structure
	while (!feof(memin)) { // start reading input file line by line and process it update the disk timer. this wil deal with irq1
		updateDiskTimer(ioRege); 
		checkirq2(ioRege, count, irq2Times); // deal with irq2
		pc = checkirq(ioRege, pc, memin); // move pc in case of an irq event
		current = Create(memin, current); // copy line to struct
		Reg_Array[1] = current->imm; // copy immediate to register
		Print_To_Trace(trace, pc, current->line, Reg_Array);//print to trace
		if (current->opcode != 19) { // check if not on a halt
			pc = Opcode_Operation(current, rege, ioRege, pc, memin, out, hwregtrace, count, leds, display, disk); // perform opcode operation then go to correct row.
			moveFP(memin, pc); // move file pointer to correct row
			count = updateclks(count, ioRege); //update clock cycles
			updateTimer(ioRege); // update the timer
		}
		else { // in case of a halt. just update clock cycle and leave
			count = updateclks(count, ioRege); //update clock cycles
			break;
		}
	}
	//opening output files to be written after the run
	mem_out = openFile(argv[4], "w");
	regout = openFile(argv[5], "w");
	cycles = openFile(argv[8], "w");
	diskout = openFile(argv[11], "w");
	Print_To_Files(mem_out, regout, trace, cycles, output, count, Reg_Array, disk, diskout); //print to files not written during run
	closeAndFree(memin, mem_out, regout, trace, hwregtrace, cycles, leds, display, diskin, irq2in, diskout, irq2Times, current); // close all files
}
