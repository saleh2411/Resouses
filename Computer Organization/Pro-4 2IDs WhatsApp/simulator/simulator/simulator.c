#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<stdint.h>
#include<assert.h>
#include<stdbool.h>


typedef struct {
    int rd;       // Destination register
    int rs;       // Source register
    int rt;       // Second source register or immediate value
    bool imm;     // Flag indicating if is an immediate value
} line_reg;

typedef struct {                      
    unsigned int irq0enable : 1;      // IRQ 0 enable flag
    unsigned int irq1enable : 1;      // IRQ 1 enable flag
    unsigned int irq2enable : 1;      // IRQ 2 enable flag
    unsigned int irq0status : 1;      // IRQ 0 status flag
    unsigned int irq1status : 1;      // IRQ 1 status flag
    unsigned int irq2status : 1;      // IRQ 2 status flag
    unsigned int irqhandler : 12;     // IRQ handler address
    unsigned int irqreturn : 12;      // IRQ return address
    int clks;                         // Clock cycles counter
    int leds;                         // LED status
    int display7seg;                  // 7-segment display status
    unsigned int timerenable : 1;     // Timer enable flag
    int timercurrent;                 // Current timer value
    int timermax;                     // Maximum timer value
    unsigned int diskcmd : 2;         // Disk command
    unsigned int disksector : 7;      // Disk sector number
    unsigned int diskbuffer : 12;     // Disk buffer address
    unsigned int diskstatus : 1;      // Disk status flag
    int reserved1;                    // Reserved field 1
    int reserved2;                    // Reserved field 2
    unsigned int monitoraddr : 16;    // Monitor address
    unsigned int monitordata : 8;     // Monitor data
    unsigned int monitorcmd : 1;      // Monitor command flag
} IO_reg;

FILE* open_fr(const char* fPath) {
    FILE* fpoint = fopen(fPath, "r"); // Open file for reading
    if (fpoint == NULL) {
        perror("Error opening file for reading"); // Print error if file cannot be opened
    }
    return fpoint;
}

FILE* open_fw(const char* fPath) {
    FILE* fpoint = fopen(fPath, "w"); // Open file for writing
    if (fpoint == NULL) {
        perror("Error opening file for writing"); // Print error if file cannot be opened
    }
    return fpoint;
}

FILE* open_fwyuv(const char* fPath) {
    FILE* fpoint = fopen(fPath, "wb"); // Open file for writing in binary mode
    if (fpoint == NULL) {
        perror("Error opening file for writing"); // Print error if file cannot be opened
    }
    return fpoint;
}

FILE* open_fp(const char* fPath) {
    FILE* fpoint = fopen(fPath, "a"); // Open file for appending
    if (fpoint == NULL) {
        perror("Error opening file for pending"); // Print error if file cannot be opened
    }
    return fpoint;
}

int sign_extend_20_to_32(int value) {
    if (value & 0x80000) { // Check if the 20th bit (MSB of a 20-bit number) is set
        value |= ~0xFFFFF; // Extend the sign bit to 32 bits
    }
    return value;
}

void init(FILE* fp, int arr[], size_t arr_size)
{
    int i = 0;
    char line[7]; // Buffer to hold each line read from the file
    while (fgets(line, 7, fp) != NULL && i < arr_size)
    {
        line[sizeof(line) - 1] = '\0'; // Ensure null termination
        arr[i] = (unsigned int)strtol(line, NULL, 16); // Convert hex string to integer
        i++;
    }
    rewind(fp); // Reset file pointer to the beginning
}

void init_irq2(FILE* irqp, int irq2[])
{
    int i = 0;
    char line[7]; // Buffer to hold each line read from the file
    while (fgets(line, 7, irqp) != NULL && i < 4096)
    {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character
        irq2[i] = (int)strtol(line, NULL, 10); // Convert decimal string to integer
        i++;
    }
    rewind(irqp); // Reset file pointer to the beginning
}

