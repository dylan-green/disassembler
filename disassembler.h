#ifndef _DISASSEMBLER_H_
#define _DISASSEMBLER_H_

#include <stdio.h>

void getNextBytes(FILE *, int bytes, long *currAddr, int *instr);

void getFirstNonZero(FILE *, long *currAddr, int *currInstr, FILE *outputFile,
                     int isFirstPosFlag);

void validateInstr(FILE *, long *currAddr, int *instruction, int *nextBytes,
                   FILE *outputFile);

// Handlers to manage each instruction case separately
void irMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile);
void rmMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile);
void mrMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile);
void OpQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile);
void jmpHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile);
void cMovHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile);
void rrMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile);
void callHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile);
void pushQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                  long *address, FILE *outputFile);
void popQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile);

// halt, ret, and nop are unique since their 1-byte instructions
void haltHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile);
void retHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile);
void nopHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile);

void invalidInstr(FILE *machineCode, int bigNibble, int littleNibble,
                  int *nextBytes, long *address, int n1, int n2,
                  FILE *outputFile);

// functions to return corresponding conditional
// jump, move, and arithmatic instructions (as char*)
char *arithmInstruction(int instruction);
char *cmovInstruction(int instruction);
char *jmpInstruction(int instruction);
// registerOne gets rA. registerTwo rB. This is because we've done a
// logical & on the first byte (instr & 0xF0; instr & 0xF) to isolate
// each register value. Could fix by bit-shifting. Low priority.
char *registerOne(int registerNumber);
char *registerTwo(int registerNumber);

#endif /* DISASSEMBLER */
