#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Define structure for instruction and opcode mapping
typedef struct {
    char instr[5];
    int opcode;
} InstructionMapping;

// Define structure for register mapping
typedef struct {
    char regname[5];
    int regnumber;
} RegisterMapping;

// Define structure for label mapping
typedef struct {
    char labname[50];
    int lablinenum;
} LabelMapping;

// labels mapping
LabelMapping labels[100];

// Instruction and opcode mappings
InstructionMapping instructions[] = {
    {"add", 0},
    {"sub", 1},
    {"mac", 2},
    {"and", 3},
    {"or", 4},
    {"xor", 5},
    {"sll", 6},
    {"sra", 7},
    {"srl", 8},
    {"beq", 9},
    {"bne", 10},
    {"blt", 11},
    {"bgt", 12},
    {"ble", 13},
    {"bge", 14},
    {"jal", 15},
    {"lw", 16},
    {"sw", 17},
    {"reti", 18},
    {"in", 19},
    {"out", 20},
    {"halt", 21},
};

// Register mapping
RegisterMapping registers[] = {
    {"$zero", 0},
    {"$imm1", 1},
    {"$imm2", 2},
    {"$v0", 3},
    {"$a0", 4},
    {"$a1", 5},
    {"$a2", 6},
    {"$t0", 7},
    {"$t1", 8},
    {"$t2", 9},
    {"$s0", 10},
    {"$s1", 11},
    {"$s2", 12},
    {"$gp", 13},
    {"$sp", 14},
    {"$ra", 15},
};


// Function to get opcode for an instruction
int getOpcode(char* instr) {
    for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
        if (strcmp(instructions[i].instr, instr) == 0) {
            return instructions[i].opcode;
        }
    }
    return -1; // Opcode not found
}

// Function to get register number
int getRegisterNumber(char* regname) {
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (strcmp(registers[i].regname, regname) == 0) {
            return registers[i].regnumber;
        }
    }
    return -1; // Register not found
}

int convert_str_to_dec(char* str) {
    if (str[1] == 'x' || str[1] == 'X') {
        char* endptr;       // Pointer to the character after the parsed number
        // Convert hexadecimal string to decimal integer
        long decimalValue = strtol(str, &endptr, 16);
        return decimalValue;
    }
    else return atoi(str);      // atoi convert string to integer
}

int getImmNumber(char* imm) {
    if (isalpha(imm[0])) {
        for (int i = 0; i < sizeof(labels) / sizeof(labels[0]); i++) {
            if (strcmp(labels[i].labname, imm) == 0) {
                return labels[i].lablinenum;
            }
        }
    }
    else return convert_str_to_dec(imm);
}

// Function to parse and assemble the instruction
void assembleInstruction(char* line, FILE* outputFile) {
    char rd[6], rs[6], rt[6], rm[6], immediate1[51], immediate2[51];
    char instr[5];

    // Ignore text after #
    char* hash_position = strchr(line, '#');
    if (hash_position != NULL) {
        *hash_position = '\0';
    }

    // Tokenize the line
    char* token = strtok(line, ", \t\n");
    if (token != NULL) {
        strcpy(instr, token);  // First member as instr

        // Read subsequent member
        int member_count = 0;
        while ((token = strtok(NULL, ", \t\n")) != NULL && member_count < 6) {
            switch (member_count) {
            case 0: strcpy(rd, token); break;
            case 1: strcpy(rs, token); break;
            case 2: strcpy(rt, token); break;
            case 3: strcpy(rm, token); break;
            case 4: strcpy(immediate1, token); break;
            case 5: strcpy(immediate2, token); break;
            }
            member_count++;
        }
    }

    // Get register, opcode and immediate numbers
    int rdNumber = getRegisterNumber(rd);
    int rsNumber = getRegisterNumber(rs);
    int rtNumber = getRegisterNumber(rt);
    int rmNumber = getRegisterNumber(rm);

    int opcodeInt = getOpcode(instr);

    int lsb12 = 0xFFF; // Set to perform masking to get the 12 LSBs

    int imm1Num = (getImmNumber(immediate1) & lsb12);
    int imm2Num = (getImmNumber(immediate2) & lsb12);

    fprintf(outputFile, "%02X%X%X%X%X%03X%03X\n", opcodeInt, rdNumber, rsNumber, rtNumber, rmNumber, imm1Num, imm2Num);
}

