#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Struct to hold describe different parts of instruction
typedef struct {
	unsigned int opcode;
	unsigned int rd;
	unsigned int rs;
	unsigned int rt;
	unsigned int rm;
	unsigned int immediate1;
	unsigned int immediate2;
} ParsedInstruction;

//Instruction int array of bits
int instructions[4096][48];
//IOregs array
unsigned int IOregs[23];
//Monitor matrix of pixels
unsigned char Monitor[256][256];
//SIMP Regs array
int Reg[16];
//Disk array
int Disk[16384];
//Main memory array
int Memory[4096];
//current PC
unsigned int PC;
//Error flag for debugging
int Error;
//Flag for when the SIMP is still in ISR
int not_in_ISR;
//Flag to halt the simulator
int halt;
//Flag to know when the disk is in use
int disk_is_used;
//counter for the 1024 cycles the disk is in use
unsigned int disk_timer;
//value to store the next interupt from irq2
unsigned int irq2_next_stop;
FILE* irq2in_file_global;

// Function to parse the 48 int array of bits to the ParsedInstruction struct
ParsedInstruction parseBitArray(int bitArray[48]) {
	ParsedInstruction instruction = { 0 };
	int i;

	// Parse opcode (bits 47 to 40)
	for (i = 47; i >= 40; --i) {
		instruction.opcode = (instruction.opcode << 1) | bitArray[i];
	}

	// Parse rd (bits 39 to 36)
	for (i = 39; i >= 36; --i) {
		instruction.rd = (instruction.rd << 1) | bitArray[i];
	}

	// Parse rs (bits 35 to 32)
	for (i = 35; i >= 32; --i) {
		instruction.rs = (instruction.rs << 1) | bitArray[i];
	}

	// Parse rt (bits 31 to 28)
	for (i = 31; i >= 28; --i) {
		instruction.rt = (instruction.rt << 1) | bitArray[i];
	}

	// Parse rm (bits 27 to 24)
	for (i = 27; i >= 24; --i) {
		instruction.rm = (instruction.rm << 1) | bitArray[i];
	}

	// Parse immediate1 (bits 23 to 12)
	for (i = 23; i >= 12; --i) {
		instruction.immediate1 = (instruction.immediate1 << 1) | bitArray[i];
	}

	// Parse immediate2 (bits 11 to 0)
	for (i = 11; i >= 0; --i) {
		instruction.immediate2 = (instruction.immediate2 << 1) | bitArray[i];
	}

	return instruction;
}

//Parse each written instruction the 48 int array of bits
void hexStringToBinary(char* hexString, int* binaryArray, int size) {
	for (int i = 0; i < size; ++i) { // For each hex character
		char hexChar = hexString[i];
		int value = 0;

		// Convert hex character to its integer value
		if (hexChar >= '0' && hexChar <= '9') {
			value = hexChar - '0';
		}
		else if (hexChar >= 'A' && hexChar <= 'F') {
			value = hexChar - 'A' + 10;
		}
		else if (hexChar >= 'a' && hexChar <= 'f') {
			value = hexChar - 'a' + 10;
		}

		// Convert to binary and store
		for (int j = 0; j < 4; ++j) {
			binaryArray[(size - 1 - i) * 4 + j] = (value >> j) & 1;
		}
	}
}

