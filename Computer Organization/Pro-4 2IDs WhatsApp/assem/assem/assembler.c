#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    char label_name[50];    // The name of the label (up to 50 characters)
    int line;               // The line number of the label in the assembly code
    int text_line;          // The line number of the label in the machine code
} Label;

typedef struct {
    char opcode[6];         // The opcode of the instruction (up to 6 characters)
    char rd[6];             // The destination register (up to 6 characters)
    char rs[6];             // The first source register (up to 6 characters)
    char rt[6];             // The second source register (up to 6 characters)
    char immediate[50];     // The immediate value or label (up to 50 characters)
    int new_line;           // The line number where the instruction is written in the memin (machine code)
    bool set;               // A flag indicating if the opcode is valid
} Format;

typedef struct {
    char opcode[3];         // The hexadecimal representation of the opcode (up to 3 characters)
    char rd;                // The hexadecimal representation of the destination register
    char rs;                // The hexadecimal representation of the first source register
    char rt;                // The hexadecimal representation of the second source register
    char immediate[50];     // The immediate value or label (up to 50 characters)
    bool set;               // A flag indicating if the instruction is valid
} RI_Format;

typedef struct {
    int new_line;           // The line number where the pseudo-instruction is written in the memin 
    char data[6];           // The data of the pseudo-instruction (up to 6 characters)
    bool set;               // A flag indicating if the pseudo-instruction is valid
} pse;


FILE* open_fr(const char* fPath) {  // Open file for read
    FILE* fpoint = fopen(fPath, "r");
    if (fpoint == NULL) {
        perror("Error opening file for reading");
    }
    return fpoint;
}

FILE* open_fw(const char* fPath) {  // Open file for write
    FILE* fpoint = fopen(fPath, "w");
    if (fpoint == NULL) {
        perror("Error opening file for writing");
    }
    return fpoint;
}

int count_lines_in_file(const char* filename) {     // Counts the lines in the file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Could not open file");
        return -1;
    }

    int count = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    fclose(file);
    return count;
}

