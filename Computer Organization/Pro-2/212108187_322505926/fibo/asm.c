#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE 500
#define MEMORY_SIZE 4096
int DMEM[MEMORY_SIZE] = { 0 };

typedef struct {                 //define insturaction as structure
    char opcode[10];
    char rd[5];
    char rs[5];
    char rt[5];
    char imm[25];
}instruction;

typedef struct {                 //define label as structure 
    char name[50];
    int address;

}label;

typedef struct {                //define word as structure
    int address;
    int value;

}word;

int islabel(char imm[20]) {                 ///check if $imm is label 
    int lab = 0;
    char firstCh[2];
    strncpy(firstCh, &imm[0], 1);           //take the first two indexes of the imm
    firstCh[1] = '\0';
    int decimm = atoi(firstCh);
    if (firstCh[0] >= 65 && firstCh[0] <= 122)      // check if its a number or a letter
        lab = 1;                                    //if it's a a letter then its a label address in the imm
    return lab;
}


int dectohex(char imm[20], FILE* fp) {          //function that convert numbers from char form to hexadecimal number and print it to memin.txt
    int decimm = atoi(imm);
    char heximm[6];
    sprintf(heximm, "%05X", decimm);;           // turn the number from dec to a 5 bytes hex string
    fprintf(fp, "%s\n", heximm);                //print the result to the file
    return 0;
}
int dectohexx(char imm[20], FILE* fp) {         //function that convert negative numbers from char to hexadecimal number and print it to memin.txt
    char lastCh[20];
    strncpy(lastCh, &imm[1], 19);
    int decimm = atoi(lastCh);
    char heximm[9];
    char heximm2[6];
    sprintf(heximm, "%05X", -1 * decimm);;
    strncpy(heximm2, &heximm[3], 5);
    fprintf(fp, "%s\n", &heximm[3]);
    return 0;
}

word readword(char line[MAX_LINE]) {    //this function work for the .word instruction
    char* token;
    word wordin;
    char array[3][500];
    token = strtok(line, "\t\n");
    token = strtok(token, " \n");
    for (int i = 0; i < 2; i++) {
        token = strtok(NULL, " \n\t");
        strcpy(array[i], token);            // get the address and the value from the line
    }
    if (strstr(array[0], "0x")) {           // if the address is in hex convert it
        wordin.address = strtol(array[0], NULL, 16);    //get the address from the .word
    }
    else {                                  // if the address is in dec number
        wordin.address = atoi(array[0]);
    }
    wordin.value = atoi(array[1]);          // get the value from the .word
    return wordin;

}
char* indexnumber(char token[MAX_LINE]) {        //function that convert the registers and opcodes to machine codes
    if (strstr(token, "$zero") != NULL)
        return "0";
    if (strstr(token, "$imm") != NULL)
        return "1";
    if (strstr(token, "$v0") != NULL)
        return "2";
    if (strstr(token, "$a0") != NULL)
        return "3";
    if (strstr(token, "$a1") != NULL)
        return "4";
    if (strstr(token, "$a2") != NULL)
        return "5";
    if (strstr(token, "$a3") != NULL)
        return "6";
    if (strstr(token, "$t0") != NULL)
        return "7";
    if (strstr(token, "$t1") != NULL)
        return "8";
    if (strstr(token, "$t2") != NULL)
        return "9";
    if (strstr(token, "$s0") != NULL)
        return "A";
    if (strstr(token, "$s1") != NULL)
        return "B";
    if (strstr(token, "$s2") != NULL)
        return "C";
    if (strstr(token, "$gp") != NULL)
        return "D";
    if (strstr(token, "$sp") != NULL)
        return "E";
    if (strstr(token, "$ra") != NULL)
        return "F";
    if (strstr(token, "add") != NULL)
        return "00";
    if (strstr(token, "sub") != NULL)
        return "01";
    if (strstr(token, "mul") != NULL)
        return "02";
    if (strstr(token, "and") != NULL)
        return "03";
    if (strstr(token, "or") != NULL)
        return "04";
    if (strstr(token, "xor") != NULL)
        return "05";
    if (strstr(token, "sll") != NULL)
        return "06";
    if (strstr(token, "sra") != NULL)
        return "07";
    if (strstr(token, "srl") != NULL)
        return "08";
    if (strstr(token, "beq") != NULL)
        return "09";
    if (strstr(token, "bne") != NULL)
        return "0A";
    if (strstr(token, "blt") != NULL)
        return "0B";
    if (strstr(token, "bgt") != NULL)
        return "0C";
    if (strstr(token, "ble") != NULL)
        return "0D";
    if (strstr(token, "bge") != NULL)
        return "0E";
    if (strstr(token, "jal") != NULL)
        return "0F";
    if (strstr(token, "lw") != NULL)
        return "10";
    if (strstr(token, "sw") != NULL)
        return "11";
    if (strstr(token, "halt") != NULL)
        return "12";
    return "";
}


