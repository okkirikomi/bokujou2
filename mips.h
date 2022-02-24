#pragma once

#include <stdint.h>

/*
  Almost complete MIPS binary parser for the VR4300 CPU.
  This produces ASM code in text format.

  MIPS uses fixed size instructions divided into three types:
  R (register)
  I (immediate)
  J (jump)

  Every instruction starts with a 6-bit opcode.
  In addition to the opcode,
  R-type instructions specify 3 registers, a shift amount field, and a function field;
  I-type instructions specify two registers and a 16-bit immediate value;
  J-type instructions follow the opcode with a 26-bit jump target.

  R   opcode (6)  rs (5)  rt (5)  rd (5)  shamt (5)   funct (6)
  I   opcode (6)  rs (5)  rt (5)  immediate (16)
  J   opcode (6)  address (26)

  Manual:
  https://hack64.net/docs/VR43XX.pdf

  Missing:
- Figure out difference with COPz instructions compared to other disassemblers like Capstone.
*/

struct JType {
  signed int target:26;
  unsigned int opcode:6;
};

struct IType {
  unsigned int immediate:16;
  unsigned int rt:5;
  unsigned int rs:5;
  unsigned int opcode:6;
};

struct RType {
  unsigned int funct:6;
  unsigned int shamt:5;
  unsigned int rd:5;
  unsigned int rt:5;
  unsigned int rs:5;
  unsigned int opcode:6;
};

typedef union Instruction {
  struct RType r;
  struct IType i;
  struct JType j;
} Instruction;

void mips_set_file(const char* name);
void mips_close_file();

bool mips_is_j(const struct JType inst);
bool mips_is_b(const struct IType inst);

bool handle_r(const uint32_t pc, const struct RType inst);
bool handle_i(const uint32_t pc, const struct IType inst);
bool handle_j(const uint32_t pc, const struct JType inst);