//Initialize all values, read all inputs and parse them to global variables for convinience
void init(char* argv[]) {

	memset(&IOregs, 0, sizeof(IOregs));
	memset(&Monitor, 0, sizeof(Monitor));
	PC = 0;
	Error = 0;
	halt = 0;
	disk_timer = 0;
	not_in_ISR = 1;
	irq2_next_stop = 0;
	for (int j = 0; j < 15; j++) {
		Reg[j] = 0;
	}

	char instruction_line[14];
	char memory_line[10];
	char disk_line[10];
	char irq2_line[501];
	int i;

	FILE* instructions_file = fopen(argv[1], "r");

	if (instructions_file == NULL) {
		perror("Error opening file imemin.txt");
		Error = 1;
		return;
	}

	i = 0;

	while (fgets(instruction_line, 14, instructions_file) != NULL) {
		hexStringToBinary(&instruction_line, &instructions[i], 12);
		i++;
	}

	fclose(instructions_file);

	FILE* memory_in_file = fopen(argv[2], "r");

	if (memory_in_file == NULL) {
		perror("Error opening file dmemin.txt");
		Error = 1;
		return;
	}

	i = 0;

	while (fgets(memory_line, 10, memory_in_file) != NULL) {
		Memory[i] = (int)strtoul(&memory_line, NULL, 16);
		i++;
	}

	fclose(memory_in_file);

	FILE* disk_in_file = fopen(argv[3], "r");

	if (disk_in_file == NULL) {
		perror("Error opening file diskin.txt");
		Error = 1;
		return;
	}

	i = 0;

	while (fgets(disk_line, 10, disk_in_file) != NULL) {
		Disk[i] = (int)strtoul(&disk_line, NULL, 16);
		i++;
	}

	fclose(disk_in_file);

	FILE* irq2in_file = fopen(argv[4], "r");
	irq2in_file_global = irq2in_file;

	if (disk_in_file == NULL) {
		perror("Error opening file diskin.txt");
		Error = 1;
		return;
	}

	//read only on1 line as each next line will be read afterwards
	if (fgets(irq2_line, 501, irq2in_file) != NULL) {
		irq2_next_stop = (unsigned int)strtol(&irq2_line, NULL, 10);
	}
}

//Function which writes to trace and called every cycle for every instruction
void write_to_trace(int PC, ParsedInstruction curr, FILE* trace_file) {

	char PC_s[4];
	sprintf(PC_s, "%03X", PC);
	fprintf(trace_file, "%s ", PC_s);
	char opcode_s[3];
	sprintf(opcode_s, "%02X", curr.opcode);
	fprintf(trace_file, "%s", opcode_s);
	char rd_s[2];
	sprintf(rd_s, "%01X", curr.rd);
	fprintf(trace_file, "%s", rd_s);
	char rs_s[2];
	sprintf(rs_s, "%01X", curr.rs);
	fprintf(trace_file, "%s", rs_s);
	char rt_s[2];
	sprintf(rt_s, "%01X", curr.rt);
	fprintf(trace_file, "%s", rt_s);
	char rm_s[2];
	sprintf(rm_s, "%01X", curr.rm);
	fprintf(trace_file, "%s", rm_s);
	char imm1_s[4];
	sprintf(imm1_s, "%03X", curr.immediate1);
	fprintf(trace_file, "%s", imm1_s);
	char imm2_s[4];
	sprintf(imm2_s, "%03X", curr.immediate2);
	fprintf(trace_file, "%s ", imm2_s);
	char R0[9];
	sprintf(R0, "%08x", Reg[0]);
	fprintf(trace_file, "%s ", R0);
	char R1[9];
	sprintf(R1, "%08x", Reg[1]);
	fprintf(trace_file, "%s ", R1);
	char R2[9];
	sprintf(R2, "%08x", Reg[2]);
	fprintf(trace_file, "%s ", R2);
	char R3[9];
	sprintf(R3, "%08x", Reg[3]);
	fprintf(trace_file, "%s ", R3);
	char R4[9];
	sprintf(R4, "%08x", Reg[4]);
	fprintf(trace_file, "%s ", R4);
	char R5[9];
	sprintf(R5, "%08x", Reg[5]);
	fprintf(trace_file, "%s ", R5);
	char R6[9];
	sprintf(R6, "%08x", Reg[6]);
	fprintf(trace_file, "%s ", R6);
	char R7[9];
	sprintf(R7, "%08x", Reg[7]);
	fprintf(trace_file, "%s ", R7);
	char R8[9];
	sprintf(R8, "%08x", Reg[8]);
	fprintf(trace_file, "%s ", R8);
	char R9[9];
	sprintf(R9, "%08x", Reg[9]);
	fprintf(trace_file, "%s ", R9);
	char R10[9];
	sprintf(R10, "%08x", Reg[10]);
	fprintf(trace_file, "%s ", R10);
	char R11[9];
	sprintf(R11, "%08x", Reg[11]);
	fprintf(trace_file, "%s ", R11);
	char R12[9];
	sprintf(R12, "%08x", Reg[12]);
	fprintf(trace_file, "%s ", R12);
	char R13[9];
	sprintf(R13, "%08x", Reg[13]);
	fprintf(trace_file, "%s ", R13);
	char R14[9];
	sprintf(R14, "%08x", Reg[14]);
	fprintf(trace_file, "%s ", R14);
	char R15[9];
	sprintf(R15, "%08x", Reg[15]);
	fprintf(trace_file, "%s\n", R15);

}