int readlabels(FILE* fipin, label labels[500]) { //function that read the labels and updates the label's list with labels name and address 
    int labelnum = 0;
    int linenumber = 0;
    char line[MAX_LINE];
    int k, j, count = 0;
    while (!feof(fipin)) {
        int flag = 0;
        fgets(line, MAX_LINE, fipin);
        count++;
        if (strcmp(line, "\n") == 0)
            continue;
        if (line[0] == '#') {
            count = count - 1;
            continue;
        }
        if (strstr(line, ".word") != NULL) { //If line is .word, continue
            count = count - 1;
            continue;
        }
        if (strstr(line, "$imm") != NULL) //If dots are found, this is a label
            count++;
        if (strstr(line, ":") != NULL)
        {
            if (strstr(line, "#") != NULL) // however, ":" can be in a remark. so check for that as well, if so go to another line
                if ((strstr(line, ":")) > (strstr(line, "#"))) {
                    continue;
                }
            count = count - 1;
            k = 0;
            j = 0;
            do {
                if (line[k] != ':') {
                    if (line[k] != '\t')
                        if (line[k] != ' ')
                        {
                            labels[labelnum].name[j] = line[k];
                            j++;
                        }
                    k++;
                }
            } while (line[k] != ':');
            labels[labelnum].name[j] = '\0';
            labels[labelnum].address = count;
            labelnum++;
        }
    }
    fclose(fipin);
    return 0;
}

int readinstructions(FILE* fipin, label labels[500], FILE* fp) { //function that read the insrucions and print to the memin.txt machcodes and the immediate 
    instruction instructions[500];
    char newString[10][10];
    char line[MAX_LINE];
    int wordadd[4096] = { 0 }; //array for the .word in the code
    int instructionnum = 0;
    int linenum = 0; // line number counter
    int finalinst = 0;
    if (fipin == NULL) {
        perror("Error opening file");
        exit(1);
    }
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }
    while (!feof(fipin)) {
        int imed = 0;
        char immediate[20];
        // read a command from the assembler file
        fgets(line, MAX_LINE, fipin);
        if (strcmp(line, "\n") == 0) //If line is blank, continue
            continue;
        if (line[0] == '#') //If line is Remark, continue
            continue;
        if (strstr(line, ".word") != NULL) { //If line is .word, continue
            char linecpy[MAX_LINE];
            strcpy(linecpy, line);
            word wordin = readword(linecpy);
            if (finalinst <= wordin.address) {
                finalinst = wordin.address;    //get the max address of the .word instruction
            }
            wordadd[wordin.address] = wordin.value;  //savw the value of the .word in the address of the array
            continue;
        }
        if (strstr(line, ":") != NULL) { //If dots are found, this is a label
            continue;
        }
        linenum += 1;
        char* token;
        token = strtok(line, "\t");
        token = strtok(token, ", \n\t");
        char machcode[25] = "";
        for (int i = 0; i < 4; i++) {
            strcat(machcode, indexnumber(token));
            token = strtok(NULL, ", \n\t");
        }
        strcpy(immediate, token);
        if (machcode[2] == '1')         //check the machine code for reg 1 the immediate
            imed = 1;
        if (machcode[3] == '1')
            imed = 1;
        if (machcode[4] == '1')
            imed = 1;
        if (imed == 1)
        {
            linenum += 1;
            if (islabel(immediate) == 1) { //check of the imm is a label
                for (int i = 0; i <= 500; i++) {
                    if (!strcmp(immediate, labels[i].name)) {  //compare the name in the imm to the labels we have
                        char value[50];
                        sprintf(value, "%d", labels[i].address); //if we hvae the label in the array print the value of the address
                        fprintf(fp, "%s\n", machcode);
                        dectohex(value, fp);
                    }
                }
            }

            else if (islabel(immediate) == 0) {
                fprintf(fp, "%s\n", machcode);
                char firstCh[2];
                strncpy(firstCh, &immediate[0], 1);
                if (strstr(firstCh, "-") != NULL) {
                    dectohexx(immediate, fp);
                }
                else {
                    dectohex(immediate, fp);
                }
            }
            imed = 0;
        }
        else if (imed == 0)
            fprintf(fp, "%s\n", machcode);
    }
    for (int i = linenum; i <= finalinst; i++) {         //print the values we have in the memory from the .word instruction
        if (wordadd[i] == 0)
            dectohex("0", fp);
        else {
            char value[50];
            sprintf(value, "%d", wordadd[i]);
            dectohex(value, fp);
        }
    }

    return 0;
    fclose(fipin);
    fclose(fp);
}



void main() {
    label labels[500];
    FILE* fipin = fopen("fib.asm", "r");
    FILE* fipin1 = fopen("fib.asm", "r");
    FILE* fp = fopen("memin.txt", "w");
    readlabels(fipin1, labels);
    readinstructions(fipin, labels, fp);
}
