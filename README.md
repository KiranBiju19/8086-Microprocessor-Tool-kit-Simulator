# 8086-Microprocessor
8086 Microprocessor Instruction Simulator

Welcome to the 8086 Microprocessor Instruction Simulator, a C-based implementation that emulates the behavior of basic 8086 instructions. This program is designed for enthusiasts, students, and developers who want to understand and experiment with the functionality of 8086 assembly instructions in a simulated environment.

Features

Supported Instructions

Data Transfer: MOV, PUSH, POP, XCHG

Arithmetic Operations: ADD, SUB, INC, DEC, MUL, DIV

Logical and Bit Manipulation: ROL, ROR, RCL, RCR

Flag Handling: Automatically updates flags (CF, ZF, SF, etc.) for operations

Simulation Highlights

Implements general-purpose registers (AX, BX, CX, DX) and pointer/index registers (SP, BP, SI, DI)

Memory management through an array simulating memory (memory[100])

Easy-to-extend modular codebase

Getting Started

Prerequisites

A C compiler (e.g., GCC or Clang)

Basic knowledge of 8086 assembly language

Installation

Clone the repository:

git clone https://github.com/your-username/8086-simulator.git

Navigate to the project directory:

cd 8086-simulator

Compile the program:

gcc project86.c -o simulator

Run the executable:

./simulator

Usage

Example Instructions

Below are some example models that demonstrate the functionality of the simulator:

Data Transfer

MOV AX, 1234H   ; Move immediate data into AX
MOV [BX], AX    ; Move AX into memory location pointed by BX
PUSH AX         ; Push AX onto the stack
POP BX          ; Pop the value from the stack into BX

Arithmetic Operations

ADD AX, BX      ; Add BX to AX
SUB CX, 0001H   ; Subtract immediate value from CX
INC DX          ; Increment DX by 1
DEC AX          ; Decrement AX by 1

Logical/Bit Manipulation

ROL AX, 1       ; Rotate AX left by 1 bit
ROR BX, 2       ; Rotate BX right by 2 bits

Advanced Example

MOV AX, 0003H   ; Load immediate data into AX
ADD AX, [SI]    ; Add value at memory location pointed by SI to AX
MOV [BX], AX    ; Store AX at memory location pointed by BX

Testing

Example Test Cases

The following test cases help validate the simulator's functionality:

Data Transfer Validation:

MOV AX, 0x1234
MOV [10H], AX
MOV BX, [10H]

Expected Result: BX = 0x1234

Arithmetic Operations:

MOV AX, 0x0001
ADD AX, 0x0002

Expected Result: AX = 0x0003

Logical Operations:

MOV AX, 0x0F0F
ROL AX, 4

Expected Result: AX = 0xF0F0

Flag Testing:

MOV AX, 0xFFFF
INC AX

Expected Result: AX = 0x0000, ZF = 1

Running Test Cases

You can hardcode the above instructions into the program or use the existing modular functions to simulate them dynamically.

Guidelines for Coding with the Simulator

Instruction Syntax

Ensure proper case and format for instructions, e.g., MOV AX, 1234H.

Separate operands with a comma.

Use hexadecimal notation for immediate values (e.g., 1234H).

Memory Access

Ensure memory indices are within bounds (0 <= index < 100).

Use valid registers or pointers for indirect memory access.

Debugging

Print register states using printf after each operation to verify intermediate values.

Use modular functions for better readability and debugging.

Extending the Simulator

Adding New Instructions: Add logic to the corresponding function (e.g., data_transfer, arithmetic).

Improving Flag Handling: Update flags in each operation as per 8086 behavior.

Future Enhancements

String Manipulation Instructions: Add support for MOVS, LODS, STOS.

Control Flow Instructions: Implement JMP, CALL, RET, and conditional jumps.

Interactive User Input: Allow users to input instructions at runtime.

Contributing

Contributions are welcome! Feel free to open issues or submit pull requests to improve the simulator.

License

This project is licensed under the MIT License.
