#include "mips.h"

#define PRINT_MIPS 1

#if PRINT_MIPS
#include <stdio.h>
#define PRINT(...) do { printf("0x%08X ", pc); printf(__VA_ARGS__); } while (0)
#else
#define PRINT(...)
#endif

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

// FIXME, replace by simple printf
static const char* ft_str(const unsigned int reg) {
  switch (reg) {
    case 0:  return "$f0";
    case 1:  return "$f1";
    case 2:  return "$f2";
    case 3:  return "$f3";
    case 4:  return "$f4";
    case 5:  return "$f5";
    case 6:  return "$f6";
    case 7:  return "$f7";
    case 8:  return "$f8";
    case 9:  return "$f9";
    case 10: return "$f10";
    case 11: return "$f11";
    case 12: return "$f12";
    case 13: return "$f13";
    case 14: return "$f14";
    case 15: return "$f15";
    case 16: return "$f16";
    case 17: return "$f17";
    case 18: return "$f18";
    case 19: return "$f19";
    case 20: return "$f20";
    case 21: return "$f21";
    case 22: return "$f22";
    case 23: return "$f23";
    case 24: return "$f24";
    case 25: return "$f25";
    case 26: return "$f26";
    case 27: return "$f27";
    case 28: return "$f28";
    case 29: return "$f29";
    case 30: return "$f30";
    case 31: return "$f31";
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
  const char* rs_str = reg_str(inst.rs);
  const char* rt_str = reg_str(inst.rt);
  const char* rd_str = reg_str(inst.rd);
  switch(inst.funct) {
    case 0:
      if (inst.rs != 0) return false;
      // If the shift value of this instruction is 0,
      // the assembler may treats this instruction as NOP.
      if (inst.shamt == 0) PRINT("%-8s\n", "NOP");
      else PRINT("%-8s %s, %s, %u\n", "sll", rd_str, rt_str, inst.shamt);
      break;
    case 2:
      if (inst.rs != 0) return false;
      PRINT("%-8s %s, %s, %u\n", "srl", rd_str, rt_str, inst.shamt);
      break;
    case 3:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, %u\n", "sra", rd_str, rt_str, inst.shamt);
      break;
    case 4:
      if (inst.shamt != 0) return false;
      // If the shift value of this instruction is 0,
      // the assembler may treats this instruction as NOP.
      if (inst.rs == 0) PRINT("%-8s\n", "NOP");
      else PRINT("%-8s %s, %s, %s\n", "sllv", rd_str, rt_str, rs_str);
      break;
    case 6:
      if (inst.shamt != 0) return false;
      PRINT("%-8s %s, %s, %s\n", "srlv", rd_str, rt_str, rs_str);
      break;
    case 7:
      if ((inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "srav", rd_str, rt_str, rs_str);
      break;
    case 8:
      if ((inst.rd & inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "jr", rs_str);
      break;
    case 9:
      if ((inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "jalr", rs_str);
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
      PRINT("%-8s %s\n", "mfhi", rd_str);
      break;
    case 17:
      if ((inst.rt & inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mthi", rs_str);
      break;
    case 18:
      if ((inst.rs & inst.rt & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mflo", rd_str);
      break;
    case 19:
      if ((inst.rt & inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s\n", "mtlo", rs_str);
      break;
    case 20:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsllv", rd_str, rt_str, rs_str);
      break;
    case 22:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsrlv", rd_str, rt_str, rs_str);
      break;
    case 23:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsrav", rd_str, rt_str, rs_str);
      break;
    case 24:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s\n", "mult", rs_str, rt_str);
      break;
    case 25:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s\n", "multu", rs_str, rt_str);
      break;
    case 26:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "div", rd_str, rs_str, rt_str);
      break;
    case 27:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "divu", rd_str, rs_str, rt_str);
      break;
    case 28:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dmult", rd_str, rs_str, rt_str);
      break;
    case 29:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dmultu", rd_str, rs_str, rt_str);
      break;
    case 30:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "ddiv", rd_str, rs_str, rt_str);
      break;
    case 31:
      if ((inst.rd & inst.shamt) != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "ddivu", rd_str, rs_str, rt_str);
      break;
    case 32:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "add", rd_str, rs_str, rt_str);
      break;
    case 33:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "addu", rd_str, rs_str, rt_str);
      break;
    case 34:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "sub", rd_str, rs_str, rt_str);
      break;
    case 35:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "subu", rd_str, rs_str, rt_str);
      break;
    case 36:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "and", rd_str, rs_str, rt_str);
      break;
    case 37:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "or", rd_str, rs_str, rt_str);
      break;
    case 38:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "xor", rd_str, rs_str, rt_str);
      break;
    case 39:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "nor", rd_str, rs_str, rt_str);
      break;
    case 42:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "slt", rd_str, rs_str, rt_str);
      break;
    case 43:
      PRINT("%-8s %s, %s, %s\n", "sltu", rd_str, rs_str, rt_str);
      break;
    case 44:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dadd", rd_str, rs_str, rt_str);
      break;
    case 45:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "daddu", rd_str, rs_str, rt_str);
      break;
    case 46:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsub", rd_str, rs_str, rt_str);
      break;
    case 47:
      if (inst.shamt != 0x0) return false;
      PRINT("%-8s %s, %s, %s\n", "dsubu", rd_str, rs_str, rt_str);
      break;
    case 48:
      PRINT("%-8s %s, %s\n", "tge", rs_str, rt_str);
      break;
    case 49:
      PRINT("%-8s %s, %s\n", "tgeu", rs_str, rt_str);
      break;
    case 50:
      PRINT("%-8s %s, %s\n", "tlt", rs_str, rt_str);
      break;
    case 51:
      PRINT("%-8s %s, %s\n", "tltu", rs_str, rt_str);
      break;
    case 54:
      PRINT("%-8s %s, %s\n", "tne", rs_str, rt_str);
      break;
    case 52:
      PRINT("%-8s %s, %s\n", "teq", rs_str, rt_str);
      break;
    case 56:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsll", rd_str, rt_str, inst.shamt);
      break;
    case 58:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsrl", rd_str, rt_str, inst.shamt);
      break;
    case 59:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsra", rd_str, rt_str, inst.shamt);
      break;
    case 60:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsll32", rd_str, rt_str, inst.shamt);
      break;
    case 62:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsrl32", rd_str, rt_str, inst.shamt);
      break;
    case 63:
      if (inst.rs != 0x0) return false;
      PRINT("%-8s %s, %s, 0x%x\n", "dsra32", rd_str, rt_str, inst.shamt);
      break;
    default:
      PRINT("NOT IMPLEMENTED\n");
      break;
  }
  return true;
}