//Function which writes to HWtrace and called every cycle there is an instruction which read/write from IOregs
void write_to_HWtrace(unsigned int idx, int is_write, unsigned int data, FILE* HWtrace_file) {

	fprintf(HWtrace_file, "%d ", IOregs[8]);
	char data_s[9];
	sprintf(data_s, "%08x", data);
	if (is_write == 0) {
		fprintf(HWtrace_file, "%s ", "READ");
	}
	else {
		fprintf(HWtrace_file, "%s ", "WRITE");
	}
	if (idx == 0) {
		fprintf(HWtrace_file, "%s ", "irq0enable");
	}
	if (idx == 1) {
		fprintf(HWtrace_file, "%s ", "irq1enable");
	}
	if (idx == 2) {
		fprintf(HWtrace_file, "%s ", "irq2enable");
	}
	if (idx == 3) {
		fprintf(HWtrace_file, "%s ", "irq0status");
	}
	if (idx == 4) {
		fprintf(HWtrace_file, "%s ", "irq1status");
	}
	if (idx == 5) {
		fprintf(HWtrace_file, "%s ", "irq2status");
	}
	if (idx == 6) {
		fprintf(HWtrace_file, "%s ", "irqhandler");
	}
	if (idx == 7) {
		fprintf(HWtrace_file, "%s ", "irqreturn");
	}
	if (idx == 8) {
		fprintf(HWtrace_file, "%s ", "clks");
	}
	if (idx == 9) {
		fprintf(HWtrace_file, "%s ", "leds");
	}
	if (idx == 10) {
		fprintf(HWtrace_file, "%s ", "display7seg");
	}
	if (idx == 11) {
		fprintf(HWtrace_file, "%s ", "timerenable");
	}
	if (idx == 12) {
		fprintf(HWtrace_file, "%s ", "timercurrent");
	}
	if (idx == 13) {
		fprintf(HWtrace_file, "%s ", "timermax");
	}
	if (idx == 14) {
		fprintf(HWtrace_file, "%s ", "diskcmd");
	}
	if (idx == 15) {
		fprintf(HWtrace_file, "%s ", "disksector");
	}
	if (idx == 16) {
		fprintf(HWtrace_file, "%s ", "diskbuffer");
	}
	if (idx == 17) {
		fprintf(HWtrace_file, "%s ", "diskstatus");
	}
	if (idx == 18) {
		fprintf(HWtrace_file, "%s ", "reserved");
	}
	if (idx == 19) {
		fprintf(HWtrace_file, "%s ", "reserved");
	}
	if (idx == 20) {
		fprintf(HWtrace_file, "%s ", "monitoraddr");
	}
	if (idx == 21) {
		fprintf(HWtrace_file, "%s ", "monitordata");
	}
	if (idx == 22) {
		fprintf(HWtrace_file, "%s ", "monitorcmd");
	}

	fprintf(HWtrace_file, "%s\n", data_s);

}

