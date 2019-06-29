#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "disassembler.h"
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

int main(int argc, char **argv) {

  FILE *machineCode, *outputFile;
  long currAddr = 0;
  int currInstr = -1;
  int *nextBytes = (int *)malloc(9 * sizeof(int));

  // Verify that the command line has an appropriate number
  // of arguments.

  if (argc < 2 || argc > 4) {
    fprintf(stderr,
            "Usage: %s InputFilename [OutputFilename] [startingOffset]\n",
            argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it
  // for reading and verify that the open did occur.
  machineCode = fopen(argv[1], "rb");

  if (machineCode == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }

  // Second argument is the file to write, attempt to open it for
  // writing and verify that the open did occur. Use standard output
  // if not provided.
  outputFile = argc <= 2 ? stdout : fopen(argv[2], "w");

  if (outputFile == NULL) {
    fprintf(stderr, "Failed to open %s: %s\n", argv[2], strerror(errno));
    fclose(machineCode);
    return ERROR_RETURN;
  }

  // If there is a 3rd argument present it is an offset so convert it
  // to a numeric value.
  if (4 == argc) {
    errno = 0;
    currAddr = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      fclose(machineCode);
      fclose(outputFile);
      return ERROR_RETURN;
    }
  }

  fprintf(stderr, "Opened %s, starting offset 0x%lX\n", argv[1], currAddr);
  fprintf(stderr, "Saving output to %s\n",
          argc <= 2 ? "standard output" : argv[2]);

  /**
   * Set the address to the offset in the stream, then get the first byte of the
   * stream. Call getFirstNonZero to jump ahead to the first real instruction.
   **/
  fseek(machineCode, currAddr, SEEK_SET);
  currInstr = fgetc(machineCode);
  int isFirstPosFlag = 1;
  getFirstNonZero(machineCode, &currAddr, &currInstr, outputFile,
                  isFirstPosFlag);

  while (1) {
    if (feof(machineCode)) {
      break;
    }
    // continue to validate instructions until you hit the end of the file
    // stream
    validateInstr(machineCode, &currAddr, &currInstr, nextBytes, outputFile);
  }

  // free the memory allocated for nextBytes, close the read file, close the
  // write file.
  free(nextBytes);
  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}

// forwards throught the byte stream until it sees the first non-zero
// byte. Updates the currInstr with the new instruction and currAddr
// with the address of the instruction.
// Breaks if it hits the end of file.
void getFirstNonZero(FILE *machineCode, long *currAddr, int *currInstr,
                     FILE *outputFile, int isFirstPosFlag) {
  if (*currInstr > 0) {
    // *currAddr = ftell(machineCode);
    return;
  } else {
    while (*currInstr == 0) {
      if (feof(machineCode)) {
        break;
      }
      *currAddr = ftell(machineCode);
      *currInstr = fgetc(machineCode);
    }
    if (*currInstr > 0) {
      printPos(outputFile, *currAddr, isFirstPosFlag);
      return;
    }
  }
  return;
}

// gets the necessary bytes for a given icode and stores them into nextBytes.
void getNextBytes(FILE *machineCode, int bytes, long *currAddr,
                  int *nextBytes) {
  for (int i = 0; i < bytes; i++) {
    nextBytes[i] = fgetc(machineCode);
  }
  return;
}

/**
 * In validateInstr() we switch cases based on the bigNibble. This is the
 * most-significant half-byte of the current instruction, and corresponds to the
 * instruction iCode. From each case we call a corresponding iCode Handler to
 * further validate the instruction, checking the littleByte and additional
 * values.
 *
 * After the handlers return we call ftell() to update the current address, and
 * fgetc to advance to the next instruction.
 **/
void validateInstr(FILE *machineCode, long *currAddr, int *currInstr,
                   int *nextBytes, FILE *outputFile) {
  int bigNibble = *currInstr & 0xF0;
  int littleNibble = *currInstr & 0x0F;

  switch (bigNibble) {
  case 0x00:
    haltHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    *currAddr = ftell(machineCode);
    *currInstr = fgetc(machineCode);
    getFirstNonZero(machineCode, currAddr, currInstr, outputFile, 0);
    return;
    // in 0x00 we return early because we do not want to call ftell and fgetc
    // again. we update the address and instruction and hand off to
    // getFirstNonZero() to get the next valid instruction/value.
  case 0x10:
    nopHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x20:
    switch (littleNibble) {
    case 0x0:
      rrMovQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
      break;
    default:
      cMovHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
      break;
    }
    break;
  case 0x30:
    irMovQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x40:
    rmMovQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x50:
    mrMovQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x60:
    OpQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x70:
    jmpHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x80:
    callHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0x90:
    retHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0xA0:
    pushQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  case 0xB0:
    popQHandler(machineCode, littleNibble, nextBytes, currAddr, outputFile);
    break;
  default:
    break;
  }
  /**
   * update the address and get the next instruction from the
   * byte stream. store both into their corresponding values.
   * validateInstr() will be called again as long as it has not hit the
   * end of the file.
   **/
  *currAddr = ftell(machineCode);
  *currInstr = fgetc(machineCode);
  return;
}

/**
 * iCode Handlers
 *
 * Each handler is responsible for validating the bytes and half-bytes
 * associated with each instruction. Each handler will fetch the necessary
 * number of new bytes for a given instruction (most often 1 byte for rA and
 * rB), and check that the littleNibble and next byte are valid for a given
 * iCode.
 *
 * If the next bytes are not valid for the given instruction, the handlers will
 * call getNextBytes to fetch enough bytes to try and read a quad and call
 * invalidInstr(), passing it the bigNibble and littleNibble, additional bytes,
 * etc.
 *
 * Typical scenario:
 *  1 byte (currInstr) is represented as bigNibble & littleNibble. These are the
 *  half-bytes of the current instruction. 1 byte = n1 and n2; the half-bytes of
 *  the next instruction. In the case of an invalid instruction we'll fetch
 *  enough bytes so that the total is 8.
 **/

// IRMOVQ
void irMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;

  if (littleNibble != 0 || n1 != 0xF0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 3, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 8, address, nextBytes);
    printIRMovQ(outputFile, registerTwo(n2), 3, littleNibble, nextBytes, 8);
  }
}

