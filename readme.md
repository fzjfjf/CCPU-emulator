# CCPU

## About

CCPU is a cpu emulator written in C. Uses a custom ISA. 

## Contribution guidelines

To contribute, please start PRs on the indev branch and not on the main branch. Use of AI to write code is not allowed. 

## Structure

CCPU has:
- 12 Registers, of which:
  - `R0` is for a return value
  - `R1`-`R4` are for holding arguments
  - `RA`-`RF` are general purpose registers
  - `RSP` which is reserved for the stack  
- 1 MB of RAM memory

## ISA

Below is a table of instructions:  

| Opcode |  Name  | Description                                                                 |  
|:------:|:------:|-----------------------------------------------------------------------------|  
|  0x00  |  ADD   | Adds two values together. Note: The first argument has to be a register!    |  
|  0x01  |  SUB   | Subtracts two values.                                                       |  
|  0x02  |  MUL   | Multiplies two values.                                                      |  
|  0x03  |  DIV   | Divides two values. The modulus is saved in the second register.             |  
|  0x04  |  MOV   | Moves a value into a register. The value can be an immideate or a register. |   
|  0x05  |  LOAD  | Loads a value from memory.                                                  |  
|  0x06  | STORE  | Stores a value in memory.                                                   |   
|  0x07  |  AND   | Does a bitwise and operation on two values.                                 |  
|  0x08  |  NOT   | Does a bitwise not operation on a value.                                    |  
|  0x09  |   OR   | Does a bitwise or operation on two values.                                  |  
|  0x0A  |  XOR   | Does a bitwise xor operation on two values.                                 |  
|  0x0B  |  CMP   | Compares two values and sets EF and GF accordingly.                         |  
|  0x0C  |  JMP   | Does an unconditional jump.                                                 |  
|  0x0D  |  JMPE  | Jumps if equal.                                                             |  
|  0x0E  | JMPNE  | Jumps if not equal.                                                         |  
|  0x0F  |  JMPG  | Jumps if greater.                                                           |  
|  0x10  |  JMPL  | Jumps if lower.                                                             |  
|  0x11  |  HALT  | Halts the CPU.                                                              |   
|  0x12  |  INT   | Does an interrupt.                                                          |  
|  0x13  |  IRET  | Returns from an interrupt.                                                  |  
|  0x14  | LOADB  | Loads one byte.                                                             |
|  0x15  | STOREB | Stores one byte.                                                            |
|  0x16  |  CALL  | Pushes the return address to the stack and jumps to a label.                |
|  0x17  |  RET   | Pops the return address from the stack and jumps to it.                     |
|  0x18  |  PUSH  | Pushes a value to the stack,                                                |
|  0x19  |  POP   | Pops the value last pushed to the stack.                                    |

Below is a table of registers:  

| Opcode |  Name  | Usage           |  
|:------:|:------:|:----------------|  
|  0x0   |   R0   | Return value    |   
|  0x1   |   R1   | Argument 1      |  
|  0x2   |   R2   | Argument 2      |  
|  0x3   |   R3   | Argument 3      |  
|  0x4   |   R4   | Argument 4      |  
|  0x5   |   RA   | General purpose |  
|  0x6   |   RB   | General purpose |  
|  0x7   |   RC   | General purpose |  
|  0x8   |   RD   | General purpose |  
|  0x9   |   RE   | General purpose |  
|  0xA   |   RF   | General purpose |  
|  0xB   |  RSP   | Stack pointer   |  

## .cexe

CCPU uses ```.cexe``` executable files. The header of a ```.cexe``` looks like:
``` hexdump
43 45 58 45  XX XX XX XX 
```
`XX XX XX XX` represents the address at which the program is to be loaded in memory.  

All instructions are 8 bytes long:  

```OP TY 00 R1  00 00 00 00```
- `OP` - Contains the opcode of the instruction  
- `TY` - Determines if the last four bytes are a register, immediate or an address
- `00` - Padding
- `R1` - The first register
- `00 00 00 00` - Contains either an address in memory, a register opcode or an immediate value.  
  Note: `TY` determines what do the bytes represent

## Software interrupts

CCPU currently supports these interrupts:
- `0x0` - Used to put characters on console  
  Setup: 
  - `R1` needs to have the address of the text
  - `R2` needs the length of the text
- `0x1` - Used to get input from user  
  - `R0` will have the lenght of the input
  - `R1` needs to have the address of the buffer
  - `R2` needs to have the length of the buffer
- `0x2` - Used to get the uptime in seconds  
  Note: The return value is stored in `R0`  
- `0x3` - Used to get the time since the Unix epoch  
  Note: The return value is stored in `R0`  
- `0x4` - Used to get the current instruction cycle  
  Note: The return value is stored in `R0`  
