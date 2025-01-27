#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// a like can have 500 chars according to instructions
#define MAX_LINE 500

// first part - utility

// Label struct and related functions

// the label linked list will help us store info about the labels. it will be built at the first iteration and used and the second. then destroyed
typedef struct Label
{
    // name will store the name of the label
    char name[50];
    // location will store the line number and will be the immediate value in related jump and branch commands
    int location;
    // a pointer to the next label
    struct Label* next;
} label;

// this function creates a label from the given name and location
label* create_label(char name[50], int location)
{
    // allocate memory for the label and create a pointer to it
    label* new_label = (label*)malloc(sizeof(label));
    // if allocation successful. insert data to label
    if (new_label != NULL) {
        // use strcpy to insert a string
        strcpy(new_label->name,name);
        // the other insertions are easy
        new_label->location = location;
        new_label->next = NULL;
    }
    return new_label;
}

// adds a label to the front of an existing label list with given name and location.
// will be used in the first iteration
label* add(label* head, char name[50], int location) {
    // build the label and check for success. otherwise return a null
    label* new_label = create_label(name, location);    if (new_label == NULL)
        return NULL;
    // the next pointer will point to the original head
    new_label->next = head;
    return new_label;
}
// this will scan the label list "head" and look for "name". it will return it's location.
// this will be used in the second iteration
int find(label* head, char name[50]) {
    // current - the current label's name
    char current[50];
    // start with the head
    strcpy(current, head->name);
    // strcmp returns 0 if names are equal
    while (strcmp(current, name) != 0) {
        // go to next label
        head = head->next;
        // in case not found - return -1
        if (head == NULL) {
            return -1;
        }
        // update name to current name
        strcpy(current, head->name);
    }
    // return the current's location
    return head->location;
}

// destroys the list and frees the memory
void destroy(label* head)
{
    // temp - a pointer to a label we are going to destroy after updating head
    label* temp;
    // all the way to the end
    while (head != NULL)
    {
        // temp gets the current node while head advances
        temp = head;
        head = head->next;
        // we destroy temp and free the memory
        free(temp);
    }
}

// MemoryLine struct and related functions

// this struct will be used to save the memory lines
typedef struct MemoryLine {
    // the opcode of the command
    char opcode[6];
    // the registers
    char rd[6];
    char rs[6];
    char rt[6];
    // immediate value
    char imm[50];
    // position of character in line
    int pos;
    // memory will also be a linked list to support infinite length programs
    struct MemoryLine* next;
}MemoryLine;

// creates new memory line with no "next"
MemoryLine* create_line(char opcode[6], char rd[6], char rs[6], char rt[6], char imm[50], int pos) {
    // allocate memory for the label and create a pointer to it
    MemoryLine* new_line  = (MemoryLine*)malloc(sizeof(MemoryLine));
    // if allocation successful. insert data to label
    if (new_line != NULL) {
        // use strcpy to insert the strings
        strcpy(new_line->opcode, opcode);
        strcpy(new_line->rd, rd);
        strcpy(new_line->rs, rs);
        strcpy(new_line->rt, rt);
        strcpy(new_line->imm, imm);
        new_line->pos = pos;
        // no next defined
        new_line->next = NULL;
    }
    return new_line;
}

// adds line to memory structure. this line will be added to the end to let the writing run it like an array
MemoryLine* add_line(MemoryLine* head, char opcode[6], char rd[6], char rs[6], char rt[6], char imm[50], int pos)
{
    // the last line as for now
    MemoryLine* tail;
    // create a line
    MemoryLine* new_line = create_line(opcode, rd, rs, rt, imm, pos);
    // if the new line is null. do nothing
    if (new_line == NULL)
        return NULL;
    // and return the current if no head supplied
    if (head == NULL)
        return new_line;
    // get the "tail" to the end of the list
    tail = head;
    while (tail->next != NULL)
        tail = tail->next;
    // add the new line
    tail->next = new_line;
    // return updated memory
    return head;
}

// destroy the memory line list and free the memory the assembler used
void destroy_memLine(MemoryLine* head) {
    // temp - a pointer to a line we are going to destroy after updating head
    label* temp;
    // all the way to the end
    while (head != NULL)
    {
        // temp gets the current node while head advances
        temp = head;
        head = head->next;
        // we destroy temp and free the memory
        free(temp);
    }
}

// get the memory line at position. can return null if does not exist
MemoryLine* getAtPos(MemoryLine* head, int pos) {
    // go until you find
    while (head != NULL && head->pos != pos)
        head = head->next;    return head;
}

