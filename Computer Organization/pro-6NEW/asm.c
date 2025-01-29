#define _CRT_SECURE_NO_WARNINGS

#include "asm.h"

#define err_msg(msg) \
    fprintf(stderr, "\nError: %s\nline: %d\n\n", msg, __LINE__);

int instruction_pc = 0, lb_count = 0, last_word_index = -1, opcodes_num = 22, num_registers = 16;

char* opcodes[] = { "add", "sub", "mac", "and", "or", "xor", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt" };
char* registers[] = { "$zero", "$imm1", "$imm2","$v0", "$a0", "$a1", "$a2",  "$t0", "$t1", "$t2", "$s0", "$s1", "$s2", "$gp", "$sp", "$ra" };
char tmp_str[MAX_LINE_LEN] = "";

int imemin_file[MAX_PROGRAM_LINES][7];
int dmemin_file[MAX_DATA_MEMORY];

/********** Init functions ***************/

/**
 * sets the imemin array to be all zeros
 */
void init_imemin_array()
{
    int i;
    for (i = 0; i < MAX_PROGRAM_LINES; i++)
    {
        for (int j = 0; j < 7; ++j)
        {
            imemin_file[i][j] = 0;
        }
    }
}

/**
 * sets the dmemin array to be all zeros
 */
void init_dmemin_array()
{
    for (int i = 0; i < MAX_DATA_MEMORY; ++i)
    {
        dmemin_file[i] = 0;
    }
}


/**
 * given filepath, reads asm code from filePath
 * checks path validation
 * return: a File pointer
 */
FILE* get_file(const char* filePath)
{
    FILE* in_file;
    in_file = fopen(filePath, "r");

    //check validation
    if (in_file == NULL)
    {
        err_msg("open file");
        exit(1);
    }
    return in_file;
}

/********** first run on file functions ***************/

/**
 * given a File pointer and an array of labels, reads each line, checks whether it concludes a label and if so call read_label function
 * pre: file is a valid FILE pointer, labels_arr initialized
 *
 * return: labels count
 */
int get_labels(FILE* file, label* labels_arr)
{
    int line_counter = 0;
    instruction_pc = 0;
    char line_str[MAX_LINE_LEN];

    while (!feof(file) && line_counter < MAX_PROGRAM_LINES)
    {
        fgets(line_str, MAX_LINE_LEN, file); // save line content in line_str

        int tokens = get_line_num_of_tokens(line_str);

        char* pch = strtok(line_str, "#"); /* remove the comment if there is one*/
        if (tokens == 0)
            continue;

        //check if an empty line, a comment, or a pseudo-instruction - .word address data -> if so continue to next line
        if (strstr(pch, ".word") != NULL)
            continue;

        int colons_count = colon_count(pch);

        char* colon_ptr = strstr(pch, ":");
        if (colon_ptr != NULL)
        {
            int num_labels_in_row = 0;
            pch = strtok(pch, ":"); // get the first label str if exist

            while (pch != NULL && num_labels_in_row < colons_count)
            {
                pch = trimwhitespace(pch);

                //check if a space pch
                int k = 0;
                while (pch[k] == '\t' || pch[k] == ' ')
                    k++;
                if (pch[k] == '\0')
                {
                    pch = strtok(NULL, ":");
                    continue;
                }

                int success = read_label(pch, labels_arr);
                if (success == 0)
                {
                    num_labels_in_row++;
                    lb_count++;
                }
                pch = strtok(NULL, ":");
            }

            if (tokens <= num_labels_in_row)
            {
                instruction_pc--; // there is a label line without instruction
            }

        }
        // instruction line
        line_counter++;
        instruction_pc++;

    }
    return lb_count;
}

/**
 * given a string, return count appearance of a colon
 *
 * return: colons count
 */
int colon_count(const char* str)
{
    int i = 0, count = 0;
    while (str[i] != '\0')
    {
        if (str[i] == ':')
            count++;
        i++;
    }
    return count;
}

/**
 * given a label name (string) and an array of labels, adds a new label to the array
 *
 * returns: 0 if succeed adding label
 *          -1 o.w. (a label already exist in the labels array)
 */
int read_label(const char* str, label* labels_arr)
{
    int i = 0;
    char ch = str[i]; // get first char
    char name[LABEL_MAX_LEN] = "";
    strcpy(labels_arr[lb_count].name, "");

    while (ch != '\0')
    {
        name[i] = ch;
        i++;
        ch = str[i];//continue to the nect char in the line
    }
    name[i] = '\0';

    if (find_index_in_labels(labels_arr, lb_count, name) == -1) // check if it is a new label
    {
        labels_arr[lb_count].address = instruction_pc;
        strcpy(labels_arr[lb_count].name, name);
    }
    else
    {
        err_msg("Duplicated labels are not allowed");
        return -1;
    }

    return 0;
}

/********** second run on file ***************/

/**
 * given a File pointer, an array of labels and num of labels, reads each line and update imemin 2D-array and dmemin array
 *
 * pre: file is a valid FILE pointer, inst_array initialized, labels_arr initialized, num_labels contains the exact amount of labels in asm program
 *
 */
void get_all_instructions(FILE* file, label* labels_arr, int num_labels)
{
    int line_counter = 0;
    instruction_pc = 0;
    char line_str[MAX_LINE_LEN];

    while (fgets(line_str, MAX_LINE_LEN, file) != NULL && line_counter < MAX_PROGRAM_LINES)
    {
        int tokens = get_line_num_of_tokens(line_str);

        if (tokens == 0 || tokens == 1)
            continue;
        if (tokens == 3) // a word line
        {
            int wo_index = word_inst(line_str);
            if (wo_index > last_word_index)
                last_word_index = wo_index;
        }
        else if (get_instruction(line_str, labels_arr, num_labels) == 1)
        {
            instruction_pc++;
        }
        line_counter++;
    }
}


/**
 * given a string, an array of labels and num of labels, reads each line and update imemin 2D-array and dmemin array
 *
 * pre: file is a valid FILE pointer, labels_arr initialized, num_labels contains the exact amount of labels in asm program
 *
 * returns: 1 if succeed
 *          0 o.w.
 */
int get_instruction(const char* line_str, label* labels_arr, int num_labels)
{
    char* pch = "";
    int counter = 0;
    strcpy(tmp_str, line_str);
    pch = strtok(tmp_str, "#"); /* remove the comment if there is one*/

    int colons_count = colon_count(pch);
    int i = 0;
    if (colons_count > 0)
    {
        pch = strtok(pch, ":"); // get the first label str if exist
        while (i < colons_count)
        {
            i++;
            pch = strtok(NULL, ":");
        }
    }

    pch = strtok(pch, " ,");

    for (counter = 0; counter < 7; counter++)
    {
        if (pch == NULL)
            return 0;
        pch = trimwhitespace(pch);


        if (counter == 0)
            imemin_file[instruction_pc][0] = find_index(opcodes, opcodes_num, pch);
        else if (counter == 1)
            imemin_file[instruction_pc][1] = find_index(registers, num_registers, pch);
        else if (counter == 2)
            imemin_file[instruction_pc][2] = find_index(registers, num_registers, pch);
        else if (counter == 3)
            imemin_file[instruction_pc][3] = find_index(registers, num_registers, pch);
        else if (counter == 4)
            imemin_file[instruction_pc][4] = find_index(registers, num_registers, pch);
        else if (counter == 5)
            imemin_file[instruction_pc][5] = imm_to_int(pch, labels_arr, num_labels) & 0xfff; // imm1 is 3-bit wide
        else if (counter == 6)
            imemin_file[instruction_pc][6] = imm_to_int(pch, labels_arr, num_labels) & 0xfff; // imm1 is 3-bit wide

        pch = strtok(NULL, " ,");
    }
    return 1;
}

/**
 * gets a word instruction (string) and update the dmemin file accordingly
 *
 * pre: str is a valid .word line
 *
 * returns: int index of the word
 *          0 o.w.
 */
int word_inst(char* str)
{
    char* pch;
    int counter = 0;
    int index, value;
    strcpy(tmp_str, str);
    pch = strtok(tmp_str, "#"); /* remove the comment if there is one*/
    pch = strtok(tmp_str, " ,");
    for (counter = 0; counter < 3; counter++)
    {
        if (pch == NULL)
            return 0;
        pch = trimwhitespace(pch);
        if (counter == 1)
            index = imm_to_int(pch, NULL, 0); // imm_to_int handles decimal and
        else if (counter == 2)
            value = imm_to_int(pch, NULL, 0);
        pch = strtok(NULL, " ,");
    }
    dmemin_file[index] = value;
    return index;
}

/**
 * count the number of tokens in a given line
 * for example the line:
 * beq $zero, $zero, $zero, $imm1, L2, 0 # jump to L2 (reg1 = address L2)
 * has 7 tokens
 * returns: num of tokens
 */
int get_line_num_of_tokens(char* line_str)
{
    char* pch;
    int counter = 0;
    strcpy(tmp_str, line_str);
    pch = trimwhitespace(tmp_str);
    clean_comments(pch, (int)strlen(pch));
    if (strlen(pch) == 0)
        return 0;
    pch = strtok(pch, " ,");
    if (strlen(pch) == 0)
        return 0;
    while (pch != NULL)
    {
        counter++;
        pch = trimwhitespace(pch);
        pch = strtok(NULL, " ,");
    }
    return counter;
}

/**
 * gets a string and returns a string without leading and trailing spaces
 */
char* trimwhitespace(char* str)
{
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void clean_comments(char* str, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (str[i] == '#')
        {
            str[i] = '\0';
            return;
        }
    }
}

/**
 * given an array of strings, it's length and a string find the index of the string in the array
 * returns: index if found
 *          exit(1) o.w.
 */
int find_index(char** arr, int len, char* val)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (strcmp(val, arr[i]) == 0)
        {
            return i;
        }
    }

    fprintf(stderr, "Error: Invalid register at instruction %d ;invalid value: %s\n", instruction_pc, val);
    exit(1);
}