void make_irq(int* io_registers, int irq2[], int* i)
{
    // Increment the timer if it is enabled
    if (io_registers[11]) io_registers[12]++;

    // Check if timer matches the max value and trigger IRQ if it does
    if (io_registers[13] == io_registers[12] && io_registers[13] != 0)
    {
        io_registers[12] = 0;
        io_registers[3] = 1;
    }

    // Check irq2 array for matching interrupts
    int j = 0;
    while (irq2[j] != NULL)
    {
        if (irq2[j] == (*i - 1))
        {
            io_registers[5] = 1;
        }
        j++;
    }
}

void check_irq(int io_registers[], int* busy, int* pc, bool* i_need_to_jump)
{
    // Evaluate each IRQ enable and status flag
    bool irq0 = io_registers[0] && io_registers[3];
    bool irq1 = io_registers[1] && io_registers[4];
    bool irq2 = io_registers[2] && io_registers[5];
    bool irq = (irq0 || irq1 || irq2); // Check if any IRQ is triggered

    // If not busy and an IRQ is triggered, handle the interrupt
    if (!*busy)
    {
        if (irq)
        {
            io_registers[7] = *pc; // Save current PC to irqreturn register
            *pc = io_registers[6]; // Jump to irqhandler address

            *busy = 1; // Set busy flag
            *i_need_to_jump = true; // Indicate a jump is needed
        }
    }
}

void disk_RW(int disk[], int memory[], int io_registers[])
{
    int i;

    // Check if disk read command is issued and not already in progress
    if (io_registers[14] == 1 && io_registers[17] == 0)
    {
        io_registers[17] = 1; // Set disk status to busy
        for (i = 0; i < 128; i++)
        {
            if (i + io_registers[16] < 4096) // Ensure within memory bounds
            {
                memory[i + io_registers[16]] = disk[(128 * io_registers[15])]; // Read from disk to memory
            }
        }
    }

    // Check if disk write command is issued and not already in progress
    if (io_registers[14] == 2 && io_registers[17] == 0)
    {
        io_registers[17] = 1; // Set disk status to busy
        for (i = 0; i < 128; i++)
        {
            if (i + io_registers[16] < 4096) // Ensure within memory bounds
            {
                disk[(128 * io_registers[15]) + i] = memory[i + io_registers[16]]; // Write from memory to disk
            }
        }
    }
}