pse* create_pse_array(size_t size) {    // Allocating memory for size of the array of pesudo instruction
    pse* array = (pse*)malloc(size * sizeof(pse));
    if (array == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    return array;
}

RI_Format* create_ri_format_array(size_t size) {    // Allocating memory for size of the array of RI Formats
    RI_Format* array = (RI_Format*)malloc(size * sizeof(RI_Format));
    if (array == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    return array;
}
void free_pse_array(pse* array) {       // Free the allocated memory of pse
    free(array);
}


void free_ri_format_array(RI_Format* array) {       // Free the allocated memory of RI_Format
    free(array);
}

void toLowerCase(char* str)         // convert to lower case
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
}

const char* decoder_opcode(const char* opcode)      // Decoding opcodes
{
    if (strcmp(opcode, "add") == 0) return "00";
    if (strcmp(opcode, "sub") == 0) return "01";
    if (strcmp(opcode, "mul") == 0) return "02";
    if (strcmp(opcode, "and") == 0) return "03";
    if (strcmp(opcode, "or") == 0)  return "04";
    if (strcmp(opcode, "xor") == 0) return "05";
    if (strcmp(opcode, "sll") == 0) return "06";
    if (strcmp(opcode, "sra") == 0) return "07";
    if (strcmp(opcode, "srl") == 0) return "08";
    if (strcmp(opcode, "beq") == 0) return "09";
    if (strcmp(opcode, "bne") == 0) return "0A";
    if (strcmp(opcode, "blt") == 0) return "0B";
    if (strcmp(opcode, "bgt") == 0) return "0C";
    if (strcmp(opcode, "ble") == 0) return "0D";
    if (strcmp(opcode, "bge") == 0) return "0E";
    if (strcmp(opcode, "jal") == 0) return "0F";
    if (strcmp(opcode, "lw") == 0)  return "10";
    if (strcmp(opcode, "sw") == 0)  return "11";
    if (strcmp(opcode, "reti") == 0) return "12";
    if (strcmp(opcode, "in") == 0)   return "13";
    if (strcmp(opcode, "out") == 0)  return "14";
    if (strcmp(opcode, "halt") == 0) return "15";
    else printf("illegal opcode\n");
    return "";
}
char decoder_rrr(const char* rd)        // Decoding registers
{
    if (strcmp(rd, "$zero") == 0) return '0';
    if (strcmp(rd, "$imm") == 0)  return '1';
    if (strcmp(rd, "$v0") == 0)   return '2';
    if (strcmp(rd, "$a0") == 0)   return '3';
    if (strcmp(rd, "$a1") == 0)   return '4';
    if (strcmp(rd, "$a2") == 0)   return '5';
    if (strcmp(rd, "$a3") == 0)   return '6';
    if (strcmp(rd, "$t0") == 0)   return '7';
    if (strcmp(rd, "$t1") == 0)   return '8';
    if (strcmp(rd, "$t2") == 0)   return '9';
    if (strcmp(rd, "$s0") == 0)   return 'A';
    if (strcmp(rd, "$s1") == 0)   return 'B';
    if (strcmp(rd, "$s2") == 0)   return 'C';
    if (strcmp(rd, "$gp") == 0)   return 'D';
    if (strcmp(rd, "$sp") == 0)   return 'E';
    if (strcmp(rd, "$ra") == 0)   return 'F';
    else printf("illegal register\n");
    return " ";
}
void write_to_text(const Format* f, FILE* fpathw, int* RI, RI_Format* ri_format_array) {
    // Copy the decoded opcode to the ri_format_array
    strncpy(ri_format_array[*RI].opcode, decoder_opcode(f->opcode), sizeof(ri_format_array[*RI].opcode) - 1);
    ri_format_array[*RI].opcode[sizeof(ri_format_array[*RI].opcode) - 1] = '\0';  // Null-terminate the string

    // Decode and copy the register identifiers (rd, rs, rt) to the ri_format_array
    ri_format_array[*RI].rd = decoder_rrr(f->rd);
    ri_format_array[*RI].rs = decoder_rrr(f->rs);
    ri_format_array[*RI].rt = decoder_rrr(f->rt);

    ri_format_array[*RI].set = true;

    // Copy the immediate value to the ri_format_array
    strncpy(ri_format_array[*RI].immediate, f->immediate, sizeof(ri_format_array[*RI].immediate) - 1);
    ri_format_array[*RI].immediate[sizeof(ri_format_array[*RI].immediate) - 1] = '\0';  // Null-terminate the string

    (*RI)++;  // Increment the RI index to point to the next available entry
}

int find_label_new_line(Label* labels_array, char* a) {
    // Function that finds the label in the labels array and returns the correct line to jump to in the machine code
    int i;
    for (i = 0; i < 4096; i++) {
        // Compare the label name in the array with the provided label name 'a'
        if (strcmp(a, labels_array[i].label_name) == 0) {
            return labels_array[i].text_line;  // Return the line number associated with the label
        }
    }
    return -1;  // Return -1 if the label is not found 
}

void print_to_file(FILE* fpathw, Label* labels_array, RI_Format* ri_format_array, int line_count, pse* pse_array, int max_line) // Print to memin
{
    int i;
    for (i = 0; i < line_count; i++)
    {
        if (ri_format_array[i].set == true)
        {
            // Write the opcode and registers to the file
            fprintf(fpathw, "%s%c%c%c\n", ri_format_array[i].opcode, ri_format_array[i].rd, ri_format_array[i].rs, ri_format_array[i].rt);

            // Check if any of the registers are the immediate register '1'
            if (ri_format_array[i].rt == '1' || ri_format_array[i].rd == '1' || ri_format_array[i].rs == '1')
            {
                // Check if the immediate value is a label 
                if (!isdigit(ri_format_array[i].immediate[0]) && ri_format_array[i].immediate[0] != '-')
                {
                    char* a = ri_format_array[i].immediate;
                    int line = find_label_new_line(labels_array, a); // Find the line number of the label
                    char hexStr[6];
                    snprintf(hexStr, sizeof(hexStr), "%05X", line); // Convert the line number to a 5-digit hexadecimal string
                    fprintf(fpathw, "%s\n", hexStr); // Write the hexadecimal line number to the file
                }
                // Check if the immediate value is in hexadecimal format
                else if (strncmp(ri_format_array[i].immediate, "0x", 2) == 0)
                {
                    unsigned int hex_value = (unsigned int)strtol(ri_format_array[i].immediate, NULL, 16); // Convert hex string to integer
                    fprintf(fpathw, "%05X\n", hex_value); // Write the hexadecimal value to the file

                }
                else
                {
                    int decimal_value = atoi(ri_format_array[i].immediate); // Convert the immediate value to an integer
                    int hex_value = decimal_value & 0xFFFFF; // Mask the value to fit 5 hexadecimal digits
                    fprintf(fpathw, "%05X\n", hex_value); // Write the hexadecimal value to the file
                }
            }
        }
    }

    for (i = 0; i < line_count; i++)
    {
        if (pse_array[i].set == true)
        {
            // If the new line number is greater than the current max line number
            if (pse_array[i].new_line > max_line - 1)
            {
                int j;
                // Write empty lines (00000) up to the new line number
                for (j = max_line + 1; j <= pse_array[i].new_line; j++)
                {
                    fprintf(fpathw, "%s\n", "00000");
                }
                max_line = pse_array[i].new_line + 1; // Update the max line number
                fprintf(fpathw, "%s\n", pse_array[i].data); // Write the pse data to the new line
            }
            else
            {
                rewind(fpathw); // Rewind the file pointer to the beginning
                int off = 7 * (pse_array[i].new_line); // Calculate the offset to the correct line (assuming each line is 7 characters long including newline)
                fseek(fpathw, off, SEEK_SET); // Move the file pointer to the correct line
                fprintf(fpathw, "%s", pse_array[i].data); // Overwrite the existing line with the new pse data
            }
        }
    }
}


int write_memin(FILE* fpathr, FILE* fpathw, Label* labels_array, int* labelarr_count, RI_Format* ri_format_array, pse* pse_array) {
    char line[500];
    int line_num = 1;
    Format format;
    int new_line = 0;
    int RI = 0;
    int pse_count = 0;

    while (fgets(line, sizeof(line), fpathr) != NULL)
    {
        format.set = false;  // Reset the format set flag for each line
        int the_line = 1;
        char* comment_start = strchr(line, '#');  // Find the start of the comment
        char* label_end = strchr(line, ':');      // Find the end of the label

        // Check if the colon is part of a comment
        if (label_end != NULL && (comment_start == NULL || label_end < comment_start))
        {
            int label_length = label_end - line;  // Calculate the length of the label

            toLowerCase(line);  // Convert the line to lowercase
            int t = 0;
            while (isspace(line[t])) {
                t++;  // Skip initial whitespace
            }
            int j = 0;
            for (int i = t; i < label_length; i++) {
                if (!isspace(line[i])) {
                    labels_array[*labelarr_count].label_name[j++] = line[i];  // Copy label name
                }
            }
            labels_array[*labelarr_count].label_name[j] = '\0';  // Null-terminate the label name
            labels_array[*labelarr_count].line = line_num;       // Store the line number of the label
            labels_array[*labelarr_count].text_line = new_line;  // Store the text line number
            (*labelarr_count)++;  // Increment the label array count
            memmove(line, label_end + 1, strlen(label_end + 1) + 1);  // Remove the label from the line
            char* trimmed_line = line;
            while (*trimmed_line == ' ' || *trimmed_line == '\t') {
                trimmed_line++;  // Skip leading whitespace after label
            }
            if (*trimmed_line == '\0' || *trimmed_line == '\n') {
                the_line = 0;  // If the remaining line is empty, skip processing
            }
        }

        int word_count = 0;
        char* word = strtok(line, " ,\t\n");  // Tokenize the line by spaces, tabs, and newlines

        if (word != NULL && strcmp(word, ".word") == 0) {
            the_line = 0;  // Flag to skip the line processing for .word directive
            pse w;
            w.set = true;
            int t;
            while (word != NULL) {
                if (word[0] == '#') {
                    break;  // Skip processing if a comment is encountered
                }
                switch (word_count) {
                case 0:
                    break;
                case 1:
                    if (strncmp(word, "0x", 2) == 0 || strncmp(word, "0X", 2) == 0) {
                        // Hexadecimal value, no conversion needed
                        t = (int)strtol(word, NULL, 16);
                        w.new_line = t;
                    }
                    else {
                        // Decimal value, convert to hexadecimal
                        t = atoi(word);
                        w.new_line = t;
                    }
                    break;
                default:
                    if (strncmp(word, "0x", 2) == 0 || strncmp(word, "0X", 2) == 0) {
                        // Hexadecimal value, no conversion needed
                        t = (int)strtol(word, NULL, 16);
                    }
                    else {
                        // Decimal value, convert to hexadecimal
                        t = atoi(word);
                    }
                    snprintf(w.data, sizeof(w.data), "%05X", t);  // Convert integer to hexadecimal format
                    w.data[sizeof(w.data) - 1] = '\0';  // Null-terminate the data
                    break;
                }
                word_count++;
                word = strtok(NULL, " ,\t\n");  // Get the next word
            }
            pse_array[pse_count] = w;  // Store the parsed .word data
            pse_count++;  // Increment the pse array count
        }
        else
        {
            while (word != NULL) {
                if (word[0] == '#') {
                    break;  // Skip processing if a comment is encountered
                }
                switch (word_count) {
                case 0:
                    strncpy(format.opcode, word, sizeof(format.opcode) - 1);  // Copy the opcode
                    format.opcode[sizeof(format.opcode) - 1] = '\0';  // Null-terminate the opcode
                    format.set = true;  // Set the format flag
                    break;
                case 1:
                    strncpy(format.rd, word, sizeof(format.rd) - 1);  // Copy the destination register
                    format.rd[sizeof(format.rd) - 1] = '\0';  // Null-terminate the destination register
                    break;
                case 2:
                    strncpy(format.rs, word, sizeof(format.rs) - 1);  // Copy the source register
                    format.rs[sizeof(format.rs) - 1] = '\0';  // Null-terminate the source register
                    break;
                case 3:
                    strncpy(format.rt, word, sizeof(format.rt) - 1);  // Copy the second source register
                    format.rt[sizeof(format.rt) - 1] = '\0';  // Null-terminate the second source register
                    break;
                default:
                    toLowerCase(word);  // Convert the word to lowercase
                    strncpy(format.immediate, word, sizeof(format.immediate) - 1);  // Copy the immediate value
                    format.immediate[sizeof(format.immediate) - 1] = '\0';  // Null-terminate the immediate value
                    break;
                }
                word_count++;
                word = strtok(NULL, " ,\t\n");  // Get the next word
            }
            if (the_line == 1 && format.set == true)
            {
                // Check if any operand is an immediate value
                if (strcmp(format.rt, "$imm") == 0 || strcmp(format.rd, "$imm") == 0 || strcmp(format.rs, "$imm") == 0 || !isdigit(format.immediate[0]))
                {
                    new_line += 2;  // Adjust new_line if there is an immediate value
                }
                else {
                    new_line += 1;  // Adjust new_line for normal instructions
                }
                write_to_text(&format, fpathw, &RI, ri_format_array);  // Write the instruction to the output file
            }
        }

        line_num++;  // Increment the line number
    }
    return new_line;
}

int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 3) {
        printf("Wrong input, you didn't give enough arguments");
        return 1; // Exit with error code if arguments are insufficient
    }

    // Open the input assembly file for reading
    FILE* asmfpr = open_fr(argv[1]);
    if (asmfpr == NULL) return EXIT_FAILURE; // Exit if file opening fails

    // Open the output file for writing
    FILE* asmfpw = open_fw(argv[2]);
    if (asmfpw == NULL) {
        fclose(asmfpr); // Close the input file before exiting
        return EXIT_FAILURE; // Exit if file opening fails
    }

    // Initialize the labels array and label count
    Label labelarr[4096];
    int label_count = 0;

    // Rewind the input file pointer to the beginning of the file
    rewind(asmfpr);

    // Count the number of lines in the input file
    int line_count = count_lines_in_file(argv[1]);
    if (line_count <= 0) {
        fclose(asmfpr); // Close the input file
        fclose(asmfpw); // Close the output file
        return EXIT_FAILURE; // Exit if the file is empty or an error occurred
    }

    // Create an array of RI_Format structures with size equal to the number of lines
    RI_Format* ri_format_array = create_ri_format_array(line_count);

    // Create an array of pse structures with size equal to the number of lines
    pse* pse_array = create_pse_array(line_count);

    // Write the assembly code to the memin file, and get the end line number
    int end_line = write_memin(asmfpr, asmfpw, labelarr, &label_count, ri_format_array, pse_array);

    // Print the RI_Format and pse arrays to the output file
    print_to_file(asmfpw, labelarr, ri_format_array, line_count, pse_array, end_line);

    // Close the input and output files
    fclose(asmfpr);
    fclose(asmfpw);

    // Free the allocated memory for the RI_Format and pse arrays
    free_ri_format_array(ri_format_array);
    free_pse_array(pse_array);

    return EXIT_SUCCESS; // Return success code
}