// Memory struct and related functions. it is used so the first iteration can return two values.
typedef struct Memory
{
    // head of memory line list
    MemoryLine* head;
    // position of last
    int last;
}Memory;

// destroys the memory struct after use
void destroy_mem(Memory* mem) {
    // destroy the memory's line list
    destroy_memLine(mem->head);
    // free the memory object's own memory
    free(mem);
}

// part 2 - the iterations over the files

// the code of the first iteration. goes trough the file row by row and looks for labels, then adds them to the label list
label* createLabelList(FILE *asembl) {
    // the code row's index. where the PC will go after reading the label
    int rowIndex = 0;
    // line the current line being read, tav1 is the first char and i used to check for remarks
    char line[MAX_LINE], tav1;
    // tav - current char when reading label name
    char tav;
    // will contain the name of the label once iteration is complete
    char lable_line[50];
    // the dots are used to say "this is a label"
    char dots[50];
    strcpy(dots, ":");
    // k is the char index for label name read, j is the index in the label name string we are building
    int k, j;
    // option will determine if it's a label only line or a label + command line
    int option;
    // counter will be the line number in the new hexadecimal code. it will go up when a line that gets translated is found
    int counter = 0;
    // the label list's head
    label* head = NULL;
    // go all the way trough the file
    while (!feof(asembl)) {
        // read a command from the assembler file
        fgets(line, MAX_LINE, asembl);
        // reset option
        option = 0;
        if (strcmp(line, "\n") == 0) //If line is blank, continue
            continue;
        tav1 = line[0];
        if (tav1 == '#') //If line is Remark, continue
            continue;
        if (strstr(line, ".word") != NULL) //If line is .word, continue
            continue;
        if (strstr(line, dots) != NULL) //If dots are found, this is a label
        {
            if (strstr(line, "#") != NULL) // however, ":" can be in a remark. so check for that as well, if so go to another line
                if ((strstr(line, dots)) > (strstr(line, "#")))
                    continue;
            //Read the label name, first reset indexes
            k = 0;
            j = 0;
            do {
                // get current char
                tav = line[k];
                // two dots is where is ends to skip
                if (tav != ':') {
                    // don't read tabs and spaces
                    if (tav != '\t')
                        if (tav != ' ')
                        {
                            // grab the read char and put it in name string
                            lable_line[j] = tav;
                            // increment name string index
                            j++;
                        }
                    // increment reading index
                    k++;
                }
            } while (tav != ':');
            // label name is null terminated
            lable_line[j] = '\0';
            k++; // Check if the line is lable line only by seeing if there are only spaces and tabs till the end
            while ((line[k] == ' ') || (line[k] == '\t'))
                k++;
            // option is 1 on label only line, otherwise 0
            if ((line[k] == '\n') || (line[k] == '#'))
                option = 1;
            // finally we add the label to label list
            head = add(head, lable_line, counter);
            if (option == 1) { // Only label line - add label and decrement counter
                counter = counter - 1;
            }
        }
        k = 0; // Check if the current line is space line using k - most commands in fib.asm and our files start with a tab or a space
        if ((line[k] == '\t') || (line[k] == ' '))
            k++;
        if (line[k] == '\n')
            continue;
        // increment hexa file line counter
        counter++;
    }
    // return the list
    return head;
}

