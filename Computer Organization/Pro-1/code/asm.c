#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEM_SIZE 4096
#define LINE_LEN 300
#define LABEL_LEN 50
#define LEN_REG_STR 6
#define LEN_OPCODE_STR 6
#define PRAM_NUM 5

//  structs
typedef struct instruction
{

	int data_word;
	int valid_word;
	int inst_vs_imm;
	char opcode[LEN_OPCODE_STR];
	char rd[LEN_REG_STR];
	char rs[LEN_REG_STR];
	char rt[LEN_REG_STR];
	char imm[LABEL_LEN];
	int PC;
} instruction;

typedef struct Label
{
	char name[50];
	int ADDREESS;
} label;

// Global variables
FILE* asm_code = NULL;
FILE* memin_file = NULL;
label* our_labels = NULL;
unsigned long** array_hex_inst = NULL;

// Functions

int ASMCODETOIN(instruction inst_arr[], int* max_address, int* PC, int* num_of_labels);
/*it scans the asm code and updates the instruction array get the max address of the data and get the max pc so we can know which is bigger
return 1 in succes and 0 in fail*/

int AddWord(char line[], instruction inst_arr[], int* max_address);
/* this function takes a line and if a word line it upadte
the word array and in the end it update the max address succes return 1 0 otherwise*/

int INSERETINST(char line[], instruction inst_arr[], int* PC);
/*we get a line and update the array of instruction and then we update the vaule of pc if the function fails it returns 0
other wise 1*/

int LABELED_LINES(char line[], int* num_of_labels, int PC);
/*the function takes a labeled line check if its in our labels array if yes it returns 0 otherwise
we update our labels and the new label to it retrun -1 if it fails otherwise 1*/

int IS_IT_EMPTY(const char line[]);
/*check if the line is empty if yes it return 1 otherwise 0 */

void DEL_SPACES(char str[]);
/*it delete the white spaces*/

unsigned long STRNUM_TO_HEX(char num[]);
/*we take a str that represnt a nubmer and return the the number in hex */

int ALLOC_HEX_ARR(int max_PC, int max_address);
/*allocating memory array_hex_inst and the array_hex_word if the allocating success it returns 1 */

int CONV_REG_TO_NUM(const char reg[]);
/*it takes a str the represnt the name of a reg and return its nummber ,returns -1 if the reg name is not found*/

int CONV_OP_TO_NUM(const char Op[]);
/*it takes a str the represnt the name of a opcode and return its nummber ,returns -1 if the opcode name is not found*/

int FIND_LABEL_ADDR(char label[], int num_of_labels);
/*it takes a a label and search for it in our labels array and retrun the address if we didnt found it it returns -1 */

unsigned long INTR_TO_HEX(instruction inst, int num_of_labels, int PC);
/*give the hex vaule of an instruc in fail return -1 */

int INST_TO_HEX_ARR(instruction inst_arr[], int max_PC, int num_of_labels, int max_address);
/* puts instructions on hex array returns 1 in success
, 0 in fail */

void PRINTOUT(int max_PC, int max_address);
/* The function print all the hex instructions to the memin output file*/

void FREE_MEM(int max_PC, int max_address);
/* close files and free allocated memory */

void INIT_WORD_ARR(instruction* arr);

