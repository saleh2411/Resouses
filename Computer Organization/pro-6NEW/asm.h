#ifndef ASM_H
#define ASM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

#define LABEL_MAX_LEN 50
#define MAX_LINE_LEN 500
#define MAX_LABELS 4096 //Max Labels
#define MAX_PROGRAM_LINES 4096 //Max Lines
#define MAX_DATA_MEMORY 4096 //Max Lines

typedef struct Label
{
    char name[LABEL_MAX_LEN];
    int address; // Label's line in the program
}label;
//void strconcat(char* dest, char* source1, char* source2);

/********** Init functions ***************/
void init_memin_array();

void init_memin_array_int();

void init_dmemin_array();

/********** first run on file functions ***************/
int get_labels(FILE* file, label* labels);

int read_label(const char* line/*,int start_index*/, label* lb);

int colon_count(const char* str);


/********** second run on file functions ***************/

void get_all_instructions(FILE* file, label* labels_arr, int num_labels);

int get_instruction(const char* line_str, label* labels_arr, int num_labels);

int word_inst(char* str);

int get_line_num_of_tokens(char* line_str);

char* trimwhitespace(char* str);

void clean_comments(char* str, int length);

int find_index(char** arr, int len, char* val);

int find_index_in_labels(label* labels_arr, int len, char* val);

int imm_to_int(char* imm, label* labels, int labels_num);

int extend_sign(uint32_t reg, uint8_t sign_bit);

/********** Helper functions ***************/

#endif //ASM_H