// the second iteration. returns the memory list
Memory* SecoundRun(FILE* file) {
    // k is the index of the current char being read
    int k = 0, i = 0, s = 0, pos1 = 0;
    // pos is the address in .word commands, as an int
    int pos = 0;
    // j saves the index in the word for the copy process
    int j = 0;
    // line will house the current line. option, rd, rs, rt and imm are the command's values
    char line[MAX_LINE], tav, option[6], rd[6], rs[6], rt[6], imm[50];
    // wordP houses the address and wordN houses the data in case of a word command
    char wordP[15], wordN[15];
    // used to detect labels. because something might be past them
    char *dots = ":";
    // used to detect the special ".word" command
    char wo = ".word";
    // the Memory list's head
    MemoryLine* head = NULL;
    // tav1 will save the first character of the line
    char tav1;
    // the loop reads the file line by line. and upon reaching null it stops as that's where the file ends
    while ((fgets(line, MAX_LINE, file)) != NULL) // To prevent last line reading twice
    {
        if (strcmp(line, "\n") == 0) ///////// in case of a Blank line, go
            continue;
        // get first line
        tav1 = line[0];
        if (tav1 == '#')  ///////// in case of a Remark line, go
            continue;
        if (strstr(line, wo) != NULL) ///////// in case of the special .word order
        {
            // reset k index
            k = 0;
            // go past all the spaces
            while (line[k] != ' ')
                k++;
            k++;

            j = 0; //Copy Address. first reset j then copy char char until the next space
            while (line[k] != ' ')
            {
                wordP[j] = line[k];
                j++;
                k++;
            }
            // terminate string with null and increment end to next char
            wordP[j] = '\0';
            k++;

            j = 0; //Copy Data. using the same way.
            while (line[k] != ' ')
            {
                // but detect an end of line string because after the data there can be a line end
                if (line[k] == '\n')
                    break;
                wordN[j] = line[k];
                j++;
                k++;
            }
            wordN[j] = '\0';

            if (wordP[0] == '0') //  change Address int. the if block considers an hexadecimal input
            {
                if (wordP[1] == 'x')
                    pos = strtol(wordP, NULL, 16);
            }
            else // and the else blocks considers a decimal input
            {
                pos = atoi(wordP);
            }

            // now. we will save the command in the memory list. NONO will be used as an indicator when writing to turn the command into a .word
            strcpy(option, "NONO");
            strcpy(rd, "NONO");
            strcpy(rs, "NONO");
            strcpy(rt, "NONO");
            // immediate - data
            strcpy(imm, wordN);

            // save line to line list
            add_line(head, option, rd, rs, rt, imm, pos);

            // update the location of the end of the memory
            if (pos > pos1) 
                pos1 = pos;
        }
    }
}

