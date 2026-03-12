#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //NOLINT
#include <stdbool.h> //NOLINT
#include  <time.h>

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
};
typedef struct {
    char *name;
    int code;
} Instruction;
typedef struct {
    char *name;
    int reg;
} Register;

uint32_t PC = 0x0;
uint32_t registers[12] = {};
uint8_t memory[0xFFFFF] = {0};
int instruction_codes[22] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
};
bool EF = false;    // Equal Flag
bool GF = false;    // Greater Flag
uint32_t timeOnStart = 0;
uint32_t cycle = 0;

Register register_mapped[] = {
    {"R0", R0}, {"R1", R1}, {"R2", R2}, {"R3", R3}, {"R4", R4},
    {"RA", RA}, {"RB", RB}, {"RC", RC}, {"RD", RD}, {"RE", RE},
    {"RF", RF}, {"RSP", RSP}
};

Instruction instructions_mapped[] = {
    {"ADD", ADD}, {"SUB", SUB}, {"MUL", MUL}, {"DIV", DIV},
    {"MOV", MOV}, {"LOAD", LOAD}, {"STORE", STORE}, {"AND", AND},
    {"NOT", NOT}, {"OR", OR}, {"XOR", XOR}, {"CMP", CMP},
    {"JMP", JMP}, {"JMPE", JMPE}, {"JMPNE", JMPNE}, {"JMPG", JMPG},
    {"JMPL", JMPL}, {"HALT", HALT}, {"INT", INT}, {"IRET", IRET},
    {"LOADB", LOADB}, {"STOREB", STOREB,}
};

int interpreter(uint8_t code, uint8_t type, uint8_t arg1, uint32_t arg2);

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
    const long long file_size = ftell(file);
    rewind(file);

    buffer = malloc(file_size);

    fread(buffer, 1, file_size, file);

    fclose(file);

    // Check for header
    if (buffer[0] != 0x43 || buffer[1] != 0x45 || buffer[2] != 0x58 || buffer[3] != 0x45) {
        puts("Invalid file");
        free(buffer);
        return -1;
    }

    // Read address
    uint32_t address = buffer[7] + (buffer[6] << 8) + (buffer[5] << 16) + (buffer[4] << 24);
    PC = address;

    // Put file into ram
    // Check if file was too big
    if (sizeof(memory) < file_size + address) {
        puts("File is too big");
        free(buffer);
        return 2;
    }

    for (long long i = 8; i < file_size; i++, address++) {
        memory[address] = buffer[i];
    }

    free(buffer); // Clear the buffer IMMEDIATELY after reading the file

    while (running) {
        uint8_t opcode = memory[PC];
        uint8_t type = memory[PC+1];
        uint8_t arg1 = memory[PC+3];
        uint32_t arg2 = (uint32_t)memory[PC+7] |
                        (uint32_t)(memory[PC+6] << 8) |
                        (uint32_t)(memory[PC+5] << 16) |
                        (uint32_t)(memory[PC+4] << 24);
        PC = PC + 8;

        if (interpreter(opcode, type, arg1, arg2) == 1) {
            running = false;
        }

        cycle++;

    }

    return 0;
}

