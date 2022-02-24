#include "mips.h"

//#define ENABLE_COPZ

#define PRINT_MIPS

#ifdef PRINT_MIPS
#include <stdio.h>
FILE* f = stdout;
#define PRINT(...) do { fprintf(f,"0x%08X ", pc); fprintf(f,__VA_ARGS__); } while (0)
#else
#define PRINT(...) ;
#endif

void mips_set_file(const char* name) {
#ifdef PRINT_MIPS
  char file_name[128];
  if (snprintf(file_name, 64, "%s.asm", name) < 0) return;
  f = fopen(file_name, "w");
  if (f == NULL) f = stdout;
#else
  (void) name;
#endif
}

void mips_close_file() {
#ifdef PRINT_MIPS
  fclose(f);
  f = stdout;
#endif
}

static const char* cond_str(const unsigned int cond) {
  switch (cond) {
    case 0:  return "f";
    case 1:  return "un";
    case 2:  return "eq";
    case 3:  return "uqe";
    case 4:  return "olt";
    case 5:  return "ult";
    case 6:  return "ole";
    case 7:  return "ule";
    case 8:  return "sf";
    case 9:  return "ngle";
    case 10: return "seq";
    case 11: return "ngl";
    case 12: return "lt";
    case 13: return "nge";
    case 14: return "le";
    case 15: return "ngt";
    default: return NULL;
  }
}

static const char* reg_str(const unsigned int reg) {
  switch (reg) {
    case 0:  return "$zero";
    case 1:  return "$at";
    case 2:  return "$v0";
    case 3:  return "$v1";
    case 4:  return "$a0";
    case 5:  return "$a1";
    case 6:  return "$a2";
    case 7:  return "$a3";
    case 8:  return "$t0";
    case 9:  return "$t1";
    case 10: return "$t2";
    case 11: return "$t3";
    case 12: return "$t4";
    case 13: return "$t5";
    case 14: return "$t6";
    case 15: return "$t7";
    case 16: return "$s0";
    case 17: return "$s1";
    case 18: return "$s2";
    case 19: return "$s3";
    case 20: return "$s4";
    case 21: return "$s5";
    case 22: return "$s6";
    case 23: return "$s7";
    case 24: return "$t8";
    case 25: return "$t9";
    case 26: return "$k0";
    case 27: return "$k1";
    case 28: return "$gp";
    case 29: return "$sp";
    case 30: return "$fp";
    case 31: return "$ra";
    // cannot happen, we handle all the possible 5 bits integer
    default: return NULL;
  }
}

// check for inconditional branch
bool mips_is_b(const struct IType inst) {
  return inst.opcode == 4 && inst.rs == 0 && inst.rt == 0;
}

bool mips_is_j(const struct JType inst) {
  return inst.opcode == 2;
}

