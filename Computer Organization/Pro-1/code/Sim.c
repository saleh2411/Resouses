#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
//libraryes we should need 
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// specific values that we need 

#define DISK_CYCLES 1024
#define SECTOR_SIZE 128
#define REG_NUM 16
#define INSTRUCTION_MEMORY_SIZE 1024
#define DATA_SIZE 4096
#define DISK_SIZE 40*(1024/4)      
#define IMM 1
#define ra 15

enum args_of_files {            // enumes for the file num 
	MEMIN = 1,
	MEMOUT,
	REGOUT,
	TRACE,
	CYCLES,
};


void CopyRegisters();
void file_opening_check(FILE* file, char filename[]);
char* get_IO_reg_name(int IORegisterNum);
void read_from_disk();
void write_to_disk();
int get_IO_reg(int IORegisterNum);
void set_IO_reg(int IORegisterNum, int value);
void file_opening_check(FILE* file, char filename[]);
void write_to_cycles(char filename[], int numofAssemblyOp);
void read_in_hex(char filename[], int hex_out[], int size);
void write_in_hex(char filename[], int inArray[], int size);
void write_to_trace(int lastPC, int registers[], int instruction);

FILE* traceFile;
FILE* irq2in;

int Registers[REG_NUM] = { 0 };
int Copy_of_registers[REG_NUM];
char irq_enable = 0;
char irq_status = 0;
int irq_handler = 0;
int irq_return = 0;
int leds = 0;
int taken = 0;
char timer_enable = 0;
int timer_current = 0;
int timer_max = 0;
char disk_cmd = 0;
char disk_sector = 0;
int disk_buffer = 0;
char disk_status = 0;
char monitor_cmd = 0;
int monitor = 0;
unsigned char monitor_data = 0;
int current_disk__cycle;
char* hwregtrace_file_name;
char* leds_file_name;
void(*OpcodeMap[22])(int, int, int);
unsigned int irq2stopCycles;
int PC = 0;
int Cycle = 0;
int instruction_memory[INSTRUCTION_MEMORY_SIZE];
int memory[DATA_SIZE];
int Disk[DISK_SIZE];
char flag_for_Exit = 0;
unsigned int irq2_stop_cycles;
char interrupted = 0;
char reset_irq2_status = 0;
char irq2_intterupt = 0;
char irq_enable;
char irq_status;
int irq_handler;
int irq_return;
char timer_enable;
int timer_max;
int timer_current;
char* leds_file_name;
int irq_return;
int current_disk_Cycle;
char disk_cmd;
char disk_status;


void cycle_incremenation() {

	Cycle++;
	if (irq2_intterupt)
	{
		irq_status |= 0b100; // irq2status == 1
		irq2_intterupt = 0;
	}

	if (timer_enable)
	{
		if (timer_current == timer_max)
		{
			irq_status |= 0b001; // irq0status == 1
			timer_current = 0;
		}
		else
		{
			timer_current++;
		}

	}


	if (Cycle == (current_disk_Cycle + DISK_CYCLES))
	{
		if (1 == disk_cmd)
		{
			read_from_disk();
		}
		else if (2 == disk_cmd)
		{
			write_to_disk();
		}
		disk_cmd = 0;
		disk_status = 0;
		irq_status |= 0b010; // irq1status == 1
	}
}

void incremenation() {
	PC++;
	cycle_incremenation();
}

void extract_imm()             // takes the imm vlaue 
{
	incremenation();
	int flag_for_sign = (instruction_memory[PC] & 0x80000) >> 19;
	if (flag_for_sign)
	{
		Registers[IMM] = -((instruction_memory[PC] ^ 0xFFFFF) + 1);
	}
	else
	{
		Registers[IMM] = instruction_memory[PC];
	}
}

void start_op(int rd, int rs, int rt)                           // checks if there is an imm value
{
	if (1 == rs || 1 == rt || rd == 1) {
		extract_imm();
	}
}

void stop_op(int rd, int rs, int rt)
{
	if (0 == rd)
	{
		Registers[0] = 0;
	}
	incremenation();
}

void nonAssignStart(int rd, int rs, int rt)
{
	if (1 == rd || 1 == rs || 1 == rt)
	{
		extract_imm();
	}
}

void jump_to_reg(int reg)
{
	PC = Registers[reg] & 0x3FF;//[low bits 9:0]

	cycle_incremenation();
}


void add(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] + Registers[rt];
	stop_op(rd, rs, rt);
}

void sub(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] - Registers[rt];
	stop_op(rd, rs, rt);
}

void and (int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] & Registers[rt];
	stop_op(rd, rs, rt);
}

void or (int rd, int rs, int rt)
{

	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] | Registers[rt];
	stop_op(rd, rs, rt);
}

void xor (int rd, int rs, int rt)
{

	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] ^ Registers[rt];
	stop_op(rd, rs, rt);
}
void mul(int rd, int rs, int rt)
{

	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] * Registers[rt];
	stop_op(rd, rs, rt);
}