int interpreter(uint8_t code, uint8_t type, uint8_t arg1, uint32_t arg2) {

    switch (code) {
        case ADD:
            printf("ADD\n");

            if (type == 0) {
                // Both arguments are registers
                registers[arg1] = registers[arg1] + registers[arg2];
            } else if (type == 1) {
                // First argument is a register, second one is immideate
                registers[arg1] = registers[arg1] + arg2;
            }

            break;
        case SUB:
            printf("SUB\n");

            if (type == 0) {
                // Both arguments are registers
                registers[arg1] = registers[arg1] - registers[arg2];
            } else if (type == 1) {
                // First argument is a register, second one is immideate
                registers[arg1] = registers[arg1] - arg2;
            }

            break;
        case MUL:
            printf("MUL\n");

            if (type == 0) {
                // Both arguments are registers
                registers[arg1] = registers[arg1] * registers[arg2];
            } else if (type == 1) {
                // First argument is a register, second one is immideate
                registers[arg1] = registers[arg1] * arg2;
            }

            break;
        case DIV:
            printf("DIV\n");

            if (type == 0) {
                // Both arguments are registers
                registers[arg1] = registers[arg1] / registers[arg2];
                registers[arg2] = registers[arg1] % registers[arg2];
            } else if (type == 1) {
                // First argument is a register, second one is immideate
                registers[arg1] = registers[arg1] / arg2;
            }

            break;
        case MOV:
            printf("MOV\n");

            if (type == 0) {
                // Both arguments are registers
                registers[arg1] = registers[arg2];
            } else if (type == 1) {
                // First argument is a register, second one is immediate
                registers[arg1] = arg2;
            }

            break;
        case LOAD:
            printf("LOAD\n");

            // Big endian encoding
            registers[arg1] = (uint32_t)memory[arg2] << 24 |
                              (uint32_t)memory[arg2+1] << 16 |
                              (uint32_t)memory[arg2+2] << 8 |
                              (uint32_t)memory[arg2+3];

            break;
        case STORE:
            printf("STORE\n");

            //Big endian encoding
            uint32_t value = registers[arg1];
            memory[arg2] = (value>>24) &0xFF;
            memory[arg2+1] = (value>>16) &0xFF;
            memory[arg2+2] = (value>>8) &0xFF;
            memory[arg2+3] = (value) &0xFF;

            break;
        case LOADB:
            puts("LOADB");

            registers[arg1] = memory[arg2];

            break;
        case STOREB:
            puts("STOREB");

            value = registers[arg1];
            memory[arg2] = value & 0xFF;

            break;
        case AND:
            printf("AND\n");

            if (type == 0) {
                registers[arg1] = registers[arg1] & registers[arg2];
            } else {
                registers[arg1] = registers[arg1] & arg2;
            }

            break;
        case NOT:
            printf("NOT\n");

            registers[arg1] = ~registers[arg1];

            break;
        case OR:
            printf("OR\n");

            if (type == 0) {
                registers[arg1] = registers[arg1] | registers[arg2];
            } else {
                registers[arg1] = registers[arg1] | arg2;
            }

            break;
        case XOR:
            printf("XOR\n");

            if (type == 0) {
                registers[arg1] = registers[arg1] ^ registers[arg2];
            } else {
                registers[arg1] = registers[arg1] ^ arg2;
            }

            break;
        case CMP:
            printf("CMP\n");

            EF = 0;
            GF = 0;

            if (type == 0) {
                if (registers[arg1] == registers[arg2]) {
                    EF = 1;
                    GF = 0;
                } else if (registers[arg1] < registers[arg2]) {
                    GF = 0;
                    EF = 0;
                } else if (registers[arg1] > registers[arg2]) {
                    GF = 1;
                    EF = 0;
                }
            } else {
                if (registers[arg1] == arg2) {
                    EF = 1;
                    GF = 0;
                } else if (registers[arg1] < arg2) {
                    GF = 0;
                    EF = 0;
                } else if (registers[arg1] > arg2) {
                    GF = 1;
                    EF = 0;
                }
            }

            printf("FLAGS: EF: %i GF %i\n", EF, GF);

            break;
        case JMP:
            printf("JMP\n");
            PC = arg2;
            break;
        case JMPE:
            printf("JMPE\n");

            // Jumps if EF = 1
            if (EF == 1 && GF == 0) {
                PC = arg2;
            }

            break;
        case JMPNE:
            printf("JMPNE\n");

            // Jumps if EF = 0
            if (EF == 0) {
                printf("INSIDE JMPNE, FLAGS: %i %i, PC: 0x%x, arg2: 0x%x\n", EF, GF, PC, arg2);
                PC = arg2;
                printf("PC: 0x%x, arg2: 0x%x\n", PC, arg2);
            }

            break;
        case JMPG:
            printf("JMPG\n");

            // CF = 1
            if (GF == 1 && EF == 0) {
                PC = arg2;
            }

            break;
        case JMPL:
            printf("JMPL\n");

            // CF = 0
            if (GF == 0 && EF == 0) {
                //jump
                PC = arg2;
            }

            break;
        case HALT:
            printf("HALT\n");
            return 1;
        case INT:
            printf("INT\n");

            switch (arg2) {
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
                    fgets(&memory[registers[R1]], registers[R2], stdin); // Ignore the message
                    break;
                case 0x2:
                    // Get time since boot up

                    uint32_t currentTime = time(NULL); //NOLINT
                    registers[R0] = currentTime - timeOnStart;

                    break;
                case 0x3:
                    // Get time since epoch

                    registers[R0] = time(NULL); //NOLINT

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
        default:
            printf("Unknown instruction!\n");
            break;
    }

    return 0;
}