//Function which checks OPCODE and performs instruction accordingly
void perform_instruction(FILE* trace_file, FILE* HWtrace_file, FILE* leds_file, FILE* display_file) {
	//need to increment PC is 1 by default and change to 0 if there was a branch or alike
	int need_to_increment_PC = 1;
	ParsedInstruction curr = parseBitArray(instructions[PC]);
	//if one immediate of the immediates (stored as unsigned in struct) greater then 0x800, it means it's actually negative and reg[1/2]
	//get it's signed extension version
	if (curr.immediate1 >= 0x800) {
		Reg[1] = 0xFFFFF000 | curr.immediate1; //A way to perform extension with 1's and store the negative number in the reg[1/2]
	}
	else {
		Reg[1] = curr.immediate1; //Positive and therefore no value change
	}
	if (curr.immediate2 >= 0x800) {
		Reg[2] = 0xFFFFF000 | curr.immediate2;
	}
	else {
		Reg[2] = curr.immediate2;
	}
	//When reg[1/2] set, everything is ready to add one more line to trace
	write_to_trace(PC, curr, trace_file);
	if (curr.opcode == 0 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] + Reg[curr.rt] + Reg[curr.rm];
	}
	if (curr.opcode == 1 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] - Reg[curr.rt] - Reg[curr.rm];
	}
	if (curr.opcode == 2 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] * Reg[curr.rt] + Reg[curr.rm];
	}
	if (curr.opcode == 3 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] & Reg[curr.rt] & Reg[curr.rm];
	}
	if (curr.opcode == 4 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] | Reg[curr.rt] | Reg[curr.rm];
	}
	if (curr.opcode == 5 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] ^ Reg[curr.rt] ^ Reg[curr.rm];
	}
	if (curr.opcode == 6 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] << Reg[curr.rt];
	}
	if (curr.opcode == 7 && curr.rd > 2) {
		Reg[curr.rd] = Reg[curr.rs] >> Reg[curr.rt]; //FIXME: signextension: make sure it works (wasn't used in fib)
	}
	if (curr.opcode == 8 && curr.rd > 2) {
		Reg[curr.rd] = ((unsigned int)Reg[curr.rs]) >> Reg[curr.rt]; //FIXME: logicalshift (wasn't used in fib)/
	}
	if (curr.opcode == 9 && Reg[curr.rs] == Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 10 && Reg[curr.rs] != Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 11 && Reg[curr.rs] < Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 12 && Reg[curr.rs] > Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 13 && Reg[curr.rs] <= Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 14 && Reg[curr.rs] >= Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 14 && Reg[curr.rs] >= Reg[curr.rt]) {
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 15) {
		Reg[curr.rd] = PC + 1;
		PC = Reg[curr.rm] & 0xFFF;
		need_to_increment_PC = 0;
	}
	if (curr.opcode == 16) {
		Reg[curr.rd] = Memory[Reg[curr.rs] + Reg[curr.rt]] + Reg[curr.rm];
	}
	if (curr.opcode == 17) {
		Memory[Reg[curr.rs] + Reg[curr.rt]] = Reg[curr.rm] + Reg[curr.rd];
	}
	if (curr.opcode == 18) {
		PC = IOregs[7];
		not_in_ISR = 1;
		need_to_increment_PC = 0;
	}
	//Write_to_HWtrace() is called as there a read from IOregs
	if (curr.opcode == 19) {
		Reg[curr.rd] = IOregs[Reg[curr.rs] + Reg[curr.rt]];
		write_to_HWtrace(Reg[curr.rs] + Reg[curr.rt], 0, Reg[curr.rd], HWtrace_file);
	}
	//Write_to_HWtrace() is called as there a write to IOregs
	if (curr.opcode == 20) {
		IOregs[Reg[curr.rs] + Reg[curr.rt]] = Reg[curr.rm];
		//If there was a write to leds reg, write relevant entry to leds_file
		if (Reg[curr.rs] + Reg[curr.rt] == 9) {
			fprintf(leds_file, "%d ", IOregs[8]);
			char leds_curr[9];
			sprintf(leds_curr, "%08x", IOregs[9]);
			fprintf(leds_file, "%s\n", leds_curr);
		}
		//If there was a write to display7seg reg, write relevant entry to display_file
		if (Reg[curr.rs] + Reg[curr.rt] == 10) {
			fprintf(display_file, "%d ", IOregs[8]);
			char display_curr[9];
			sprintf(display_curr, "%08x", IOregs[10]);
			fprintf(display_file, "%s\n", display_curr);
		}
		write_to_HWtrace(Reg[curr.rs] + Reg[curr.rt], 1, Reg[curr.rm], HWtrace_file);
	}
	if (curr.opcode == 21) {
		halt = 1;
	}
	if (need_to_increment_PC) {
		PC++;
	}

}