void sll(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = (unsigned)Registers[rs] << Registers[rt];
	stop_op(rd, rs, rt);
}

void sra(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = Registers[rs] >> Registers[rt];
	stop_op(rd, rs, rt);
}

void srl(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = (unsigned)Registers[rs] >> Registers[rt];
	stop_op(rd, rs, rt);
}

void beq(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] == Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void bne(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] != Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void blt(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] < Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void bgt(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] > Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void ble(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] <= Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void bge(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	if (Registers[rs] >= Registers[rt])
	{
		jump_to_reg(rd);
	}
	else
	{
		incremenation();
	}
}

void jal(int rd, int rs, int rt)                      // R[rd] = next instruction address, pc = R[rs]
{
	if (1 == rd || rs == 1)
	{
		extract_imm();
	}
	Registers[rd] = PC + 1;
	PC = Registers[rs];

}

void lw(int rd, int rs, int rt)                              // R[rd] = MEM[R[rs]+R[rt]], with sign extension
{
	start_op(rd, rs, rt);
	Registers[rd] = memory[Registers[rs] + Registers[rt]];
	stop_op(rd, rs, rt);
}

void sw(int rd, int rs, int rt)                                  // rd rs rt MEM[R[rs] + R[rt]] = R[rd](bits 19:0)
{
	nonAssignStart(rd, rs, rt);
	memory[Registers[rs] + Registers[rt]] = Registers[rd];
	incremenation();
}

void reti(int rd, int rs, int rt)
{
	PC = irq_return & 0x3FF;
	interrupted = 0;
	cycle_incremenation();
}

void in(int rd, int rs, int rt)
{
	start_op(rd, rs, rt);
	Registers[rd] = get_IO_reg(Registers[rs] + Registers[rt]);
	stop_op(rd, rs, rt);
}

void out(int rd, int rs, int rt)
{
	nonAssignStart(rd, rs, rt);
	set_IO_reg(Registers[rs] + Registers[rt], Registers[rd]);
	incremenation();
}

void halt(int rd, int rs, int rt)
{
	cycle_incremenation();
	flag_for_Exit = 1;
}

void(*OpcodeMap[22])(int, int, int) = {
	add,
	sub,
	and,
	or ,
	xor,
	mul,
	sll,
	sra,
	srl,
	beq,
	bne,
	blt,
	bgt,
	ble,
	bge,
	jal,
	lw,
	sw,
	reti,
	in,
	out,
	halt
};





int get_IO_reg(int IORegisterNum)
{
	int retVal;
	switch (IORegisterNum)
	{
	case 0:
		retVal = (irq_enable & 0b001) ? 1 : 0;
		break;
	case 1:
		retVal = (irq_enable & 0b010) ? 1 : 0;
		break;
	case 2:
		retVal = (irq_enable & 0b100) ? 1 : 0;
		break;
	case 3:
		retVal = (irq_status & 0b001) ? 1 : 0;
		break;
	case 4:
		retVal = (irq_status & 0b010) ? 1 : 0;
		break;
	case 5:
		retVal = (irq_status & 0b100) ? 1 : 0;
		break;
	case 6:
		retVal = irq_handler & 0x3FF;
		break;
	case 7:
		retVal = irq_return & 0x3FF;
		break;
	case 8:
		retVal = Cycle;
		break;
	case 9:
		retVal = Cycle;
		break;

	default:
		printf("Attempt to access unrecognized IO register");
		exit(1);
		return -1;
	}
	return retVal;
}


void set_IO_reg(int IORegisterNum, int value)
{
	switch (IORegisterNum)
	{
	case 0:
		irq_enable = (0 == value) ? (irq_enable & 0b110) : (irq_enable | 0b001);
		break;
	case 1:
		irq_enable = (0 == value) ? (irq_enable & 0b101) : (irq_enable | 0b010);
		break;
	case 2:
		irq_enable = (0 == value) ? (irq_enable & 0b011) : (irq_enable | 0b100);
		break;
	case 3:
		irq_status = (0 == value) ? (irq_enable & 0b110) : (irq_enable | 0b001);
		break;
	case 4:
		irq_status = (0 == value) ? (irq_enable & 0b101) : (irq_enable | 0b010);
		break;
	case 5:
		irq_status = (0 == value) ? (irq_enable & 0b011) : (irq_enable | 0b100);
		break;
	case 6:
		irq_handler = value & 0x3FF;
		break;
	case 7:
		irq_return = value & 0x3FF;
		break;
	case 8:
		//clks is a read only variable for us
		break;
		/*
		case 9:
			if (0 == disk_cmd) // in order to prevent the assembler access to the disk while it's in the middle of a previous task
			{
				disk_cmd = value & 0b11;
				if ((1 == disk_cmd) | (2 == disk_cmd))
				{
					disk_status = 1;
					current_disk_Cycle = Cycle;
				}
			}
			break;
		*/
	default:
		printf("Attempt to access unrecognized IO register");
		exit(1);
		return;
	}
}