// main
int main(int argc, char* argv[])
{

	instruction inst_arr[MEM_SIZE];

	int max_address = 0;
	int max_PC = 0;
	int num_of_labels = 0;

	// bulding the labels array
	our_labels = malloc(sizeof(label));
	if (NULL == our_labels)
	{
		printf("Error in allocate\n");
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	// Initialization the word_arr to unvaild
	INIT_WORD_ARR(inst_arr);

	// Open files
	asm_code = fopen(argv[1], "r");
	if (NULL == asm_code)
	{
		printf("Error open files!\n");
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	memin_file = fopen(argv[2], "w");
	if (NULL == memin_file)
	{
		printf("Error open files!\n");
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	// put the data of the asm_code into two arrays of structs
	if (!ASMCODETOIN(inst_arr, &max_address, &max_PC, &num_of_labels))
	{
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	// Initialize hex array of instructions and hex array of data
	// unsigned long  array_hex_inst[max_PC];
	// unsigned long array_hex_word[max_address];
	int len = max_PC;
	if (len < max_address)
		len = max_address + 1;

	array_hex_inst = malloc(len * sizeof(unsigned long*));
	if (NULL == array_hex_inst)
	{
		printf("null pointer!\n");
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	if (!ALLOC_HEX_ARR(max_PC, max_address))
	{
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	// Put the instructions and data in hex array
	if (!INST_TO_HEX_ARR(inst_arr, max_PC, num_of_labels, max_address))
	{
		FREE_MEM(max_PC, max_address);
		exit(-1);
	}

	// Write data and  instructions in hex to the output files
	PRINTOUT(max_PC, max_address);

	// Close open files and free all the  memory at the end of the code
	FREE_MEM(max_PC, max_address);
	return 0;
}

int ASMCODETOIN(instruction inst_arr[], int* max_address, int* PC, int* num_of_labels)
{
	char line[LINE_LEN];
	int is_labeled = 0;

	// we get every line in the file
	while (!feof(asm_code) && fgets(line, LINE_LEN, asm_code))            // read every single line in the file 
	{
		if (strstr(line, "#") != NULL)		 // we have a note
			strcpy(line, strtok(line, "#")); // Remove the note

		// we check if the line is labeld if yes update our labels array
		is_labeled = LABELED_LINES(line, num_of_labels, *PC);
		if (is_labeled == -1)
		{
			printf("Error in LABELED_LINES function");
			return 0;
		}

		if (IS_IT_EMPTY(line))
		{
			continue;
		}

		if (strstr(line, ".word") != NULL)
		{ // check if its a word
			if (!AddWord(line, inst_arr, max_address))
			{
				printf("Error in AddWord function");
				return 0;
			}
		}
		else
		{ // its an instruction
			if (!INSERETINST(line, inst_arr, PC))
			{
				printf("Error in INSERETINST function");
				return 0;
			}
		}
	}
	return 1;
}

int AddWord(char line[], instruction inst_arr[], int* max_address)
{
	char* token;
	unsigned long data, address;
	strtok(line, " \t\r\n\f");		   // delete ".word"
	token = strtok(NULL, " \t\r\n\f"); // take the address
	if (token == NULL)
	{
		printf("Error\n");
		return 0;
	}
	address = STRNUM_TO_HEX(token);
	token = strtok(NULL, " \t\r\n\f"); // take the data
	if (token == NULL)
	{
		printf("NO DATA\n");
		return 0;
	}
	data = STRNUM_TO_HEX(token);
	if (address < MEM_SIZE && address >= 0)
	{
		if (*max_address < address)
			*max_address = address;

		inst_arr[address].valid_word = 1;
		inst_arr[address].data_word = data;
	}
	else
	{
		printf("invalid address\n");
		return 0;
	}
	return 1;
}

int INSERETINST(char line[], instruction inst_arr[], int* PC)
{
	int there_is_imm = 0;
	char* args[PRAM_NUM];
	int i = 0;
	args[i] = strtok(line, " \t\r\n\f,");  // if we find any one of these values in the array down line 
	if (args[i] == NULL)
	{
		printf("Error in inst %d, Missing %d param\n", PC[0], i);
		return 0;
	}
	for (i = 1; i < PRAM_NUM; i++)
	{
		args[i] = strtok(NULL, " \t\r\n\f,");
		if (args[i] == NULL)
		{
			printf("Error in inst %d, Missing %d param\n", PC[0], i);
			return 0;
		}
		if (strcmp(args[i], "$imm") == 0)
			there_is_imm = 1;

	}

	// update the inst array with the right parm
	inst_arr[*PC].inst_vs_imm = 0;
	strcpy(inst_arr[*PC].opcode, args[0]);
	strcpy(inst_arr[*PC].rd, args[1]);
	strcpy(inst_arr[*PC].rs, args[2]);
	strcpy(inst_arr[*PC].rt, args[3]);
	inst_arr[*PC].PC = *PC;

	if (there_is_imm)                 // checking if there is an i format in order make the ordered changes in pc 
	{
		*PC += 1;
		inst_arr[*PC].inst_vs_imm = 1;  //0 for r format and 1 for i format 
		strcpy(inst_arr[*PC].imm, args[4]);
		inst_arr[*PC].PC = *PC;
	}

	*PC += 1;
	return 1;
}

int LABELED_LINES(char line[], int* num_of_labels, int PC)
{
	char label_name[LABEL_LEN];
	if (strstr(line, ":") != NULL && ((strstr(line, ":") < strstr(line, "#")) || (strstr(line, "#") == NULL)))
	{ // Label
		// delete ":" and the spaces to get the label
		strcpy(label_name, strtok(line, ":"));
		strcpy(line, (strtok(NULL, "")));
		DEL_SPACES(label_name);

		our_labels = realloc(our_labels, (*num_of_labels + 1) * sizeof(label));
		if (NULL == our_labels)
		{
			printf("Error in allocate\n");
			return -1;
		}
		// update the right vaules in our array
		strcpy(our_labels[*num_of_labels].name, label_name);
		our_labels[*num_of_labels].ADDREESS = PC;
		*num_of_labels += 1;
		return 1;
	}
	return 0; // its not label
}

int IS_IT_EMPTY(const char line[])
{
	char cpy_line[LINE_LEN];
	strcpy(cpy_line, line);
	DEL_SPACES(cpy_line);
	if (strcmp(cpy_line, "\n") == 0)
		return 1;
	return 0;
}

void DEL_SPACES(char str[])
{
	int i = 0, j = 0;
	while (str[i])
	{
		if (str[i] != ' ' && str[i] != '\t')
			str[j++] = str[i];
		i++;
	}
	str[j] = '\0';
}

unsigned long STRNUM_TO_HEX(char num[])
{
	unsigned long hex_num;
	if (strstr(num, "0x") != NULL || strstr(num, "0X") != NULL) // 0x format for hex num   
		hex_num = strtol(num, NULL, 16);
	else
		hex_num = strtol(num, NULL, 10); // dec num
	return hex_num;
}

void INIT_WORD_ARR(instruction* arr)
{

	for (int i = 0; i < MEM_SIZE; i++)
	{

		arr[i].valid_word = 0;
	}
}

int ALLOC_HEX_ARR(int max_PC, int max_address)
{
	int len = max_PC;
	if (len < max_address)
		len = max_address + 1;

	for (int i = 0; i < len; i++)
	{
		array_hex_inst[i] = malloc(sizeof(unsigned long));
		if (NULL == array_hex_inst[i])
		{
			printf("null pointer!\n");
			return 0;
		}
	}

	return 1;
}

int CONV_REG_TO_NUM(const char reg_name[])             // giving every register the suitable value for him 
{
	if (reg_name == NULL)
	{
		return -1;
	}
	if (strcmp(reg_name, "$zero") == 0)
		return 0;
	else if (strcmp(reg_name, "$imm") == 0)
		return 1;
	else if (strcmp(reg_name, "$v0") == 0)
		return 2;
	else if (strcmp(reg_name, "$a0") == 0)
		return 3;
	else if (strcmp(reg_name, "$a1") == 0)
		return 4;
	else if (strcmp(reg_name, "$a2") == 0)
		return 5;
	else if (strcmp(reg_name, "$a3") == 0)
		return 6;
	else if (strcmp(reg_name, "$t0") == 0)
		return 7;
	else if (strcmp(reg_name, "$t1") == 0)
		return 8;
	else if (strcmp(reg_name, "$t2") == 0)
		return 9;
	else if (strcmp(reg_name, "$s0") == 0)
		return 10;
	else if (strcmp(reg_name, "$s1") == 0)
		return 11;
	else if (strcmp(reg_name, "$s2") == 0)
		return 12;
	else if (strcmp(reg_name, "$gp") == 0)
		return 13;
	else if (strcmp(reg_name, "$sp") == 0)
		return 14;
	else if (strcmp(reg_name, "$ra") == 0)
		return 15;

	else
	{
		printf("Error, no such register: %s\n ", reg_name);
		return -1;
	}
}

int CONV_OP_TO_NUM(const char opcode_name[])            // giving the opcode the suitable value for it 
{
	if (opcode_name == NULL)
	{
		return -1;
	}
	if (strcmp(opcode_name, "add") == 0)
		return 0;
	else if (strcmp(opcode_name, "sub") == 0)
		return 1;
	else if (strcmp(opcode_name, "mul") == 0)
		return 2;
	else if (strcmp(opcode_name, "and") == 0)
		return 3;
	else if (strcmp(opcode_name, "or") == 0)
		return 4;
	else if (strcmp(opcode_name, "xor") == 0)
		return 5;
	else if (strcmp(opcode_name, "sll") == 0)
		return 6;
	else if (strcmp(opcode_name, "sra") == 0)
		return 7;
	else if (strcmp(opcode_name, "srl") == 0)
		return 8;
	else if (strcmp(opcode_name, "beq") == 0)
		return 9;
	else if (strcmp(opcode_name, "bne") == 0)
		return 10;
	else if (strcmp(opcode_name, "blt") == 0)
		return 11;
	else if (strcmp(opcode_name, "bgt") == 0)
		return 12;
	else if (strcmp(opcode_name, "ble") == 0)
		return 13;
	else if (strcmp(opcode_name, "bge") == 0)
		return 14;
	else if (strcmp(opcode_name, "jal") == 0)
		return 15;
	else if (strcmp(opcode_name, "lw") == 0)
		return 16;
	else if (strcmp(opcode_name, "sw") == 0)
		return 17;
	else if (strcmp(opcode_name, "reti") == 0)
		return 18;
	else if (strcmp(opcode_name, "in") == 0)
		return 19;
	else if (strcmp(opcode_name, "out") == 0)
		return 20;
	else if (strcmp(opcode_name, "halt") == 0)
		return 21;

	else
	{
		printf("Error, no such opcode %s\n", opcode_name);
		return -1;
	}
}

int FIND_LABEL_ADDR(char label[], int num_of_labels)
{
	int i;
	for (i = 0; i < num_of_labels; i++)
	{
		if (strcmp(label, our_labels[i].name) == 0)
			break; // found a label
	}
	if (i == num_of_labels)
		return -1; // the label is not in our labels
	return our_labels[i].ADDREESS;
}

unsigned long INTR_TO_HEX(instruction inst, int num_of_labels, int PC)
{
	unsigned long opcode, rd, rs, rt, imm;

	if (inst.inst_vs_imm == 0)
	{
		opcode = CONV_OP_TO_NUM(inst.opcode);
		rd = CONV_REG_TO_NUM(inst.rd);
		rs = CONV_REG_TO_NUM(inst.rs);
		rt = CONV_REG_TO_NUM(inst.rt);

		// WRONG name of opcode or regester
		if (opcode == -1 || rd == -1 || rs == -1 || rt == -1)
		{
			// error
			printf("Error ,wrong name of opcode or regester\n");
			return -1;
		}
		// this is in order to organize the array for the registers 
		rt = (rt & 0xF);
		rs = (rs & 0xF) << 4;
		rd = (rd & 0xF) << 8;
		opcode = (opcode & 0xFF) << 12;
		return opcode + rd + rs + rt; // ordering the values in one line
	}

	if (inst.inst_vs_imm == 1)
	{

		if ((inst.imm[0] >= 'a' && inst.imm[0] <= 'z') || (inst.imm[0] >= 'A' && inst.imm[0] <= 'Z'))
		{
			imm = FIND_LABEL_ADDR(inst.imm, num_of_labels);
			if (imm == -1 || imm >= MEM_SIZE)
			{
				// error
				printf("Error \n");
				return -1;
			}
		}
		else
		{
			imm = STRNUM_TO_HEX(inst.imm);
		}

		// The result
		imm = imm & 0xFFFFF;
	}
	return imm;
}

int INST_TO_HEX_ARR(instruction inst_arr[], int max_PC, int num_of_labels, int max_address)
{
	unsigned long hex_inst = 0;
	for (int i = 0; i < max_PC; i++)
	{
		//  str instruction to hex

		hex_inst = INTR_TO_HEX(inst_arr[i], num_of_labels, i);
		if (hex_inst == -1)
			return 0;

		*array_hex_inst[i] = hex_inst;
	}

	unsigned long hex_word = 0;
	for (int i = max_PC; i <= max_address; i++)
	{
		if (inst_arr[i].valid_word)
		{ // found data
			hex_word = inst_arr[i].data_word;
		}
		else
		{ //  no data
			hex_word = 0;
		}

		*array_hex_inst[i] = hex_word;
	}

	return 1;
}

void PRINTOUT(int max_PC, int max_address)              // printing the finall values in the file 
{
	for (int i = 0; i < max_PC; i++)
	{
		fprintf(memin_file, "%05lX\n", *array_hex_inst[i]);
	}
	if (max_address == 0)
		return;
	for (int i = max_PC; i <= max_address; i++)
	{
		fprintf(memin_file, "%05lX\n", *array_hex_inst[i]);
	}
}

void FREE_MEM(int max_PC, int max_address)
{
	if (asm_code != NULL)
	{
		fclose(asm_code);
	}
	if (memin_file != NULL)
	{
		fclose(memin_file);
	}

	if (our_labels != NULL)
		free(our_labels);
	if (array_hex_inst != NULL)
	{
		for (int i = 0; i < max_PC; i++)
			if (array_hex_inst[i] != NULL)
				free(array_hex_inst[i]);
		free(array_hex_inst);
	}
}