void simulator(FILE* memeinp, FILE* memeoutp, FILE* regoutp, FILE* tracep, FILE* hwregtracep, FILE* cyclesp,
    FILE* ledsp, FILE* display7segp, FILE* monitorp, int arr[], unsigned int memory[], IO_reg IO, int irq2[],
    unsigned int disk[], FILE* monitoryuvp, FILE* diskoutp)
{
    char line[7];  // Buffer to store each line from the file
    line_reg liner;  // Struct to store instruction details
    int imm_temp;  // Temporary storage for immediate value
    int pc = 0;  // Program counter
    int printpc;  // Variable to store the PC for printing
    int i = 0;  // Cycle counter
    int j = 0;  // Memory output index
    int cyc = 0;  // Total cycles
    int busy = 0;  // Flag for IRQ busy status
    uint8_t monitor[256 * 256] = { 0 };  // Monitor memory
    bool i_need_to_jump = false;  // Flag to check if a jump is needed
    int io_registers[] = {  // Array to store IO register values
        IO.irq0enable, IO.irq1enable, IO.irq2enable, IO.irq0status,
        IO.irq1status, IO.irq2status, IO.irqhandler, IO.irqreturn,
        IO.clks, IO.leds, IO.display7seg, IO.timerenable,
        IO.timercurrent, IO.timermax, IO.diskcmd, IO.disksector,
        IO.diskbuffer, IO.diskstatus, IO.reserved1, IO.reserved2,
        IO.monitoraddr, IO.monitordata, IO.monitorcmd
    };

    const char* IO_reg_names[] = {  // Names of IO registers
        "irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status", "irq2status",
        "irqhandler", "irqreturn", "clks", "leds", "display7seg", "timerenable",
        "timercurrent", "timermax", "diskcmd", "disksector", "diskbuffer", "diskstatus",
        "reserved1", "reserved2", "monitoraddr", "monitordata", "monitorcmd"
    };

    int io_index;  // Index for IO register access
    int busy_cyc = -1;  // Cycle counter for disk operation

    while (fgets(line, sizeof(line), memeinp) != NULL && i != -1) {
        i++;
        // Handle disk operation completion after 1024 cycles
        if (busy_cyc != -1 && i >= busy_cyc + 1024) {
            io_registers[17] = 0;  // Reset disk status
            io_registers[14] = 0;  // Reset disk command
            io_registers[4] = 1;  // Set IRQ1 status
            busy_cyc = -1;  // Reset busy cycle counter
        }

        check_irq(&io_registers, &busy, &pc, &i_need_to_jump);  // Check for IRQ
        make_irq(&io_registers, irq2, &i);  // Generate IRQ

        // Handle PC jump if needed
        if (i_need_to_jump) {
            rewind(memeinp);
            int off = 7 * pc;
            fseek(memeinp, off, SEEK_SET);
            fgets(line, sizeof(line), memeinp);
            i_need_to_jump = false;
        }
        printpc = pc;
        char temp[7];
        strcpy(temp, line);
        temp[5] = '\0';
        liner.imm = false;
        // Check if the line is empty
        if (strlen(line) == 0) continue;

        // Extract the operation code (first 2 characters)
        char op[3];
        strncpy(op, line, 2);
        op[2] = '\0';  // Null-terminate the string

        // Convert the operation code from hex to integer
        int op_int = (int)strtol(op, NULL, 16);

        // Extract rd, rs, and rt values from the line (make sure the line is long enough)
        if (strlen(line) >= 5) {  // Ensure the line is long enough
            char temprd[2] = { line[2], '\0' };
            char temprs[2] = { line[3], '\0' };
            char temprt[2] = { line[4], '\0' };
            liner.rd = (int)strtol(temprd, NULL, 16);
            liner.rs = (int)strtol(temprs, NULL, 16);
            liner.rt = (int)strtol(temprt, NULL, 16);

            arr[1] = 0;
            // Check if any of the registers are 1
            liner.imm = (liner.rd == 1 || liner.rs == 1 || liner.rt == 1);
            if (liner.imm) {

                // Read the next line
                if (fgets(line, sizeof(line), memeinp) != NULL) {
                    pc += 2;
                    // Remove newline character if present
                    line[strcspn(line, "\n")] = '\0';

                    // Convert the line to an integer and store in arr
                    int value = (int)strtol(line, NULL, 16);
                    arr[1] = sign_extend_20_to_32(value); // Save the integer value to the provided array

                }
                else {
                    // Handle the case where there's no next line to read
                    printf("No additional line to read.\n");
                }
            }
            else pc += 1;
            fprintf(tracep, "%03X %s %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X\n",  // Printing to trace file
                printpc, temp, arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], arr[8], arr[9], arr[10], arr[11],
                arr[12], arr[13], arr[14], arr[15]);

            // Process the operation code
            switch (op_int) {
            case 0x00:  // ADD
                arr[liner.rd] = arr[liner.rs] + arr[liner.rt];
                break;
            case 0x01:  // SUB
                arr[liner.rd] = arr[liner.rs] - arr[liner.rt];
                break;
            case 0x02:  //MUL
                arr[liner.rd] = arr[liner.rs] * arr[liner.rt];
                break;
            case 0x03:  //AND
                arr[liner.rd] = arr[liner.rs] & arr[liner.rt];
                break;
            case 0x04:  // OR
                arr[liner.rd] = arr[liner.rs] | arr[liner.rt];
                break;
            case 0x05:  // XOR
                arr[liner.rd] = arr[liner.rs] ^ arr[liner.rt];
                break;
            case 0x06:  // SLL
                arr[liner.rd] = arr[liner.rs] << arr[liner.rt];
                break;
            case 0x07:  // SRA
                if (arr[liner.rt] < 0)
                {
                    arr[liner.rd] = (arr[liner.rs] >> arr[liner.rt]) | ((0x0) >> arr[liner.rt]);
                }
                else
                {
                    arr[liner.rd] = (arr[liner.rs] >> arr[liner.rt]);
                }
                break;
            case 0x08:  // SRL
                arr[liner.rd] = arr[liner.rs] >> arr[liner.rt];
                break;
            case 0x09:  // BEQ
                if (arr[liner.rs] == arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;

                }
                break;
            case 0x0A:  // BNE
                if (arr[liner.rs] != arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;
                }
                break;
            case 0x0B:  // BLT 
                if (arr[liner.rs] < arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;
                }
                break;
            case 0x0C:  // BGT
                if (arr[liner.rs] > arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;
                }
                break;
            case 0x0D:  // BLE
                if (arr[liner.rs] <= arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;
                }
                break;
            case 0x0E:  // BGE 
                if (arr[liner.rs] >= arr[liner.rt])
                {
                    pc = arr[liner.rd];
                    i_need_to_jump = true;
                }
                break;
            case 0x0F:  // JAL
                arr[liner.rd] = pc;
                pc = arr[liner.rs];
                i_need_to_jump = true;
                break;
            case 0x10:  // LW
                if ((arr[liner.rs] + arr[liner.rt] < 4096) && (arr[liner.rs] + arr[liner.rt] >= 0))
                {
                    arr[liner.rd] = (int)memory[arr[liner.rs] + arr[liner.rt]];
                }
                break;
            case 0x11:  // SW
                if ((arr[liner.rs] + arr[liner.rt] < 4096) && (arr[liner.rs] + arr[liner.rt] >= 0))
                {
                    memory[arr[liner.rs] + arr[liner.rt]] = (arr[liner.rd] & 0X000FFFFF);
                }
                break;
            case 0x12:  // RETI
                pc = io_registers[7];
                i_need_to_jump = true;
                busy = 0;
                break;
            case 0x13:  //IN - READ
                // Compute the index into the io_registers array
                io_index = arr[liner.rs] + arr[liner.rt];
                // Ensure the index is within bounds
                if (io_index >= 0 && io_index < (int)(sizeof(io_registers) / sizeof(io_registers[0]))) {
                    if (io_index == 22) { arr[liner.rd] = 0; }
                    else { arr[liner.rd] = io_registers[io_index]; }
                    fprintf(hwregtracep, "%d %s %s %08X\n", i - 1, "READ", IO_reg_names[io_index], arr[liner.rd]);
                }
                break;
            case 0x14:  // OUT - WRITE
                // Compute the index into the io_registers array
                io_index = arr[liner.rs] + arr[liner.rt];
                // Ensure the index is within bounds
                if (io_index >= 0 && io_index < (int)(sizeof(io_registers) / sizeof(io_registers[0]))) {
                    io_registers[io_index] = arr[liner.rd];
                    if (io_index == 22 && arr[liner.rd] == 1)
                    {
                        monitor[io_registers[20]] = io_registers[21];
                    }
                    if (io_index == 14)
                    {
                        disk_RW(disk, memory, io_registers);
                        busy_cyc = i;
                    }

                    if (io_index == 9)
                    {
                        fprintf(ledsp, "%d %08X\n", i - 1, io_registers[9]);
                    }
                    if (io_index == 10) { fprintf(display7segp, "%d %08X\n", i - 1, arr[liner.rd]); }
                    fprintf(hwregtracep, "%d %s %s %08X\n", i - 1, "WRITE", IO_reg_names[io_index], arr[liner.rd]);

                }
                break;
            case 0x15: // HALT
                cyc = i;
                i = -1;
                break;
            default:
                printf("Operation is out of range\n");
                break;
            }

        }
    }
    int m = 0;
    int endm = (256 * 256 - 1);

    // Find the last non-zero entry in the monitor array
    while (endm > 0 && (monitor[endm] == 0))
    {
        endm--;
    }

    // Print the non-zero entries in the monitor array to the monitor file
    while (m < 256 * 256)
    {
        if (m <= endm)
        {
            fprintf(monitorp, "%02X\n", monitor[m]);
        }
        m++;
    }

    // Write the entire monitor array to the YUV file
    fwrite(monitor, 1, 256 * 256, monitoryuvp);

    // Print the register values to the regout file
    fprintf(regoutp, "%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X",
        arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], arr[8], arr[9], arr[10], arr[11],
        arr[12], arr[13], arr[14], arr[15]);

    // Print the total cycle count to the cycles file
    fprintf(cyclesp, "%d", cyc);

    int end = 4095;

    // Find the last non-zero entry in the memory array
    while (end > 0 && (memory[end] == 0))
    {
        end--;
    }

    // Print the non-zero entries in the memory array to the memeout file
    while (j <= end)
    {
        fprintf(memeoutp, "%05X\n", memory[j]);
        j++;
    }

    int dis = 0;

    // Print all the entries in the disk array to the diskout file
    while (dis < 128 * 128)
    {
        fprintf(diskoutp, "%05X\n", disk[dis]);
        dis++;
    }
}


