#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //NOLINT
#include <time.h>

enum {
    REG_REG,
    REG_ADDR,
    REG_IMM,
};
enum {
    R0, R1, R2, R3, R4,
    RA, RB, RC, RD, RE, RF,
    RSP
};
enum {
    ADD, SUB, MUL, DIV,
    MOV, LOAD, STORE,
    AND, NOT, OR, XOR,
    CMP, JMP, JMPE, JMPNE, JMPG, JMPL,
    HALT, INT, IRET,
    LOADB, STOREB,
    CALL, RET, PUSH, POP
};

typedef struct {
    uint8_t opcode;
    uint8_t type;
    uint8_t register1;
    uint32_t arg2;
} Instruction;

uint32_t PC = 0x0;
uint32_t registers[12] = {0};
uint8_t memory[0x100000] = {0}; // 0x10000 is exactly one MB of memory
bool EF = false;    // Equal Flag
bool GF = false;    // Greater Flag
time_t timeOnStart = 0;
uint32_t cycle = 0;

int interpreter(Instruction instruction);
int push(uint8_t type, uint8_t arg1, uint32_t arg2);
uint32_t pop();

int main(int argc, char *argv[]) {

    if (argc < 2) {
        puts("No file provided!");
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");
    uint8_t *buffer = NULL; //NOLINT
    bool running = true;

    if (file == NULL) {
        puts("FILE DOESNT EXIST");
        return -1;
    }

    timeOnStart = time(NULL); //NOLINT

    fseek(file, 0, SEEK_END);

    long file_size = ftell(file);
    if (file_size < 0) {
        puts("Failed to get file size");
        fclose(file);
        return -1;
    }

    rewind(file);

    buffer = malloc((size_t)file_size);
    if (!buffer) {
        puts("malloc failed");
        fclose(file);
        return -1;
    }

    fread(buffer, 1, (size_t)file_size, file);
    // Check for header
    if (buffer[0] != 0x43 || buffer[1] != 0x45 || buffer[2] != 0x58 || buffer[3] != 0x45) {
        puts("Invalid file");
        free(buffer);
        return -1;
    }

    // Read address
    uint32_t address = (uint32_t)buffer[7] +
                       (uint32_t)(buffer[6] << 8) +
                       (uint32_t)(buffer[5] << 16) +
                       (uint32_t)(buffer[4] << 24);
    PC = address;

    // Put file into ram
    // Check if file was too big
    if ((long long)sizeof(memory) < file_size + address) {
        puts("File is too big");
        free(buffer);
        return 2;
    }

    for (long i = 8; i < file_size; i++, address++) {
        memory[address] = buffer[i];
    }

    free(buffer); // Clear the buffer IMMEDIATELY after reading the file

    // Initialize the stack pointer
    registers[RSP] = 0xFFFFF;  // The stack begins at the start
    // Make an instruction variable
    Instruction instruction;
    
    while (running) {
        instruction.opcode = memory[PC];
        instruction.type = memory[PC+1];
        instruction.register1 = memory[PC+3];
        instruction.arg2 = (uint32_t)memory[PC+7] |
                        (uint32_t)(memory[PC+6] << 8) |
                        (uint32_t)(memory[PC+5] << 16) |
                        (uint32_t)(memory[PC+4] << 24);
        PC = PC + 8;

        if (interpreter(instruction) != 0) {
            running = false;
        }

        cycle++;

    }

    return 0;
}

int interpreter(Instruction instruction) {

    uint32_t value = 0;

    switch (instruction.opcode) {
        case ADD:
            printf("ADD\n");

            if (instruction.type == 0) {
                // Both arguments are registers
                registers[instruction.register1] = registers[instruction.register1] + registers[instruction.arg2];
            } else if (instruction.type == 1) {
                // First argument is a register, second one is immideate
                registers[instruction.register1] = registers[instruction.register1] + instruction.arg2;
            }

            break;
        case SUB:
            printf("SUB\n");

            if (instruction.type == 0) {
                // Both arguments are registers
                registers[instruction.register1] = registers[instruction.register1] - registers[instruction.arg2];
            } else if (instruction.type == 1) {
                // First argument is a register, second one is immideate
                registers[instruction.register1] = registers[instruction.register1] - instruction.arg2;
            }

            break;
        case MUL:
            printf("MUL\n");

            if (instruction.type == 0) {
                // Both arguments are registers
                registers[instruction.register1] = registers[instruction.register1] * registers[instruction.arg2];
            } else if (instruction.type == 1) {
                // First argument is a register, second one is immideate
                registers[instruction.register1] = registers[instruction.register1] * instruction.arg2;
            }

            break;
        case DIV:
            printf("DIV\n");
            
            if (instruction.type == 0) {
                // Both arguments are registers
                value = registers[instruction.register1] / registers[instruction.arg2];
                registers[instruction.arg2] = registers[instruction.register1] % registers[instruction.arg2];
                registers[instruction.register1] = value;
            } else if (instruction.type == 1) {
                // First argument is a register, second one is immideate
                registers[instruction.register1] = registers[instruction.register1] / instruction.arg2;
            }

            break;
        case MOV:
            printf("MOV\n");

            if (instruction.type == 0) {
                // Both arguments are registers
                registers[instruction.register1] = registers[instruction.arg2];
            } else if (instruction.type == 1) {
                // First argument is a register, second one is immediate
                registers[instruction.register1] = instruction.arg2;
            }

            break;
        case LOAD:
            printf("LOAD\n");

            // Big endian encoding
            registers[instruction.register1] = (uint32_t)memory[instruction.arg2] << 24 |
                                               (uint32_t)memory[instruction.arg2+1] << 16 |
                                               (uint32_t)memory[instruction.arg2+2] << 8 |
                                               (uint32_t)memory[instruction.arg2+3];

            break;
        case STORE:
            printf("STORE\n");

            //Big endian encoding
            value = registers[instruction.register1];
            memory[instruction.arg2] = (uint8_t)(value>>24) &0xFF;
            memory[instruction.arg2+1] = (uint8_t)(value>>16) &0xFF;
            memory[instruction.arg2+2] = (uint8_t)(value>>8) &0xFF;
            memory[instruction.arg2+3] = (uint8_t)(value) &0xFF;

            break;
        case LOADB:
            puts("LOADB");

            registers[instruction.register1] = memory[instruction.arg2];

            break;
        case STOREB:
            puts("STOREB");

            value = registers[instruction.register1];
            memory[instruction.arg2] = value & 0xFF;

            break;
        case AND:
            printf("AND\n");

            if (instruction.type == 0) {
                registers[instruction.register1] = registers[instruction.register1] & registers[instruction.arg2];
            } else {
                registers[instruction.register1] = registers[instruction.register1] & instruction.arg2;
            }

            break;
        case NOT:
            printf("NOT\n");

            registers[instruction.register1] = ~registers[instruction.register1];

            break;
        case OR:
            printf("OR\n");

            if (instruction.type == 0) {
                registers[instruction.register1] = registers[instruction.register1] | registers[instruction.arg2];
            } else {
                registers[instruction.register1] = registers[instruction.register1] | instruction.arg2;
            }

            break;
        case XOR:
            printf("XOR\n");

            if (instruction.type == 0) {
                registers[instruction.register1] = registers[instruction.register1] ^ registers[instruction.arg2];
            } else {
                registers[instruction.register1] = registers[instruction.register1] ^ instruction.arg2;
            }

            break;
        case CMP:
            printf("CMP\n");

            EF = 0;
            GF = 0;

            if (instruction.type == 0) {
                if (registers[instruction.register1] == registers[instruction.arg2]) {
                    EF = 1;
                    GF = 0;
                } else if (registers[instruction.register1] < registers[instruction.arg2]) {
                    GF = 0;
                    EF = 0;
                } else if (registers[instruction.register1] > registers[instruction.arg2]) {
                    GF = 1;
                    EF = 0;
                }
            } else {
                if (registers[instruction.register1] == instruction.arg2) {
                    EF = 1;
                    GF = 0;
                } else if (registers[instruction.register1] < instruction.arg2) {
                    GF = 0;
                    EF = 0;
                } else if (registers[instruction.register1] > instruction.arg2) {
                    GF = 1;
                    EF = 0;
                }
            }

            printf("FLAGS: EF: %i GF %i\n", EF, GF);

            break;
        case JMP:
            printf("JMP\n");
            PC = instruction.arg2;
            break;
        case JMPE:
            printf("JMPE\n");

            // Jumps if EF = 1
            if (EF == 1 && GF == 0) {
                PC = instruction.arg2;
            }

            break;
        case JMPNE:
            printf("JMPNE\n");

            // Jumps if EF = 0
            if (EF == 0) {
                printf("INSIDE JMPNE, FLAGS: %i %i, PC: 0x%x, arg2: 0x%x\n", EF, GF, PC, instruction.arg2);
                PC = instruction.arg2;
                printf("PC: 0x%x, arg2: 0x%x\n", PC, instruction.arg2);
            }

            break;
        case JMPG:
            printf("JMPG\n");

            // CF = 1
            if (GF == 1 && EF == 0) {
                PC = instruction.arg2;
            }

            break;
        case JMPL:
            printf("JMPL\n");

            // CF = 0
            if (GF == 0 && EF == 0) {
                //jump
                PC = instruction.arg2;
            }

            break;
        case HALT:
            printf("HALT\n");
            return 1;
        case INT:
            printf("INT\n");

            switch (instruction.arg2) {
                case 0x0:
                    uint32_t start = registers[R1];
                    uint32_t len   = registers[R2];
                    uint32_t end   = start + len;

                    for (uint32_t i = start; i < end; i++) {
                        printf("%c", memory[i]);
                    }
                    break;
                case 0x1:
                    // Input interrupt
                    fgets((char *)&memory[registers[R1]], (int)registers[R2], stdin); // Ignore the message
                    break;
                case 0x2:
                    // Get time since boot up

                    uint32_t currentTime = (uint32_t)time(NULL); //NOLINT
                    registers[R0] = (uint32_t)(currentTime - timeOnStart);

                    break;
                case 0x3:
                    // Get time since epoch

                    registers[R0] = (uint32_t)time(NULL); //NOLINT

                    break;
                case 0x4:
                    // Get instruction cycle

                    registers[R0] = cycle;

                    break;
                default:
                    puts("Invalid interrupt!");
            }

            break;
        case IRET:
            printf("IRET\n");
            break;
        case PUSH:
            puts("PUSH");

            push(instruction.type, instruction.register1, instruction.arg2);

            break;
        case POP:
            puts("POP");

            registers[instruction.register1] = pop();

            break;
        case CALL:
            printf("CALL\n");

            // Pushes PC to the stack and jumps to the address provided
            push(REG_IMM, 0, PC);
            PC = instruction.arg2;

            break;
        case RET:
            printf("RET");

            // Pops PC from the stack and jumps back
            PC = pop();

            break;
        default:
            printf("Unknown instruction!\n");
            printf("OPCODE: 0x%x; TYPE: 0x%x; REG1: 0x%x; ARG2: 0x%x",
                instruction.opcode, instruction.type, instruction.register1, instruction.arg2);
            break;
    }

    return 0;
}

int push(uint8_t type, uint8_t arg1, uint32_t arg2) {

    // Check if the stack pointer is too close to the end of the memory
    if (registers[RSP] > 0xFFFFF - 0xFF) {
        puts("Stack underflow!");
        return 2;
    }

    uint32_t value = 0;

    switch (type) {
        case REG_REG:
            value = registers[arg1];
            break;
        case REG_IMM:
            value = arg2;
            break;
        default:
            puts("Invalid type!");
            return -1;
    }

    registers[RSP] -= 4;

    memory[registers[RSP]] = (uint8_t)(value >> 24 & 0xFF);
    memory[registers[RSP]+1] = (uint8_t)(value >> 16 & 0xFF);
    memory[registers[RSP]+2] = (uint8_t)(value >> 8 & 0xFF);
    memory[registers[RSP]+3] = (uint8_t)(value & 0xFF);

    return 0;
}

uint32_t pop() {

    // Check if the stack pointer is too high
    if (registers[RSP] > 0xFFFFF - 3) {
        puts("Stack Overflow!");
        return 2;
    }

    uint32_t value =
        (uint32_t)memory[registers[RSP]+3] |
        (uint32_t)memory[registers[RSP]+2] << 8 |
        (uint32_t)memory[registers[RSP]+1] << 16 |
        (uint32_t)memory[registers[RSP]] << 24;

    registers[RSP] += 4;

    return value;
}