char* get_IO_reg_name(int IORegisterNum)
{
	switch (IORegisterNum)
	{
	case 0:
		return "irq0enable";
	case 1:
		return "irq1enable";
	case 2:
		return "irq2enable";
	case 3:
		return "irq0status";
	case 4:
		return "irq1status";
	case 5:
		return "irq2status";
	case 6:
		return "irqhandler";
	case 7:
		return "irqreturn";
	case 8:
		return "clks";
		/*
	case 9:
	*/
	default:
		printf("Attempt to access unrecognized IO register");
		exit(1);
		return "1";
	}

}






void read_from_disk()
{
	int disk_Sector_p = disk_sector * SECTOR_SIZE;
	int i = 0;
	while (i < SECTOR_SIZE)
	{
		memory[disk_buffer + i] = Disk[disk_Sector_p + i];
		i++;
	}
}

void write_to_disk()
{
	int disk_Sector_P = disk_sector * SECTOR_SIZE;
	int i = 0;

	while (i < SECTOR_SIZE)
	{
		Disk[disk_Sector_P + i] = memory[disk_buffer + i];
		i++;
	}
}

void CopyRegisters()
{
	for (int i = 0; i < REG_NUM; i++)
	{
		Copy_of_registers[i] = Registers[i];
	}
}

void file_opening_check(FILE* file, char filename[])
{
	if (NULL == file) {
		printf("Error opening file %s!\n", filename);
		exit(1);
	}
}

void read_in_hex(char filename[], int hex_out[], int size)
{
	unsigned int memory;
	FILE* file = fopen(filename, "r");

	file_opening_check(file, filename);

	int i = 0;
	while (i < size && fscanf(file, "%x", &memory) != EOF)
	{
		hex_out[i] = memory;
		i++;
	}

	while (i < size) {
		hex_out[i] = 0;
		i++;
	}

	fclose(file);
}
void write_to_cycles(char filename[])
{
	FILE* file = fopen(filename, "w");

	file_opening_check(file, filename);

	fprintf(file, "%d\n", Cycle);

}

void write_in_hex(char filename[], int hex_arr[], int size)
{
	FILE* file = fopen(filename, "w");

	file_opening_check(file, filename);

	for (int i = 0; i < size; i++)
	{
		if (size == REG_NUM) {
			if (i == 0 || i == 1)
				continue;
		}
		if (hex_arr[i] <= 0xCB228) {  
			fprintf(file, "%05X\n", hex_arr[i]);
		}
		else {
			for (int j = i; j < size; j++) {
				fprintf(file, "%05X\n", 0x00000);
			}
			i = size;
		}
	}
}


void write_to_trace(int lastPC, int registers[], int instruction)
{
	fprintf(traceFile, "%03X %05X", lastPC, instruction);
	for (int i = 0; i < REG_NUM; i++)
	{
		fprintf(traceFile, " %08X", registers[i]);
	}

	fprintf(traceFile, "\n");
}
/*
input arguments:
argv and argc are command line arguments are passed to main() in C.
argv is an array of strings, which is the arguments recieved in the command line.
argc (argument count) will be the number of strings pointed to by argv (argument vector).
This will (in practice) be 1 plus the number of arguments, as virtually all implementations will prepend the name of the program to the array (the exe).
the main executes the code it-self.
At the start it opens the file that should be opened, and initialize what needs to be.
After the setup, it enters the while lop which is the assemly code itself (that we get from the instruction from the assembler) while writing to trace,
until the we get an halt operation, or we get to the end of the instructions.
After that we wrap up the simulator by writing to the out files and closing any opened files.
*/

int main(int argc, char* argv[])
{
	read_in_hex(argv[MEMIN], instruction_memory, INSTRUCTION_MEMORY_SIZE);
	traceFile = fopen(argv[TRACE], "w");
	file_opening_check(traceFile, argv[TRACE]);
	int cnt = 0;


	while (!flag_for_Exit) {

		if (irq_enable & irq_status && !interrupted)
		{
			irq_return = PC;
			PC = irq_handler;
			interrupted = 1;
		}
		int lastPC = PC;
		CopyRegisters();
		int currentInstruction = instruction_memory[PC];
		int currentOpCode = (currentInstruction & 0xFF000) >> 12;
		int rd = (currentInstruction & 0xF00) >> 8;
		int rs = (currentInstruction & 0xF0) >> 4;
		int rt = (currentInstruction & 0xF);
		OpcodeMap[currentOpCode](rd, rs, rt);
		Copy_of_registers[1] = Registers[1];              // in order to save the imm value 
		write_to_trace(lastPC, Copy_of_registers, currentInstruction);



		if (PC >= INSTRUCTION_MEMORY_SIZE)
		{
			flag_for_Exit = 1;
		}
	}

	write_in_hex(argv[MEMOUT], memory, DATA_SIZE);
	write_in_hex(argv[REGOUT], Registers, REG_NUM);
	write_to_cycles(argv[CYCLES]);
	fclose(traceFile);

	return 0;
}