/**
 * given an array of labels, it's length and a string, finds the index of the string in the labels array by name.
 * returns: index if found
 *          -1 o.w.
 */
int find_index_in_labels(label* labels_arr, int len, char* val)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (strcmp(val, labels_arr[i].name) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 * gets an immediate string, labels array and addresses array and the size of the array
 * first check if imm appear in the labels array. If so return the corresponding address from labels_addresses
 * otherwise return the integer value of the immediate (assumes to be either in decimal or hexadecimal)
 */
int imm_to_int(char* imm, label* labels_arr, int labels_num)
{
    /* first check if it is a label*/
    int i;
    if (labels_arr)
    {
        i = find_index_in_labels(labels_arr, labels_num, imm);
        if (i >= 0)
            return labels_arr[i].address; //
    }
    if (imm[0] == '0' && imm[1] == 'x')
    {
        uint32_t num = (uint32_t)strtol(imm, NULL, 16);
        return extend_sign(num, 12);
    }
    return (int)strtol(imm, NULL, 10); // decimal
}

/**
 * extend sign from left to register
 * i.e. reg=0x00000fab, sign_bit=11 -> result=0xffffffab
 *
 */
int extend_sign(uint32_t reg, uint8_t sign_bit)
{
    int sign = (int)(reg >> sign_bit) & 1;
    if (sign)
        reg |= ~0 << sign_bit;
    return reg;
}

/********** Write functions ***************/

/**
 * write  dmemin file from mem_file
 * format:  Each line contains 8-bit hexadecimal data.
 * post:    The file path mem_file lead to a valid file
 *          that containing dmemin data
 */
int write_file_dmem(char* mem_file)
{
    FILE* fmem;
    fmem = fopen(mem_file, "w");
    if (fmem == NULL)
    {
        err_msg("open file fault");
        return 1;
    }
    int i;
    for (i = 0; i <= last_word_index; i++)
        fprintf(fmem, "%08x\n", dmemin_file[i]);
    
    if (fclose(fmem) != 0)
    {
        err_msg("close file");
        return 1;
    }
    return 0;
}

/**
 * write  imemin file from mem_file
 * format:  Each line contains 12-bit hexadecimal data.
 * post:    The file path mem_file lead to a valid file
 *          that containing imemin data
 */
int write_file_imem(char* mem_file)
{
    FILE* fmem;
    fmem = fopen(mem_file, "w");
    if (fmem == NULL)
    {
        err_msg("open file");
        return 1;
    }
    int i = 0;
    for (i = 0; i < instruction_pc; i++)
    {
        fprintf(fmem, "%02X", imemin_file[i][0]);
        fprintf(fmem, "%01X", imemin_file[i][1]);
        fprintf(fmem, "%01X", imemin_file[i][2]);
        fprintf(fmem, "%01X", imemin_file[i][3]);
        fprintf(fmem, "%01X", imemin_file[i][4]);
        fprintf(fmem, "%03X", imemin_file[i][5]);
        fprintf(fmem, "%03X\n", imemin_file[i][6]);
    }

    if (fclose(fmem) != 0)
    {
        err_msg("close file");
        return 1;
    }
    return 0;
}

// -------------- main -------------- //

int main(int argc, char* argv[])
{
    // argv = {asm.exe program.asm imemin.txt dmemin.txt}
    if (argc != 4)
    {
        // do error
        err_msg("Invalid arguments");
        exit(1);
    }


    int labels_num;

    label lb_array[MAX_LABELS];
    init_imemin_array();
    init_dmemin_array();


    //first run on the file
    FILE* asm_file = get_file(argv[1]);
    labels_num = get_labels(asm_file, lb_array);
    if (fclose(asm_file) != 0)
    {
        err_msg("close file");
        return 1;
    }
    // end of first run

    //second run on the file
    asm_file = get_file(argv[1]);
    get_all_instructions(asm_file, lb_array, labels_num);

    if (fclose(asm_file) != 0)
    {
        err_msg("close file");
        return 1;
        return 1;
    }
    // end of second run

    write_file_imem(argv[2]);
    write_file_dmem(argv[3]);

    return 0;
}