static const char* bc_str(const uint8_t bc) {
  switch (bc) {
    case 0:  return "bc1f";
    case 1:  return "bc1t";
    case 2:  return "bc1fl";
    case 3:  return "bc1tl";
    default: return NULL;
  }
}
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

bool handle_i(const uint32_t pc, const struct IType inst) {
  // FIXME, don't always retrieve these
  const char* rs_str = reg_str(inst.rs);
  const char* rt_str = reg_str(inst.rt);
  switch (inst.opcode) {
      case 1:
      { // FIXME
        const char* regimm = regimm_str(inst.rt);
        if (regimm == NULL) return false;
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", regimm_str(inst.rt), rs_str, offset);
        break;
      }
      case 4:
      {
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, 0x%08X\n", "beq", rs_str, rt_str, offset);
        break;
      }
      case 5: // FIXME, handle bnez, wrong offset
      {
        int32_t offset = (int32_t) inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, 0x%08X\n", "bne", rs_str, rt_str, offset);
        break;
      }
      case 6:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "blez", rs_str, offset);
        break;
      }
      case 7:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "bgtz", rs_str, offset);
        break;
      }
      case 8: // FIXME
      {
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, %d\n", "addi", rt_str, rs_str, (int32_t)offset);
        break;
      }
      case 9: // FIXME
        PRINT("%-8s %s, %s, %u\n", "addiu", rt_str, rs_str, inst.immediate);
        break;
      case 10:
        PRINT("%-8s %s, %s, %d\n", "slti", rt_str, rs_str, (int32_t) inst.immediate);
        break;
      case 11:
        PRINT("%-8s %s, %s, %d\n", "sltiu", rt_str, rs_str, inst.immediate);
        break;
      case 12:
        PRINT("%-8s %s, %s, %u\n", "andi", rt_str, rs_str, inst.immediate);
        break;
      case 13:
        PRINT("%-8s %s, %s, %u\n", "ori", rt_str, rs_str, inst.immediate);
        break;
      case 14:
        PRINT("%-8s %s, %s, %u\n", "xori", rt_str, rs_str, inst.immediate);
        break;
      case 15:
        if (inst.rs != 0x0) return false;
        PRINT("%-8s %s, %u\n", "lui", rt_str, inst.immediate);
        break;
      case 16:
        // FIXME, bad 2nd value
        if ((inst.immediate >> 6) == 0 && inst.rt == 0 && inst.rs == 0x10) {
          const uint32_t last_six = (inst.immediate & 0x3F);
          if (last_six == 0x1) {
            PRINT("%-8s\n", "tlbr"); break;
          } else if (last_six == 0x2) {
            PRINT("%-8s\n", "tlbwi"); break;
          } else if (last_six == 0x6) {
            PRINT("%-8s\n", "tlbwr"); break;
          } else if (last_six == 0x8) {
            PRINT("%-8s\n", "tlbp"); break;
          } else if (last_six == 0x18) {
            PRINT("%-8s\n", "eret"); break;
          }
        }
        if (inst.rs == 0) PRINT("%-8s %s, %s\n", "mfc0", rt_str, reg_str(inst.immediate >> 11));
        else if (inst.rs == 4) PRINT("%-8s %s, %s, %u\n", "mtc0", rt_str, rs_str, inst.rs);
        else return false;
        break;
      case 17:
      {
        // FIXME
        // move to difference function to handle floating arithmetic
        if (inst.rs == 0x8) {
          const char* bc = bc_str(inst.rt & 0x3);
          if (bc != NULL) {
            int32_t offset = inst.immediate;
            offset = offset << 2;
            offset = offset + pc + 4;          
            PRINT("%-8s 0x%0x\n", bc, offset);
            break;
          }
        }

        const uint32_t last_six = (inst.immediate & 0x3F);
        if (last_six == 0x0 && inst.rs > 6) {
          PRINT("%-s%-4c %s, %s, %s\n", "add.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11), ft_str(inst.rt));
          break;
        }
        if (last_six == 0x1) {
          PRINT("%-s%-4c %s, %s, %s\n", "sub.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11), ft_str(inst.rt));
          break;
        }
        if (last_six == 0x2) { // FIXME
          PRINT("%-s%-4c %s, %s, %s\n", "mul.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11), ft_str(inst.rt));
          break;
        }
        if (last_six == 0x3) { // FIXME
          PRINT("%-s%-4c %s, %s, %s\n", "div.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11), ft_str(inst.rt));
          break;
        }
        if (last_six == 0x4 && inst.rt == 0) {
          PRINT("%-s%-3c %s, %s\n", "sqrt.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        if (last_six == 0x6 && inst.rt == 0) {
          PRINT("%-s%-4c %s, %s\n", "trunc.l.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        if (last_six == 0x7 && inst.rt == 0) {
          PRINT("%-s%-4c %s, %s\n", "neg.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        if (last_six == 0x9 && inst.rt == 0) {
          PRINT("%-s%-4c %s, %s\n", "neg.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        if (last_six == 0xd && inst.rt == 0) {
          PRINT("%-s%-4c %s, %s\n", "trunc.w.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        if (last_six == 0x21 && inst.rt == 0) {
          PRINT("%-s%-2c %s, %s\n", "cvt.d.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        // CVT.L.fmt
        if (last_six == 0x25 && inst.rt == 0) {
          PRINT("%-s%-2c %s, %s\n", "cvt.l.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        // CVT.S.fmt
        if (last_six == 0x20 && inst.rt == 0) {
          PRINT("%-s%-2c %s, %s\n", "cvt.s.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        // CVT.W.fmt
        if (last_six == 0x24 && inst.rt == 0) {
          PRINT("%-s%-2c %s, %s\n", "cvt.w.", fmt_str(inst.rs), ft_str((inst.immediate >> 6) & 0x3F), ft_str(inst.immediate >> 11));
          break;
        }
        // C.cond.fmt
        if (((inst.immediate >> 4) & 0xF) == 0x3) {
          PRINT("%-s%s.%-3c %s, %s\n", "c.", cond_str(inst.immediate & 0xF), fmt_str(inst.rs), ft_str(inst.immediate >> 11), ft_str(inst.rt));
          break;
        }

        // else last 6 bits must be 0
        if ((inst.immediate & 0x7FF) != 0) return false;
        if (inst.rs == 0)      PRINT("%-8s %s, %s\n", "mfc1", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 2) PRINT("%-8s %s, %s\n", "cfc1", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 4) PRINT("%-8s %s, %s\n", "mtc1", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 6) PRINT("%-8s %s, $%d\n", "ctc1", rt_str, inst.immediate >> 11);
        else return false;
        break;
      }
      case 18:
        // FIXME, bad 2nd value
        if ((inst.immediate & 0x7FF) != 0) return false;
        if (inst.rs == 0)      PRINT("%-8s %s, %s\n", "mfc2", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 2) PRINT("%-8s %s, %s\n", "cfc2", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 4) PRINT("%-8s %s, %s\n", "mtc2", rt_str, ft_str(inst.immediate >> 11));
        else if (inst.rs == 6) PRINT("%-8s %s, $%d\n", "ctc2", rt_str, inst.immediate >> 11);
        else return false;        
        break;
      case 20: // FIXME, wrong offset
      {
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        // If it does not branch, the instruction in the branch delay slot is discarded.
        PRINT("%-8s %s, %s, 0x%08X\n", "beql", rs_str, rt_str, offset);
        break;
      }
      case 21: // FIXME, wrong offset, update program counter
      {
        int32_t offset = (int32_t)inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        // If it does not branch, the instruction in the branch delay slot is discarded.
        PRINT("%-8s %s, %s, 0x%08X\n", "bnel", rs_str, rt_str, offset);
        break;
      }
      case 22:
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "blezl", rs_str, offset);
        break;
      }
      case 23: // FIXME, wrong offset
      {
        if (inst.rt != 0x0) return false;
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, 0x%08X\n", "bgtzl", rs_str, offset);
        break;
      }
      case 24: //FIXME
      {
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, %d\n", "daddi", rt_str, rs_str, (int32_t)offset);
        break;
      }
      case 25: //FIXME
      {
        int32_t offset = inst.immediate;
        offset = offset << 2;
        offset = offset + pc + 4;
        PRINT("%-8s %s, %s, %d\n", "daddiu", rt_str, rs_str, (int32_t)offset);
        break;
      }
      case 27:
        PRINT("%-8s %s, 0x%x(%s)\n", "ldr", rt_str, inst.immediate, rs_str);
        break;
      case 32:
        PRINT("%-8s %s, 0x%x(%s)\n", "lb", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 33:
        PRINT("%-8s %s, 0x%x(%s)\n", "lh", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 34:
        PRINT("%-8s %s, (%s)\n", "lwl", rt_str, rs_str);
        break;
      case 35: // FIXME, bad offset
        PRINT("%-8s %s, 0x%x(%s)\n", "lw", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 36:
        PRINT("%-8s %s, 0x%x(%s)\n", "lbu", rt_str, inst.immediate, rs_str);
        break;
      case 37:
        PRINT("%-8s %s, 0x%x(%s)\n", "lhu", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 38:
        PRINT("%-8s %s, 0x%x(%s)\n", "lwr", rt_str, (int32_t) inst.immediate,rs_str);
        break;
      case 39:
        PRINT("%-8s %s, 0x%x(%s)\n", "lwu", rt_str, (int32_t) inst.immediate,rs_str);
        break;
      case 40: // FIXME, can be negative
        PRINT("%-8s %s, 0x%x(%s)\n", "sb", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 41:
        PRINT("%-8s %s, 0x%x(%s)\n", "sh", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 42:
        PRINT("%-8s %s, (%s)\n", "swl", rt_str, rs_str);
        break;
      case 43:
        PRINT("%-8s %s, 0x%x(%s)\n", "sw", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 44: // FIXME sign
        PRINT("%-8s %s, 0x%x(%s)\n", "sdl", rt_str, inst.immediate, rs_str);
        break;
      case 45: // FIXME sign
        PRINT("%-8s %s, 0x%x(%s)\n", "sdr", rt_str, inst.immediate, rs_str);
        break;
      case 46:
        PRINT("%-8s %s, 0x%x(%s)\n", "swr", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 47:
        PRINT("%-8s 0x%x, (%s)\n", "cache", (int32_t) inst.rt, rs_str);
        break;
      case 48:
        PRINT("%-8s %s, 0x%x(%s)\n", "ll", rt_str, inst.immediate, rs_str);
        break;
      case 49:
        PRINT("%-8s %s, 0x%x(%s)\n", "lwc1", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 50:
        PRINT("%-8s %s, 0x%x(%s)\n", "lwc2", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 52:
        PRINT("%-8s %s, 0x%x(%s)\n", "lld", rt_str, inst.immediate, rs_str);
        break;
      case 53:
        PRINT("%-8s %s, 0x%x(%s)\n", "ldc1", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 54:
        PRINT("%-8s %s, 0x%x(%s)\n", "ldc2", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 55:
        PRINT("%-8s %s, 0x%x(%s)\n", "ld", rt_str, inst.immediate, rs_str);
        break;
      case 56: // FIXME sign
        PRINT("%-8s %s, 0x%x(%s)\n", "sc", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 57:
        PRINT("%-8s %s, 0x%x(%s)\n", "swc1", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 58:
        PRINT("%-8s %s, 0x%x(%s)\n", "swc2", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 60: // FIXME sign
        PRINT("%-8s %s, 0x%x(%s)\n", "scd", rt_str, (int32_t) inst.immediate, rs_str);
        break;
      case 61:
        PRINT("%-8s %s, 0x%x(%s)\n", "sdc1", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 62:
        PRINT("%-8s %s, 0x%x(%s)\n", "sdc2", ft_str(inst.rt), inst.immediate, rs_str);
        break;
      case 63: // FIXME sign
        PRINT("%-8s %s, 0x%x(%s)\n", "sd", rt_str, inst.immediate, rs_str);
        break;
      default:
        PRINT("NOT IMPLEMENTED\n");
        return false;
  }
  return true;
}

bool handle_j(const uint32_t pc, const struct JType inst) {
  uint32_t target = inst.target;
  target = target << 2;
  target = target | (0x10000000 & pc);

  PRINT("%-8s 0x%08X\n", inst.opcode == 2 ? "j" : "jal", target);
  return true;
}