int main(int argc, char* argv[])
{
    // Check if the correct number of arguments are provided
    if (argc != 14) {
        printf("Not enough arguments");
        return EXIT_FAILURE;
    }

    // Open files using command-line arguments
    FILE* meminp = open_fr(argv[1]);
    FILE* diskinp = open_fr(argv[2]);
    FILE* irqp = open_fr(argv[3]);
    FILE* memeoutp = open_fw(argv[4]);
    FILE* regoutp = open_fw(argv[5]);
    FILE* tracep = open_fw(argv[6]);
    FILE* hwregtracep = open_fw(argv[7]);
    FILE* cyclesp = open_fw(argv[8]);
    FILE* ledsp = open_fw(argv[9]);
    FILE* display7segp = open_fw(argv[10]);
    FILE* diskoutp = open_fw(argv[11]);
    FILE* monitorp = open_fw(argv[12]);
    FILE* monitoryuvp = open_fwyuv(argv[13]);

    // Checks if the files open correctly
    if (meminp == NULL || diskinp == NULL || irqp == NULL || memeoutp == NULL ||
        regoutp == NULL || tracep == NULL || hwregtracep == NULL || cyclesp == NULL ||
        ledsp == NULL || display7segp == NULL || diskoutp == NULL || monitorp == NULL ||
        monitoryuvp == NULL) {
        return EXIT_FAILURE;
    }

    // Initialize registers and memory arrays
    int reg[16] = { 0 };                     // Register file with 16 registers, initialized to 0
    unsigned int memory[4096] = { 0 };       // Memory array with 4096 spaces, initialized to 0
    unsigned int disk[(128 * 128)] = { 0 };  // Disk array with 128*128 spaces, initialized to 0
    int irq2[4096] = { '\0' };               // IRQ2 array with 4096 spaces, initialized to 0
    IO_reg IO;                               // IO register structure
    memset(&IO, 0, sizeof(IO));              // Initialize IO register structure to 0

    // Initialize IRQ2 and memory arrays
    init_irq2(irqp, irq2);   // Initialize IRQ2 array from the input file
    init(meminp, memory, 4096);  // Initialize memory array from the input file
    init(diskinp, disk, (128 * 128));  // Initialize disk array from the input file

    // Run the simulator with all initialized data and file pointers
    simulator(meminp, memeoutp, regoutp, tracep, hwregtracep, cyclesp, ledsp, display7segp, monitorp,
        reg, memory, IO, irq2, disk, monitoryuvp, diskoutp);


    // Closes all the files
    fclose(meminp);
    fclose(diskinp);
    fclose(irqp);
    fclose(memeoutp);
    fclose(regoutp);
    fclose(tracep);
    fclose(hwregtracep);
    fclose(cyclesp);
    fclose(ledsp);
    fclose(display7segp);
    fclose(diskoutp);
    fclose(monitorp);
    fclose(monitoryuvp);

    return 0;
}