//Function which performs IO related procedures after every instruction was performed
void IOprocedures() {

	//If timercurrent == timermax, raise interupt and initialize timercurrent to 0
	if (IOregs[12] == IOregs[13]) {
		IOregs[3] = 1;
		IOregs[12] = 0;
	}
	//else, increment timercurrent but only if timerenable is 1
	else {
		if (IOregs[11] == 1) {
			IOregs[12]++;
		}
	}
	//If monitorcmd == 1, perform write to monitor, when the relevant line would be "monitoraddr/256" and relevant collumn "monitoraddr%256"
	if (IOregs[22]) {
		Monitor[IOregs[20] / 256][IOregs[20] % 256] = IOregs[21];
		//after write was done, initialize monitorcmd to 0 as it should be 0 every ocassion it could be read (spec)
		IOregs[22] = 0;
	}
	//If diskcmd == 1 (read) and diskstatus == 0 (not busy - that way if busy it will detect violation of protocol and do nothing), read from disk
	if (IOregs[14] == 1 & !IOregs[17]) {
		//read 128 lines from Disk (1 sector) from disksector address in disk to diskbuffer address in memory
		for (int i = 0; i < 128; i++) {
			Memory[IOregs[16] + i] = Disk[(IOregs[15] * 128) + i];
		}
		//set diskstatus to 1
		IOregs[17] = 1;
	}
	if (IOregs[14] == 2 & !IOregs[17]) {
		//write 128 lines to Disk (1 sector) from diskbuffer address in memory to disksector address in disk
		for (int i = 0; i < 128; i++) {
			Disk[(IOregs[15] * 128) + i] = Memory[IOregs[16] + i];
		}
		//set diskstatus to 1
		IOregs[17] = 1;
	}
	//if diskstatus is up, increment disk timer until it reaches 1024 so then an interupt can be raised, diskstatus intialized to 0 (and disktimer too)
	if (IOregs[17]) {
		if (disk_timer < 1024) {
			disk_timer++;
		}
		else {
			disk_timer = 0;
			IOregs[14] = 0;
			IOregs[17] = 0;
			IOregs[4] = 1;
		}
	}
	//When reached next stop from irq2 (irq2_next_stop == clks) interupts, raise an interupt and read one more line from irq2in.txt to set irq2_next_stop
	if (irq2_next_stop == IOregs[8]) {
		IOregs[5] = 1;
		char irq2_line[501];
		if (fgets(irq2_line, 501, irq2in_file_global) != NULL) {
			//printf("The irq2in_line is: %sat time %d\n", irq2_line, IOregs[8]);
			irq2_next_stop = (unsigned int)strtol(&irq2_line, NULL, 10);
		}
	}
	//if not the time to stop, make sure interupt from irq2 is 0
	else {
		IOregs[5] = 0;
	}
	//if clks didn't reach 0xFFFFFFFF then incrememnt, otherwise initialize to 0
	if (IOregs[8] != 0xFFFFFFFF) {
		IOregs[8]++;
	}
	else {
		IOregs[8] = 0;
	}
}

void create_dmemout(char* argv[]) {

	FILE* dmemout_file = fopen(argv[5], "w");
	if (dmemout_file == NULL) {
		perror("Error opening file dmemout.txt");
		return;
	}
	for (int i = 0; i < 4096; i++) {
		char buffer[9];
		sprintf(buffer, "%08X", Memory[i]);
		fprintf(dmemout_file, "%s\n", buffer);
	}
	fclose(dmemout_file);

}

void create_diskout(char* argv[]) {

	FILE* diskout_file = fopen(argv[12], "w");
	if (diskout_file == NULL) {
		perror("Error opening file diskout.txt");
		return;
	}
	for (int i = 0; i < 16384; i++) { //will be many zeros lines, is it fine?
		char buffer[9];
		sprintf(buffer, "%08X", Disk[i]);
		fprintf(diskout_file, "%s\n", buffer);
	}
	fclose(diskout_file);

}