// RMMOVQ
void rmMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;

  if (littleNibble != 0 || n1 > 0xE0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 4, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 8, address, nextBytes);
    printRMMovQ(outputFile, registerOne(n1), registerTwo(n2), 4, littleNibble,
                nextBytes, 8);
  }
}

// MROVQ
void mrMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;

  if (littleNibble != 0 || n1 > 0xE0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 5, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 8, address, nextBytes);
    printMRMovQ(outputFile, registerTwo(n2), registerOne(n1), 5, littleNibble,
                nextBytes, 8);
  }
}

// OPQ (arithmatic instrs)
void OpQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;

  if (littleNibble > 0x6 || n1 > 0xE0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 6, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    printOpQ(outputFile, arithmInstruction(littleNibble), registerOne(n1),
             registerTwo(n2), 6, littleNibble, nextBytes, 1);
  }
}

// JMPXX
void jmpHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile) {
  if (littleNibble > 0x6) {
    getNextBytes(machineCode, 7, address, nextBytes);
    invalidInstr(machineCode, 7, littleNibble, nextBytes, address, -1, -1,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 8, address, nextBytes);
    printJXX(outputFile, jmpInstruction(littleNibble), 7, littleNibble,
             nextBytes, 8);
  }
}

// CMOVXX
void cMovHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;
  if (littleNibble > 0x6 || n1 > 0xE0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 2, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    printCMovXX(outputFile, cmovInstruction(littleNibble), registerOne(n1),
                registerTwo(n2), 2, littleNibble, nextBytes, 1);
  }
}

// RRMOVQ
void rrMovQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                   long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;
  if (littleNibble != 0x0 || n1 > 0xE0 || n2 > 0xE) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 2, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    printRRMovQ(outputFile, registerOne(n1), registerTwo(n2), 2, littleNibble,
                nextBytes, 1);
  }
}

// CALLXX
void callHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile) {
  if (littleNibble != 0x0) {
    getNextBytes(machineCode, 7, address, nextBytes);
    invalidInstr(machineCode, 8, littleNibble, nextBytes, address, -1, -1,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 8, address, nextBytes);
    printCall(outputFile, 8, littleNibble, nextBytes, 8);
  }
}

// PUSHQ
void pushQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                  long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;
  if (littleNibble != 0x0 || n1 > 0xE0 || n2 != 0x0F) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 0xa, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    printPushQ(outputFile, registerOne(n1), 0xA, littleNibble, nextBytes, 1);
  }
}

// POPQ
void popQHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile) {
  getNextBytes(machineCode, 1, address, nextBytes);
  int n1 = nextBytes[0] & 0xF0;
  int n2 = nextBytes[0] & 0x0F;
  if (littleNibble != 0x0 || n1 > 0xE0 || n2 != 0x0F) {
    getNextBytes(machineCode, 6, address, nextBytes);
    invalidInstr(machineCode, 0xb, littleNibble, nextBytes, address, n1, n2,
                 outputFile);
    return;
  } else {
    printPopQ(outputFile, registerOne(n1), 0xB, littleNibble, nextBytes, 1);
  }
}