//Handling the case of pseudo-Instruction '.word'
void word_Instruction(char* line, char* address, char* value) {

    int member_count = 0;
    char wordIns[6];
    // Tokenize the line
    char* token = strtok(line, " \t\n");
    if (token != NULL) {
        strcpy(wordIns, token);  // First member in the line
    }
    // Read 2 other members
    while ((token = strtok(NULL, " \t\n")) != NULL && member_count < 2) {
        switch (member_count) {
        case 0: strcpy(address, token); break;
        case 1: strcpy(value, token); break;
        }
        member_count++;
    }
}

int main(int argc, char* argv[]) {
    FILE* assemblyFile = fopen(argv[1], "r");
    FILE* imeminFile = fopen(argv[2], "w");
    FILE* dmeminFile = fopen(argv[3], "w");

    int memory[4096] = { 0 }; // Initialize memory with zeroes

    if (assemblyFile == NULL || imeminFile == NULL || dmeminFile == NULL) {
        printf("Error opening files.\n");
        return -1;
    }

    int numofline = 0, numoflabels = 0, numofcharinline = 0;

    char line[501];

    // A loop that maps all labels
    while (fgets(line, sizeof(line), assemblyFile)) {
        numofcharinline = 0;
        //Ignoring white spaces
        while (line[numofcharinline] == ' ' || line[numofcharinline] == '\t') {
            numofcharinline++;
        }

        // Skip comments and empty lines
        if (line[numofcharinline] == '#' || line[numofcharinline] == '\n') {
            continue;
        }
        else {
            char firstWord[51];
            sscanf(line, "%s", &firstWord);
            // Checking if the line starts with a label
            if (strchr(firstWord, ':') != NULL) {
                size_t word_len = strlen(firstWord);
                // Remove the final char ':'
                firstWord[word_len - 1] = '\0';
                // Adding the label to the list
                strcpy(labels[numoflabels].labname, firstWord);
                labels[numoflabels].lablinenum = numofline;
                numoflabels++;

                numofcharinline = word_len;
                //Ignoring white spaces
                while (line[numofcharinline] == ' ' || line[numofcharinline] == '\t') {
                    numofcharinline++;
                }
                //Checking if there is no instruction on the same line of the label
                if (line[numofcharinline] == '#' || line[numofcharinline] == '\n') continue;
            }
            else if (!strcmp(firstWord, ".word")) continue;            //Not counting lines of pseudo-instruction
            numofline++;
        }
    }

    // Rewind the file pointer to the beginning of the file
    rewind(assemblyFile);

    while (fgets(line, sizeof(line), assemblyFile)) {
        // Skip comments and empty lines
        int i = 0;
        while (line[i] == ' ' || line[i] == '\t') {
            i++;
        }
        if (line[i] == '#' || line[i] == '\n') {
            continue;
        }
        else {
            char firstWord[51];
            sscanf(line, "%s", &firstWord);

            // Checking if the line starts with the pseudo-instruction ".word"
            if (!strcmp(firstWord, ".word")) {
                char address[11], value[11];
                word_Instruction(line, address, value);
                // Update memory
                int addressI = convert_str_to_dec(address);
                int valueI = convert_str_to_dec(value);
                memory[addressI] = valueI;
                continue;
            }

            // Checking if the line starts with a label
            if (strchr(firstWord, ':') != NULL) {
                size_t word_len = strlen(firstWord);
                numofcharinline = word_len;
                //Ignoring white spaces
                while (line[numofcharinline] == ' ' || line[numofcharinline] == '\t') {
                    numofcharinline++;
                }
                //Checking if there is an instruction on the same line of the label
                if (line[numofcharinline] == '#' || line[numofcharinline] == '\n') continue;
                else {
                    assembleInstruction(&line[numofcharinline], imeminFile);
                    continue;
                }
            }
        }
        assembleInstruction(line, imeminFile);
    }

    // Write memory content to dmemin.txt
    for (int i = 0; i < 4096; i++) {
        fprintf(dmeminFile, "%08X\n", memory[i]);
    }

    fclose(assemblyFile);
    fclose(imeminFile);
    fclose(dmeminFile);
    printf("Done");
    return 0;
}