void create_regout(char* argv[]) {

	FILE* regout_file = fopen(argv[6], "w");
	if (regout_file == NULL) {
		perror("Error opening file regout.txt");
		return;
	}
	for (int i = 0; i < 13; i++) {
		char buffer[9];
		sprintf(buffer, "%08X", Reg[i + 3]);
		fprintf(regout_file, "%s\n", buffer);
	}
	fclose(regout_file);

}

void create_cycles(char* argv[]) {

	FILE* cycles_file = fopen(argv[9], "w");
	if (cycles_file == NULL) {
		perror("Error opening file cycles.txt");
		return;
	}
	fprintf(cycles_file, "%d", IOregs[8]);
	fclose(cycles_file);

}

void create_monitor_txt(char* argv[]) {

	FILE* monitor_file = fopen(argv[13], "w");
	if (monitor_file == NULL) {
		perror("Error opening file monitor.txt");
		return;
	}
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			char buffer[3];
			sprintf(buffer, "%02X", Monitor[i][j]);
			fprintf(monitor_file, "%s\n", buffer);
		}
	}
	fclose(monitor_file);
}

create_monitor_yuv(char* argv[]) {

	FILE* monitor_yuv = fopen(argv[14], "wb");
	if (!monitor_yuv) {
		perror("Failed to open monitor.yuv");
		return;
	}

	for (int y = 0; y < 256; ++y) {
		if (fwrite(Monitor[y], 1, 256, monitor_yuv) != 256) {
			perror("Failed to write Y component");
			fclose(monitor_yuv);
			return;
		}
	}
	fclose(monitor_yuv);

}

int main(int argc, char* argv[]) {

	//initialize all values and data holders before starting implementing the program itself, open input files and read them into appropriate holders
	init(argv);
	//Files which being written on the run on dependency of the code are opened here and passed to perform instruction as arguments as they are written there
	FILE* trace_file = fopen(argv[7], "w");
	if (trace_file == NULL) {
		perror("Error opening file trace.txt");
		return 1;
	}
	FILE* HWtrace_file = fopen(argv[8], "w");
	if (HWtrace_file == NULL) {
		perror("Error opening file hwregtrace.txt");
		return 1;
	}
	FILE* leds_file = fopen(argv[10], "w");
	if (leds_file == NULL) {
		perror("Error opening file leds.txt");
		return 1;
	}
	FILE* display_file = fopen(argv[11], "w");
	if (display_file == NULL) {
		perror("Error opening file display7seg.txt");
		return 1;
	}
	//While there halt command wasn't executed and Error wasn't raised, perform the loop:
	//each iteration first check for interupts and their enables and if the same interupt kind has status and enable on, go into ISR
	//then perform the instruction which changes register values/PC value/Memory/IOregs or halts
	//then perform IOprocedures as function of current state of IOregs
	while (!halt & !Error) {
		int irq = (IOregs[0] & IOregs[3]) | (IOregs[1] & IOregs[4]) | (IOregs[2] & IOregs[5]);
		if (irq & not_in_ISR) {
			not_in_ISR = 0;
			IOregs[7] = PC;
			PC = IOregs[6];
		}
		perform_instruction(trace_file, HWtrace_file, leds_file, display_file);
		IOprocedures();
	}
	//Close all files that have been in use
	fclose(irq2in_file_global);
	fclose(trace_file);
	fclose(leds_file);
	fclose(display_file);
	fclose(HWtrace_file);
	//Create Output txts that are dependant only on the final state of some holders like Regs/Monitor/IOregs/Disk/Memory
	create_dmemout(argv);
	create_regout(argv);
	create_cycles(argv);
	create_diskout(argv);
	create_monitor_txt(argv);
	create_monitor_yuv(argv);
	//If Error was raised return 1 for debugging purposed, otherwise Error initially is 0
	return Error;

}