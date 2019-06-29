#include "printRoutines.h"
#include <stdio.h>
#include <unistd.h>

/*
  Print routines corresponding to each instruction in Y-86 assembly
*/

// print for .pos directive
int printPos(FILE *out, long address, int isFirstPosFlag) {
  int res = 0;
  if (isFirstPosFlag == 1) {
    return fprintf(out, ".pos 0x%lx\n", address);
  } else
    return res += fprintf(out, "\n.pos 0x%lx\n", address);
}

// print for halt
int printHalt(FILE *out, int *nextBytes, long *address) {
  int res = 0;
  if (nextBytes[0] < 0) {
    return 0;
  }
  res += fprintf(out, "    %-8s", "halt");
  res += fprintf(out, "%s\n", "       # 00");
  return res;
}

// print for nop
int printNop(FILE *out) {
  int res = 0;
  res += fprintf(out, "    %-8s", "nop");
  res += fprintf(out, "%s\n", "       # 10");
  return res;
}

// print for ret
int printRet(FILE *out) {
  int res = 0;
  res += fprintf(out, "    %-8s", "ret");
  res += fprintf(out, "%s\n", "       # 90");
  return res;
}

// print for RRMovQ
int printRRMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded) {
  int res = 0;
  res += fprintf(out, "    %-8s%s, %s", "rrmovq", r1, r2);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for IRMovQ
int printIRMovQ(FILE *out, char *reg, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded) {
  int res = 0;
  unsigned long value = getInstructionValue(nextBytes, 0);
  res += fprintf(out, "    %-8s$0x%lx, %s", "irmovq", value, reg);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for RMMovQ
int printRMMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded) {
  int res = 0;
  unsigned long disp = getInstructionValue(nextBytes, 0);
  res += fprintf(out, "    %-8s%s, 0x%lx(%s)", "rmmovq", r1, disp, r2);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for MRMovQ
int printMRMovQ(FILE *out, char *r1, char *r2, int bigNibble, int littleNibble,
                int *nextBytes, int bytesNeeded) {
  int res = 0;
  unsigned long disp = getInstructionValue(nextBytes, 0);
  res += fprintf(out, "    %-8s0x%lx(%s), %s", "mrmovq", disp, r1, r2);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for OpQ
int printOpQ(FILE *out, char *instr, char *r1, char *r2, int bigNibble,
             int littleNibble, int *nextBytes, int bytesNeeded) {
  int res = 0;
  res += fprintf(out, "    %-8s%s, %s", instr, r1, r2);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for JMPXX
int printJXX(FILE *out, char *jmpInstr, int bigNibble, int littleNibble,
             int *nextBytes, int bytesNeeded) {
  int res = 0;
  unsigned long dest = getInstructionValue(nextBytes, 0);
  res += fprintf(out, "    %-8s0x%lx", jmpInstr, dest);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for CMOVXX
int printCMovXX(FILE *out, char *instr, char *r1, char *r2, int bigNibble,
                int littleNibble, int *nextBytes, int bytesNeeded) {
  int res = 0;
  res += fprintf(out, "    %-8s%s, %s", instr, r1, r2);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for CALL
int printCall(FILE *out, int bigNibble, int littleNibble, int *nextBytes,
              int bytesNeeded) {
  int res = 0;
  unsigned long dest = getInstructionValue(nextBytes, 0);
  res += fprintf(out, "    %-8s0x%lx", "call", dest);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for PUSHQ
int printPushQ(FILE *out, char *r1, int bigNibble, int littleNibble,
               int *nextBytes, int bytesNeeded) {
  int res = 0;
  res += fprintf(out, "    %-8s%s", "pushq", r1);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for POPQ
int printPopQ(FILE *out, char *r1, int bigNibble, int littleNibble,
              int *nextBytes, int bytesNeeded) {
  int res = 0;
  res += fprintf(out, "    %-8s%s", "popq", r1);
  res += commentHandler(bigNibble, littleNibble, nextBytes, bytesNeeded, out);
  return res;
}

// print for .byte 0x0
int printByte(FILE *out, int bigNibble, int littleNibble) {
  return fprintf(out, "     %-8s0x%x%x\n", ".byte", bigNibble, littleNibble);
}

// print for .quad 0x0
int printQuad(FILE *out, int bigNibble, int littleNibble, int n1, int n2,
              int *nextBytes) {
  int res = 0;
  // the first case is to print quads when we have NOT previously fetched a
  // lone byte, i.e. an invalid jump instruction, n1 and n2 were passed as -1,
  // so we know we have to read 7 bytes to make a quad.
  if (n1 < 0 || n2 < 0) {
    res += fprintf(out, "    %-8s0x", ".quad");
    // loop backwards through our nextBytes for endian purposes
    for (int i = 6; i >= 0; i--) {
      if (nextBytes[i] != 0) {
        res += fprintf(out, "%x", nextBytes[i]);
      }
    }
    res += fprintf(out, "%x%x", bigNibble, littleNibble);
    int comment = fprintf(out, "            # %X%X", bigNibble, littleNibble);
    for (int i = 0; i < 6; i++) {
      comment += fprintf(out, "%X", nextBytes[i]);
    }
    comment += fprintf(out, "\n");
    res += comment;
  }
  // when we have already fetched an additional byte (such as irmovq), we
  // include n1 and n2, and read 6 more bytes to make a quad.
  else {
    res += fprintf(out, "    %-8s0x", ".quad");

    for (int i = 5; i >= 0; i--) {
      if (nextBytes[i] != 0) {
        res += fprintf(out, "%x", nextBytes[i]);
      }
    }
    res += fprintf(out, "%x%x%x%x", n1, n2, bigNibble, littleNibble);
    int comment =
        fprintf(out, "            # %X%X%X%X", bigNibble, littleNibble, n1, n2);
    for (int i = 0; i < 6; i++) {
      comment += fprintf(out, "%X", nextBytes[i]);
    }
    comment += fprintf(out, "\n");
    res += comment;
  }
  return res;
}

// handles creating comments for every instruction. called from within
// instruction-specific print functions.
int commentHandler(int bigNibble, int littleNibble, int *nextBytes,
                   int bytesNeeded, FILE *out) {
  int comment = 0;
  comment += fprintf(out, "%s", "            # ");
  comment += fprintf(out, "%X", bigNibble);
  comment += fprintf(out, "%X", littleNibble);

  for (int i = 0; i < bytesNeeded; i++) {
    comment += fprintf(out, "%02X", nextBytes[i]);
  }
  comment += fprintf(out, "\n");
  return comment;
}

// Get the Dest, Dist, or Value from the next 8 bytes of the instruction for
// printing Start reading at appropriate place from the next bytes based on the
// instruction
// @params: int *nextBytes, int startPos
// @returns: int next8Bytes;
unsigned long getInstructionValue(int *nextBytes, int startPos) {
  int j = 0;
  unsigned long next8Bytes = 0;
  for (int i = startPos; i < (startPos + 8); i++) {
    next8Bytes += (unsigned long)nextBytes[i] << 8 * j;
    j++;
  }
  return next8Bytes;
}
