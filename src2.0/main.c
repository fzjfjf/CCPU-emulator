// ============================================
//                  INCLUDES
// ============================================
#include "input.h"
#include <stdint.h>
#include <stdbool.h> //NOLINT - had to put this because it was annoying me it grayed it out
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================
//                    MACROS
// ============================================
#define MAGIC_BIOS 0x42494F53

// ============================================
//             GLOBAL DEFINITIONS
// ============================================

// ===== ENUMS =====
typedef enum {
    R0, R1, R2, R3, R4,
    RA, RB, RC, RD, RE, RF,
    RSP,
    REG_COUNT
} REGS;

typedef enum {
    ZF, CF, OF, SF,
    AF_COUNT
} ARIT_FLAGS; //NOLINT - disable spelling check

typedef enum {
    EF, GF, LW,
    CF_COUNT
} CMP_FLAGS;

enum {
    NOP, ADD, SUB, MUL, DIV,
    MOV, LOAD, STORE, LOADB, STOREB, 
    AND, NOT, OR, XOR, RSHIFT, LSHIFT,
    CMP, JMP, JMPE, JMPNE, JMPG, JMPGE, JMPL, JMPLE,
    PUSH, POP, CALL, RET, INT, IRET, HALT
}

enum {
    REG_REG, REG_IMM, ADDR, REG_ADDR
}

// ===== STRUCTS =====
typedef struct {
    uint32_t regs[REG_COUNT];
    uint32_t pc;
    uint32_t ic;                //NOLINT - Instruction counter
    bool arit_flags[AF_COUNT];  //NOLINT - disable spelling check
    bool comp_flags[CF_COUNT];
    uint8_t memory[0x100000];
} CPU;

// ===== FUNCTION DECLARATIONS =====
uint8_t* open_file(char *file_name);
bool execute_instruction(uint8_t opcode, uint8_t type, uint8_t reg1, uint32_t immediate, CPU *cpu);
void alu(char op, uint8_t reg1, uint32_t immediate, uint8_t type, CPU *cpu);

// ===== GLOBAL VARIABLES =====
size_t file_size = 0;



int main(int argc, char *argv[]) {
    CPU cpu = {0};
    
    // Try to read the file
    if (argc < 2) {
        puts("Usage:\nCCPU_Emulator: file.cexe");
        return -1;
    }

    uint8_t *buffer = open_file(argv[1]);
    if (buffer == NULL) {
        puts("ERROR: Could not read file provided");
        free(buffer);
        return -1;
    }

    // Check for header
    if (buffer[0] != 0x43 || buffer[1] != 0x45 || buffer[2] != 0x58 || buffer[3] != 0x45) {
        puts("ERROR: File is invalid");
        free(buffer);
        return -1;
    }

    // Read address
    uint32_t address = (uint32_t)buffer[7] |
                       (uint32_t)(buffer[6] << 8) |
                       (uint32_t)(buffer[5] << 16) |
                       (uint32_t)(buffer[4] << 24);
    
    // Check if address is within bounds
    if (address == MAGIC_BIOS) {};  // Special case for BIOS if provided

    if (address >= 0x01100 && address < 0xEFFFF) {
        cpu.pc = address;
    } else { 
        puts("ERROR: Invalid address");
        free(buffer);
        return -10;
    }
    
    if ((size_t)file_size + (size_t)address > 0xFEFFF) {
        puts("ERROR: File is too big");
        free(buffer);
        return -11;
    }

    // Put file into ram
    for (long i = 8; i < file_size; i++, address++) {
        cpu.memory[address] = buffer[i];
    }

    free(buffer); // Clear the buffer IMMEDIATELY after reading the file
    
    bool is_running = true;
    uint8_t opcode = 0;
    uint8_t type = 0;
    uint8_t reg1 = 0;
    uint32_t immediate = 0;

    time_t timeOnStart = time(NULL);
    while (is_running) {
        opcode = cpu.memory[cpu.pc];
        type = cpu.memory[cpu.pc+1];
        reg1 = cpu.memory[cpu.pc+3];
        immediate = (uint32_t)(cpu.memory[cpu.pc+4] << 24) |
                    (uint32_t)(cpu.memory[cpu.pc+5] << 16) |
                    (uint32_t)(cpu.memory[cpu.pc+6] << 8) |
                    (uint32_t)(cpu.memory[cpu.pc+7]);

        cpu.pc += 8; 
        is_running = execute_instruction(opcode, type, reg1, immediate, &cpu);
        cpu.ic += 1;
    }
 
    return 0;
}