//gets memory head and file indicator. prints the memory into file
void PrintDataToFile(Memory* mem, FILE *ftw)
{
	int i = 0, num=0;
	while (mem->head!=NULL && i<=mem->head->pos)
	{
		{if (strcmp(getAtPos(mem->head,i)->opcode, "add") == 0) // Printing Opcode
			fprintf(ftw, "0");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "sub") == 0)
			fprintf(ftw, "1");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "and") == 0)
			fprintf(ftw, "2");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "or") == 0)
			fprintf(ftw, "3");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "sll") == 0)
			fprintf(ftw, "4");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "sra") == 0)
			fprintf(ftw, "5");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "limm") == 0)
			fprintf(ftw, "6");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "beq") == 0)
			fprintf(ftw, "7");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "bgt") == 0)
			fprintf(ftw, "8");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "ble") == 0)
			fprintf(ftw, "9");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "bne") == 0)
			fprintf(ftw, "A");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "jal") == 0)
			fprintf(ftw, "B");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "lw") == 0)
			fprintf(ftw, "C");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "sw") == 0)
			fprintf(ftw, "D");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "jr") == 0)
			fprintf(ftw, "E");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "halt") == 0)
			fprintf(ftw, "F");
		else if(strcmp(getAtPos(mem->head,i)->opcode, "NONO") == 0) {
			if (strcmp(getAtPos(mem->head, i)->rd, "NONO") == 0) {
				if (strcmp(getAtPos(mem->head, i)->.rs, "NONO") == 0) {
					if (strcmp(getAtPos(mem->head, i)->rt, "NONO") == 0) {
						if ((getAtPos(mem->head, i)->imm[0] == '0') && (getAtPos(mem->head, i)->imm[1] == 'x')) //Check if immidiate in hex
						{
							num = strtol(getAtPos(mem->head, i)->imm, NULL, 16);
						}
						else//Imiddiate is decimal
						{
							num = atoi(getAtPos(mem->head, i)->imm);
						}

						//Print immidiate in hex
						fprintf(ftw, "%08X", num);
					}
				}
			}
		}
		else
			fprintf(ftw, "0"); }


		{if (strcmp(getAtPos(mem->head,i)->rd, "$zero") == 0) // Printing Rd
			fprintf(ftw, "0");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$at") == 0)
			fprintf(ftw, "1");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$v0") == 0)
			fprintf(ftw, "2");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$a0") == 0)
			fprintf(ftw, "3");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$a1") == 0)
			fprintf(ftw, "4");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$t0") == 0)
			fprintf(ftw, "5");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$t1") == 0)
			fprintf(ftw, "6");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$t2") == 0)
			fprintf(ftw, "7");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$t3") == 0)
			fprintf(ftw, "8");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$s0") == 0)
			fprintf(ftw, "9");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$s1") == 0)
			fprintf(ftw, "A");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$s2") == 0)
			fprintf(ftw, "B");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$gp") == 0)
			fprintf(ftw, "C");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$sp") == 0)
			fprintf(ftw, "D");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$fp") == 0)
			fprintf(ftw, "E");
		else if(strcmp(getAtPos(mem->head,i)->rd, "$ra") == 0)
			fprintf(ftw, "F");
		else if(strcmp(getAtPos(mem->head,i)->rd, "NONO") == 0)
			num = 0;
		else
			fprintf(ftw, "0"); }


		{if (strcmp(getAtPos(mem->head,i)->rs, "$zero") == 0) // Printing Rs
			fprintf(ftw, "0");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$at") == 0)
			fprintf(ftw, "1");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$v0") == 0)
			fprintf(ftw, "2");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$a0") == 0)
			fprintf(ftw, "3");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$a1") == 0)
			fprintf(ftw, "4");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$t0") == 0)
			fprintf(ftw, "5");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$t1") == 0)
			fprintf(ftw, "6");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$t2") == 0)
			fprintf(ftw, "7");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$t3") == 0)
			fprintf(ftw, "8");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$s0") == 0)
			fprintf(ftw, "9");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$s1") == 0)
			fprintf(ftw, "A");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$s2") == 0)
			fprintf(ftw, "B");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$gp") == 0)
			fprintf(ftw, "C");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$sp") == 0)
			fprintf(ftw, "D");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$fp") == 0)
			fprintf(ftw, "E");
		else if(strcmp(getAtPos(mem->head,i)->rs, "$ra") == 0)
			fprintf(ftw, "F");
		else if(strcmp(getAtPos(mem->head,i)->rs, "NONO") == 0)
			num = 0;
		else
			fprintf(ftw, "0"); }


		{if (strcmp(getAtPos(mem->head,i)->rt, "$zero") == 0) // Printing Rt
			fprintf(ftw, "0");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$at") == 0)
			fprintf(ftw, "1");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$v0") == 0)
			fprintf(ftw, "2");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$a0") == 0)
			fprintf(ftw, "3");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$a1") == 0)
			fprintf(ftw, "4");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$t0") == 0)
			fprintf(ftw, "5");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$t1") == 0)
			fprintf(ftw, "6");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$t2") == 0)
			fprintf(ftw, "7");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$t3") == 0)
			fprintf(ftw, "8");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$s0") == 0)
			fprintf(ftw, "9");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$s1") == 0)
			fprintf(ftw, "A");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$s2") == 0)
			fprintf(ftw, "B");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$gp") == 0)
			fprintf(ftw, "C");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$sp") == 0)
			fprintf(ftw, "D");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$fp") == 0)
			fprintf(ftw, "E");
		else if(strcmp(getAtPos(mem->head,i)->rt, "$ra") == 0)
			fprintf(ftw, "F");
		else if(strcmp(getAtPos(mem->head,i)->rt, "NONO") == 0)
			num = 0;
		else
			fprintf(ftw, "0"); }

		if (strcmp(getAtPos(mem->head,i)->opcode, "NONO") != 0) //If order is not .word, print num
		{
			if ((getAtPos(mem->head,i)->imm[0] == '0') && (getAtPos(mem->head,i)->imm[1] == 'x')) //Check if immidiate in hex
			{
				num = strtol(getAtPos(mem->head,i)->imm, NULL, 16);
			}
			else//Imiddiate is decimal
			{
				num = atoi(getAtPos(mem->head,i)->imm);
			}

			//Print immidiate in hex
			fprintf(ftw, "%04X", num);
		}

		if (i < pos) //Print \n except the last line
			fprintf(ftw, "\n");

		i++;
	}
}


// part 3 - the main function

// the main takes two arguments, the input file and the output file. indexes start with 1 because argv[0] is the program itself
int main(int arc, char* argv[]) {
    // open the input file. doing so in the main function will allow us to have infinite length file names
    // why i call it "asembl"? because of what it is
    FILE *asembl = fopen(argv[1], "r");
    // leave if null file is supplied
    if (asembl == NULL) {
        exit(1);
    }
    // the first iteration, locate the labels and write thier locations to the linked list
    label* labels = createLabelList(asembl);
    // close the file from the first iteration
    fclose(asembl);
    // and reopen it for the second
    FILE* asembl = fopen(argv[1], "r");
    // another null check in case something happend
    if (asembl == NULL) {
        exit(1);
    }
    // start the second iteration
    Memory *memory = SecondRun(asembl);
	
	// print the data to file
	PrintDataToFile(memory, asembl);

    fclose(asembl);
    // free the memory taken by the label list
    destroy(labels);
}
