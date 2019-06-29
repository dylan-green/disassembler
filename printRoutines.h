/* This file contains the prototypes and constants needed to use the
   routines defined in printRoutines.c
*/

#ifndef _PRINTROUTINES_H_
#define _PRINTROUTINES_H_

#include <stdio.h>

int printPosition(FILE *, unsigned long);
int printInstruction(FILE *);

int printHalt(FILE *out, int *nextBytes, long *address);
int printNop(FILE *out);
int printRet(FILE *out);
int printPos(FILE *out, long address, int isFirstPosFlag);

unsigned long getInstructionValue(int *nextBytes, int startPos);

int printIRMovQ(FILE *out, char *reg, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded);
int printRMMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded);
int printMRMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded);
int printOpQ(FILE *out, char *instr, char *r1, char *r2, int bigNibble,
             int littleNibble, int *nextBytes, int bytesNeeded);
int printJXX(FILE *out, char *jmpInstr, int bigNibble, int littleNibble,
             int *nextBytes, int bytesNeeded);
int printCMovXX(FILE *out, char *instr, char *r1, char *r2, int bigNibble,
                int littleNibble, int *nextBytes, int bytesNeeded);
int printRRMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded);
int printPushQ(FILE *out, char *r1, int bigNibble, int littleNibble,
               int *nextBytes, int bytesNeeded);
int printPopQ(FILE *out, char *r1, int bigNibble, int littleNibble,
              int *nextBytes, int bytesNeeded);
int printCall(FILE *out, int bigNibble, int littleNibble, int *nextBytes,
              int bytesNeeded);
int printQuad(FILE *out, int bigNibble, int littleNibble, int n1, int n2,
              int *nextBytes);
int printByte(FILE *out, int bigNibble, int littleNibble);
int commentHandler(int bigNibble, int littleNibble, int *nextBytes,
                   int bytesNeeded, FILE *outputFile);

#endif /* PRINTROUTINES */