// ===== FUNCTION DEFINITIONS =====
uint8_t* open_file(char *file_name) {
    FILE *file = fopen(file_name, "rb");
    uint8_t *buffer = NULL; //NOLINT
    bool running = true;

    if (file == NULL) {
        puts("FILE DOESNT EXIST");
        return NULL;    //NOLINT
    }

    fseek(file, 0, SEEK_END);

    file_size = ftell(file);
    if (file_size < 0) {
        puts("Failed to get file size");
        fclose(file);
        return NULL;    //NOLINT
    }

    rewind(file);

    buffer = malloc((size_t)file_size);
    if (!buffer) {
        puts("malloc failed");
        fclose(file);
        return NULL;    //NOLINT
    }

    fread(buffer, 1, (size_t)file_size, file);
    
    fclose(file);
    return buffer;
}

void alu(char op, uint8_t reg1, uint32_t immediate, uint8_t type, CPU *cpu) {
    uint32_t a = cpu->regs[reg1];
    uint32_t b = 0;
    uint32_t result = 0;
    uint64_t big_res = 0;
    if (type == REG_REG) {b = cpu->regs[immediate];}
    if (type == REG_IMM) {b = immediate;} 

    // Clear flags
    for (int i = 0; i < 4; i++) cpu->arit_flags[i] = false;

    switch (op) {
        case '+':
            result = a + b;
            if (result < a) cpu->arit_flags[CF] = true;
            if ((a >> 31) == (b >> 31) && (result >> 31) != (a >> 31)) cpu->arit_flags[OF] = true;
            a = result;
            break;
        case '-':
            result = a - b;
            if (a < b) cpu->arit_flags[CF] = true;  
            if ((a >> 31) != (b >> 31) && (result >> 31) != (a >> 31)) cpu->arit_flags[OF] = true;
            a = result;
            break;
        case '*':
            big_res = (uint64_t)a * (uint64_t)b;
            if (big_res > 0xFFFFFFFF) {
                cpu->arit_flags[CF] = true;
                cpu->arit_flags[OF] = true;
            }
            a = big_res & 0xFFFFFFFF;
            break;
        case '/':
            uint32_t mod = a % b;
            a = a / b;
            cpu->regs[immediate] = mod;
            break;
        case '&':
            a = a & b;
            break;
        case '|':
            a = a | b;
            break;
        case '^':
            a = a ^ b;
            break;
        case '~':
            a = ~a;
            break;
        case '<':
            a = a << b;
            break;
        case '>':
            a = a >> b;
            break;
        default:
            puts("ERROR: Invalid ALU call!");
            break;
    }
    
    cpu->regs[reg1] = a;

    if (a == 0) cpu->arit_flags[ZF] = true;
    if ((a >> 31) == 1) cpu->arit_flags[SF] = true;
}

bool execute_instruction(uint8_t opcode, uint8_t type, uint8_t reg1, uint32_t immediate, CPU *cpu) {
    /* 
     * 
     * 
     */ 

    switch (opcode) {
        case NOP:
            return true;
        case ADD:
            if (type == REG_REG) {
                cpu->regs[reg1] += cpu->regs[immediate];
            } else if (type == REG_IMM) {
                cpu->regs[reg1] += immediate;
            }

            return true;
        case SUB:
            if (type == REG_REG)
    }
   
}