// RET
void retHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile) {
  if (littleNibble != 0x0) {
    getNextBytes(machineCode, 7, address, nextBytes);
    invalidInstr(machineCode, 9, littleNibble, nextBytes, address, -1, -1,
                 outputFile);
    return;
  } else {
    printRet(outputFile);
  }
}

// HALT
void haltHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                 long *address, FILE *outputFile) {
  if (littleNibble != 0x0) {
    getNextBytes(machineCode, 7, address, nextBytes);
    invalidInstr(machineCode, 0, littleNibble, nextBytes, address, -1, -1,
                 outputFile);
    return;
  } else {
    getNextBytes(machineCode, 1, address, nextBytes);
    printHalt(outputFile, nextBytes, address);
  }
}

// NOP
void nopHandler(FILE *machineCode, int littleNibble, int *nextBytes,
                long *address, FILE *outputFile) {
  if (littleNibble != 0x0) {
    getNextBytes(machineCode, 7, address, nextBytes);
    invalidInstr(machineCode, 1, littleNibble, nextBytes, address, -1, -1,
                 outputFile);
    return;
  } else {
    printNop(outputFile);
  }
}

// check if the starting address % 8 is zero. If it is, try and read a quad
// otherwise print a byte, then read the next instruction.
void invalidInstr(FILE *machineCode, int bigNibble, int littleNibble,
                  int *nextBytes, long *address, int n1, int n2,
                  FILE *outputFile) {
  int quad = 0;
  if (*address % 8 == 0) {
    if (feof(machineCode)) {
      quad = 0;
    } else {
      quad = 1;
    }
    if (quad) {
      printQuad(outputFile, bigNibble, littleNibble, n1, n2, nextBytes);
    }
  } else {
    printByte(outputFile, bigNibble, littleNibble);
  }
}

// return the corresponding arithmatic operator
char *arithmInstruction(int instruction) {
  switch (instruction) {
  case 0:
    return "addq";
  case 1:
    return "subq";
  case 2:
    return "andq";
  case 3:
    return "xorq";
  case 4:
    return "mulq";
  case 5:
    return "divq";
  case 6:
    return "modq";
  default:
    return "";
  }
}

// return corresponding conditional jump instruction
char *jmpInstruction(int instruction) {
  switch (instruction) {
  case 0:
    return "jmp";
  case 1:
    return "jle";
  case 2:
    return "jl";
  case 3:
    return "je";
  case 4:
    return "jne";
  case 5:
    return "jge";
  case 6:
    return "jg";
  default:
    return "";
  }
}

// return the corresponding conditional move instruction
char *cmovInstruction(int instruction) {
  switch (instruction) {
  case 1:
    return "cmovle";
  case 2:
    return "cmovl";
  case 3:
    return "cmove";
  case 4:
    return "cmovne";
  case 5:
    return "cmovge";
  case 6:
    return "cmovg";
  default:
    return "";
  }
}

// return the corresponding register for r1
// (we can simplify this by bitshifting r1, but not a priority right now)
char *registerOne(int registerNumber) {
  switch (registerNumber) {
  case 0x0:
    return "%rax";
  case 0x10:
    return "%rcx";
  case 0x20:
    return "%rdx";
  case 0x30:
    return "%rbx";
  case 0x40:
    return "%rsp";
  case 0x50:
    return "%rbp";
  case 0x60:
    return "%rsi";
  case 0x70:
    return "%rdi";
  case 0x80:
    return "%r8";
  case 0x90:
    return "%r9";
  case 0xA0:
    return "%r10";
  case 0xB0:
    return "%r11";
  case 0xC0:
    return "%r12";
  case 0xD0:
    return "%r13";
  case 0xE0:
    return "%r14";
  default:
    return "";
  }
}

// return the corresponding register for r2
// (again the bitshifting comment. can be simplified - not a priority)
char *registerTwo(int registerNumber) {
  switch (registerNumber) {
  case 0x0:
    return "%rax";
  case 0x1:
    return "%rcx";
  case 0x2:
    return "%rdx";
  case 0x3:
    return "%rbx";
  case 0x4:
    return "%rsp";
  case 0x5:
    return "%rbp";
  case 0x6:
    return "%rsi";
  case 0x7:
    return "%rdi";
  case 0x8:
    return "%r8";
  case 0x9:
    return "%r9";
  case 0xA:
    return "%r10";
  case 0xB:
    return "%r11";
  case 0xC:
    return "%r12";
  case 0xD:
    return "%r13";
  case 0xE:
    return "%r14";
  default:
    return "";
  }
}