bool handle_r(const uint32_t pc, const struct RType inst) {
  switch(inst.funct) {
    case 0:
      if (inst.rs != 0) return false;
      // If the shift value of this instruction is 0,
      // the assembler may treats this instruction as NOP.
      if (inst.shamt == 0) PRINT("%-8s\n", "NOP");
      else PRINT("%-8s %s, %s, %u\n", "sll", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 2:
      if (inst.rs != 0) return false;
      PRINT("%-8s %s, %s, %u\n", "srl", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 3:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, %u\n", "sra", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 4:
      if (inst.shamt != 0) return false;
      // If the shift value of this instruction is 0,
      // the assembler may treats this instruction as NOP.
      if (inst.rs == 0) PRINT("%-8s\n", "NOP");
      else PRINT("%-8s %s, %s, %s\n", "sllv", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 6:
      if (inst.shamt != 0) return false;
      PRINT("%-8s %s, %s, %s\n", "srlv", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 7:
      if ((inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "srav", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 8:
      if ((inst.rd & inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "jr", reg_str(inst.rs));
      break;
    case 9:
      if ((inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "jalr", reg_str(inst.rs));
      break;
    case 12:
      PRINT("%-8s \n", "syscall");
      break;
    case 13:
      PRINT("%-8s %u\n", "break", inst.rs << 5 | inst.rt);
      break;
    case 15:
      if ((inst.rs & inst.rt & inst.rd & inst.shamt) != 0x0) return false;
      // The SYNC instruction is executed as a NOP on the VR4300. This operation
      // maintains compatibility with code that conforms to the VR4400.
      //PRINT("%-8s %u\n", "sync", inst.rs << 5 | inst.rt);
      PRINT("%-8s\n", "NOP");
      break;
    case 16:
      if ((inst.rs & inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mfhi", reg_str(inst.rd));
      break;
    case 17:
      if ((inst.rt & inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mthi", reg_str(inst.rs));
      break;
    case 18:
      if ((inst.rs & inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mflo", reg_str(inst.rd));
      break;
    case 19:
      if ((inst.rt & inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mtlo", reg_str(inst.rs));
      break;
    case 20:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsllv", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 22:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsrlv", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 23:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsrav", reg_str(inst.rd), reg_str(inst.rt), reg_str(inst.rs));
      break;
    case 24:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s\n", "mult", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 25:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s\n", "multu", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 26:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "div", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 27:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "divu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 28:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dmult", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 29:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dmultu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 30:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "ddiv", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 31:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "ddivu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 32:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "add", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 33:
      if (inst.shamt != 0x0) return false;
      if (inst.rt == 0) PRINT("%-8s %s, %s\n", "move", reg_str(inst.rd), reg_str(inst.rs));
      else PRINT("%-8s %s, %s, %s\n", "addu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 34:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "sub", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 35:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "subu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 36:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "and", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 37:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "or", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 38:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "xor", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 39:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "nor", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 42:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "slt", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 43:
      PRINT("%-8s %s, %s, %s\n", "sltu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 44:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dadd", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 45:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "daddu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 46:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsub", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 47:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsubu", reg_str(inst.rd), reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 48:
      PRINT("%-8s %s, %s\n", "tge", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 49:
      PRINT("%-8s %s, %s\n", "tgeu", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 50:
      PRINT("%-8s %s, %s\n", "tlt", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 51:
      PRINT("%-8s %s, %s\n", "tltu", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 54:
      PRINT("%-8s %s, %s\n", "tne", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 52:
      PRINT("%-8s %s, %s\n", "teq", reg_str(inst.rs), reg_str(inst.rt));
      break;
    case 56:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsll", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 58:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsrl", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 59:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsra", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 60:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsll32", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 62:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsrl32", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    case 63:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsra32", reg_str(inst.rd), reg_str(inst.rt), inst.shamt);
      break;
    default:
      PRINT("NOT IMPLEMENTED\n");
      return false;
  }
  return true;
}

static const char* bc_str(const uint8_t bc, const int cop) {
  switch (bc) {
    case 0:
      if      (cop==0) return "bc0f";
      else if (cop==1) return "bc1f";
      else if (cop==2) return "bc2f";
      break;
    case 1:
      if      (cop==0) return "bc0t";
      else if (cop==1) return "bc1t";
      else if (cop==2) return "bc2t";
      break;
    case 2:
      if      (cop==0) return "bc0fl";
      else if (cop==1) return "bc1fl";
      else if (cop==2) return "bc2fl";
      break;
    case 3:
      if      (cop==0) return "bc0tl";
      else if (cop==1) return "bc1tl";
      else if (cop==2) return "bc2tl";
      break;
    default: return NULL;
  }
  return NULL;
}

// copz
static const char* regimm_str(const uint32_t regimm) {
  switch (regimm) {
    case 0:  return "bltz";
    case 1:  return "begz";
    case 2:  return "bltzl";
    case 3:  return "bgezl";
    case 8:  return "tgei";
    case 9:  return "tgeiu";
    case 10: return "tlti";
    case 11: return "tltiu";
    case 12: return "teqi";
    case 14: return "tnei";
    case 16: return "bltzal";
    case 17: return "bgezal";
    case 18: return "bltzall";
    case 19: return "bgezall";
    default: return NULL;
  }
}

static char fmt_str(const uint32_t fmt) {
  switch (fmt) {
    case 16: return 's';
    case 17: return 'd';
    case 20: return 'w';
  }
  return '?';
}

// FIXME, change to Rtype
static bool handle_fpu(const uint32_t pc, const IType inst) {
  // check if Branch On FPU
  if (inst.rs == 0x8) {
    const char* bc = bc_str(inst.rt & 0x3, 1);
    if (bc != NULL) {
      int32_t offset = inst.immediate;
      offset = offset << 2;
      offset = offset + pc + 4;          
      PRINT("%-8s 0x%0x\n", bc, offset);
      return true;
    }
  }
#ifdef ENABLE_COPZ
  if ((inst.rs >> 4) == 0x1) {
    PRINT("%-8s 0x%x\n", "cop1", ((inst.rs & 0xF) << 20) | (inst.rt << 15) | (inst.immediate));
    return true;
  }
#endif
  // check last 6 bits for FPU instructions
  switch (inst.immediate & 0x3F) {
    case 0x0: 
      if (inst.rs > 6) {
        PRINT("%-s%-4c $f%d, $f%d, $f%d\n", "add.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11, inst.rt);
        return true;
      }
      break;
    case 0x1:
        PRINT("%-s%-4c $f%d, $f%d, $f%d\n", "sub.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11, inst.rt);
        return true;
    case 0x2:
      PRINT("%-s%-4c $f%d, $f%d, $f%d\n", "mul.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11, inst.rt);
      return true;
    case 0x3:
      PRINT("%-s%-4c $f%d, $f%d, $f%d\n", "div.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11, inst.rt);
      return true;
    case 0x4:
      if (inst.rt == 0) {
        PRINT("%-s%-3c $f%d, $f%d\n", "sqrt.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x6:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "mov.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x7:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "neg.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x8:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "round.l.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x9:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "trunc.l.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xa:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "ceil.l.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xb:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "floor.l.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xc:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "round.w.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xd:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "trunc.w.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xe:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "ceil.w.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0xf:
      if (inst.rt == 0) {
        PRINT("%-s%-4c $f%d, $f%d\n", "floor.w.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x20:
      if (inst.rt == 0) {
        PRINT("%-s%-2c $f%d, $f%d\n", "cvt.s.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x21:
      if (inst.rt == 0) {
        PRINT("%-s%-2c $f%d, $f%d\n", "cvt.d.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x24:
      if (inst.rt == 0) {
        PRINT("%-s%-2c $f%d, $f%d\n", "cvt.w.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    case 0x25:
      if (inst.rt == 0) {
        PRINT("%-s%-2c $f%d, $f%d\n", "cvt.l.", fmt_str(inst.rs), (inst.immediate >> 6) & 0x3F, inst.immediate >> 11);
        return true;
      }
      break;
    default: break;
  }

  // floating point compare
  if (((inst.immediate >> 4) & 0xF) == 0x3) {
    PRINT("%-s%s.%-3c $f%d, $f%d\n", "c.", cond_str(inst.immediate & 0xF), fmt_str(inst.rs), inst.immediate >> 11, inst.rt);
    return true;
  }

  // else last 3 bits must not be 0
  if ((inst.immediate & 0x7FF) != 0) return false;

  // handle other stuff that's on COP1 10001
  if      (inst.rs == 0) PRINT("%-8s %s, $f%d\n", "mfc1", reg_str(inst.rt), inst.immediate >> 11);
  else if (inst.rs == 2) PRINT("%-8s %s, $f%d\n", "cfc1", reg_str(inst.rt), inst.immediate >> 11);
  else if (inst.rs == 4) PRINT("%-8s %s, $f%d\n", "mtc1", reg_str(inst.rt), inst.immediate >> 11);
  else if (inst.rs == 6) PRINT("%-8s %s, $%d\n",  "ctc1", reg_str(inst.rt), inst.immediate >> 11);
  else return false;

  return true;
}

bool handle_i(const uint32_t pc, const struct IType inst) {
  switch (inst.opcode) {
      case 1:
      {
        const char* regimm = regimm_str(inst.rt);
        if (regimm == NULL) return false;
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", regimm_str(inst.rt), reg_str(inst.rs), offset);
        break;
      }
      case 4:
      {
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, 0x%08X\n", "beq", reg_str(inst.rs), reg_str(inst.rt), offset);
        break;
      }
      case 5:
      {
        int32_t offset = (int32_t) inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        if (inst.rt == 0) PRINT("%-8s %s, 0x%08X\n", "bnez", reg_str(inst.rs), offset);
        else PRINT("%-8s %s, %s, 0x%08X\n", "bne", reg_str(inst.rs), reg_str(inst.rt), offset);
        break;
      }
      case 6:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "blez", reg_str(inst.rs), offset);
        break;
      }
      case 7:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "bgtz", reg_str(inst.rs), offset);
        break;
      }
      case 8:
        PRINT("%-8s %s, %s, %i\n", "addi", reg_str(inst.rt), reg_str(inst.rs), (int16_t)inst.immediate);
        break;
      case 9:
        PRINT("%-8s %s, %s, %i\n", "addiu", reg_str(inst.rt), reg_str(inst.rs), (int16_t)inst.immediate);
        break;
      case 10:
        PRINT("%-8s %s, %s, %i\n", "slti", reg_str(inst.rt), reg_str(inst.rs), (int16_t) inst.immediate);
        break;
      case 11:
        PRINT("%-8s %s, %s, %i\n", "sltiu", reg_str(inst.rt), reg_str(inst.rs), (int16_t) inst.immediate);
        break;
      case 12:
        PRINT("%-8s %s, %s, 0x%x\n", "andi", reg_str(inst.rt), reg_str(inst.rs), inst.immediate);
        break;
      case 13:
        PRINT("%-8s %s, %s, 0x%x\n", "ori", reg_str(inst.rt), reg_str(inst.rs), inst.immediate);
        break;
      case 14:
        PRINT("%-8s %s, %s, 0x%x\n", "xori", reg_str(inst.rt), reg_str(inst.rs), inst.immediate);
        break;
      case 15:
        if (inst.rs != 0x0) return false;
        PRINT("%-8s %s, 0x%x\n", "lui", reg_str(inst.rt), inst.immediate);
        break;
      case 16:
        // check if Branch On FPU
        if (inst.rs == 0x8) {
          const char* bc = bc_str(inst.rt & 0x3, 0);
          if (bc != NULL) {
            int32_t offset = inst.immediate;
            offset = offset << 2;
            offset = offset + pc + 4;          
            PRINT("%-8s 0x%0x\n", bc, offset);
            return true;
          }
        }
#ifdef ENABLE_COPZ
        if ((inst.rs >> 4) == 0x1) {
          PRINT("%-8s 0x%x\n", "cop0", ((inst.rs & 0xF) << 20) | (inst.rt << 15) | (inst.immediate));
          return true;
        }
#endif
        if ((inst.immediate >> 6) == 0 && inst.rt == 0 && inst.rs == 0x10) {
          // check last 6 bits first
          switch (inst.immediate & 0x3F) {
            case 0x1:  PRINT("%-8s\n", "tlbr");  return true;
            case 0x2:  PRINT("%-8s\n", "tlbwi"); return true;
            case 0x6:  PRINT("%-8s\n", "tlbwr"); return true;
            case 0x8:  PRINT("%-8s\n", "tlbp");  return true;
            case 0x18: PRINT("%-8s\n", "eret");  return true;
            default: break;
          }
        }
        // else last 11 must be 0
        if ((inst.immediate & 0x7FF) != 0) return false;
        if      (inst.rs == 0) PRINT("%-8s %s, %s\n", "mfc0", reg_str(inst.rt), reg_str(inst.immediate >> 11));
        else if (inst.rs == 1) PRINT("%-8s %s, %s\n","dmfc0", reg_str(inst.rt), reg_str(inst.immediate >> 11));
        else if (inst.rs == 4) PRINT("%-8s %s, %s\n", "mtc0", reg_str(inst.rt), reg_str(inst.immediate >> 11));
        else return false;
        break;
      case 17: return handle_fpu(pc, inst);
      case 18:
        // check if Branch On FPU
        if (inst.rs == 0x8) {
          const char* bc = bc_str(inst.rt & 0x3, 2);
          if (bc != NULL) {
            int32_t offset = inst.immediate;
            offset = offset << 2;
            offset = offset + pc + 4;          
            PRINT("%-8s 0x%0x\n", bc, offset);
            return true;
          }
        }
#ifdef ENABLE_COPZ
        if ((inst.rs >> 4) == 0x1) {
          PRINT("%-8s 0x%x\n", "cop2", ((inst.rs & 0xF) << 20) | (inst.rt << 15) | (inst.immediate));
          return true;
        }
#endif
        // else last 11 must be 0
        if ((inst.immediate & 0x7FF) != 0) return false;
        if      (inst.rs == 0) PRINT("%-8s %s, $f%d\n", "mfc2", reg_str(inst.rt), inst.immediate >> 11);
        else if (inst.rs == 2) PRINT("%-8s %s, $f%d\n", "cfc2", reg_str(inst.rt), inst.immediate >> 11);
        else if (inst.rs == 4) PRINT("%-8s %s, $f%d\n", "mtc2", reg_str(inst.rt), inst.immediate >> 11);
        else if (inst.rs == 6) PRINT("%-8s %s, $%d\n",  "ctc2", reg_str(inst.rt), inst.immediate >> 11);
        else return false;        
        break;
      case 20:
      {
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, 0x%08X\n", "beql", reg_str(inst.rs), reg_str(inst.rt), offset);
        break;
      }
      case 21:
      {
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, 0x%08X\n", "bnel", reg_str(inst.rs), reg_str(inst.rt), offset);
        break;
      }
      case 22:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "blezl", reg_str(inst.rs), offset);
        break;
      }
      case 23:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "bgtzl", reg_str(inst.rs), offset);
        break;
      }
      case 24:
      {
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, %i\n", "daddi", reg_str(inst.rt), reg_str(inst.rs), offset);
        break;
      }
      case 25:
      {
        int32_t offset = (int16_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, %i\n", "daddiu", reg_str(inst.rt), reg_str(inst.rs), offset);
        break;
      }
      case 27:
        PRINT("%-8s %s, %i(%s)\n", "ldr", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 32:
        PRINT("%-8s %s, %i(%s)\n", "lb", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 33:
        PRINT("%-8s %s, %i(%s)\n", "lh", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 34:
        PRINT("%-8s %s, %i(%s)\n", "lwl", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 35:
        PRINT("%-8s %s, %i(%s)\n", "lw", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 36:
        PRINT("%-8s %s, %i(%s)\n", "lbu", reg_str(inst.rt),(int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 37:
        PRINT("%-8s %s, %i(%s)\n", "lhu", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 38:
        PRINT("%-8s %s, %i(%s)\n", "lwr", reg_str(inst.rt),(int16_t) inst.immediate,reg_str(inst.rs));
        break;
      case 39:
        PRINT("%-8s %s, %i(%s)\n", "lwu", reg_str(inst.rt), (int16_t) inst.immediate,reg_str(inst.rs));
        break;
      case 40:
        PRINT("%-8s %s, %i(%s)\n", "sb", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 41:
        PRINT("%-8s %s, %i(%s)\n", "sh", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 42:
        PRINT("%-8s %s, %i(%s)\n", "swl", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 43:
        PRINT("%-8s %s, %i(%s)\n", "sw", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 44:
        PRINT("%-8s %s, %i(%s)\n", "sdl", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 45:
        PRINT("%-8s %s, %i(%s)\n", "sdr", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 46:
        PRINT("%-8s %s, %i(%s)\n", "swr", reg_str(inst.rt), (int16_t) inst.immediate, reg_str(inst.rs));
        break;
      case 47:
        PRINT("%-8s %i, %i(%s)\n", "cache", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 48:
        PRINT("%-8s %s, %i(%s)\n", "ll", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 49:
        PRINT("%-8s $f%d, %i(%s)\n", "lwc1", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 50:
        PRINT("%-8s $f%d, %i(%s)\n", "lwc2", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 52:
        PRINT("%-8s %s, %i(%s)\n", "lld", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 53:
        PRINT("%-8s $f%d, %i(%s)\n", "ldc1", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 54:
        PRINT("%-8s $f%d, %i(%s)\n", "ldc2", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 55:
        PRINT("%-8s %s, %i(%s)\n", "ld", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 56:
        PRINT("%-8s %s, %i(%s)\n", "sc", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 57:
        PRINT("%-8s $f%d, %i(%s)\n", "swc1", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 58:
        PRINT("%-8s $f%d, %i(%s)\n", "swc2", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 60:
        PRINT("%-8s %s, %i(%s)\n", "scd", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 61:
        PRINT("%-8s $f%d, %i(%s)\n", "sdc1", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 62:
        PRINT("%-8s $f%d, %i(%s)\n", "sdc2", inst.rt, (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      case 63:
        PRINT("%-8s %s, %i(%s)\n", "sd", reg_str(inst.rt), (int16_t)inst.immediate, reg_str(inst.rs));
        break;
      default:
        PRINT("NOT IMPLEMENTED\n");
        return false;
  }
  return true;
}

bool handle_j(const uint32_t pc, const struct JType inst) {
  uint32_t target = (uint32_t)inst.target;
  // shift left twice
  target = (target & 0xFFFFFF ) << 2;
  // add the first for bits of PC
  target = ((pc+4) & 0xF0000000) | target;

  PRINT("%-8s 0x%08X\n", inst.opcode == 2 ? "j" : "jal", target);
  return true;
}
