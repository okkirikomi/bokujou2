#include "rom.h"

#if defined(__linux__)
# include <endian.h>
#elif defined(__WINDOWS__)
# include <winsock2.h>
# include <sys/param.h>
# if BYTE_ORDER == LITTLE_ENDIAN<
#   define be32toh(x) ntohl(x)
# else
#   define be32toh(x) (x)
# endif
#else
# error "Define be32toh for your platform."
#endif

#include <stdlib.h>
#include <string.h>

#include "crc_check.h"
#include "log.h"
#include "mips.h"
#include "shift_js.h"

// Biggest possible N64 ROM is 512 megabits
static const long MAX_ROM_SIZE = 0x3D09000;
// Smallest possible N64 ROM is 32 megabits
static const long MIN_ROM_SIZE = 0x3D0900;

// ASM starts here
static const uint32_t BOOTCODE_ENDS = 0x1000;

static const char* rom_type_string(const byte b) {
  switch (b) {
    case 'N': return "cart";
    case 'D': return "64DD disk";
    case 'C': return "cartridge part of expandable game";
    case 'E': return "64DD expansion for cart";
    case 'Z': return "Aleck64 cart";
    default: return NULL;
  }
}

static const char* country_string(const byte b) {
  switch (b) {
    case 0x37: return "Beta";
    case 0x41: return "Asian (NTSC)";
    case 0x42: return "Brazilian";
    case 0x43: return "Chinese";
    case 0x44: return "German";
    case 0x45: return "North America";
    case 0x46: return "French";
    case 0x47: return "Gateway 64 (NTSC)";
    case 0x48: return "Dutch";
    case 0x49: return "Italian";
    case 0x4A: return "Japanese";
    case 0x4B: return "Korean";
    case 0x4C: return "Gateway 64 (PAL)";
    case 0x4E: return "Canadian";
    case 0x50: return "European (basic spec.)";
    case 0x53: return "Spanish";
    case 0x55: return "Australian";
    case 0x57: return "Scandinavian";
    case 0x58: return "European";
    case 0x59: return "European";
    default: return NULL;
  }
}

void Rom::unload() {
  free(data);
  free(rom_name);
}

bool Rom::load(const char* path) {
  bool ok = false;
  rom_name = NULL;

  // open the file
  FILE* file = fopen(path, "rb");
  if (!file) {
      LOG_ERROR("Can't open file:%s\n", path);
      return ok;
  }

  // figure out the file size
  if (fseek(file, 0L, SEEK_END) != 0) goto close_file;

  data_size = ftell(file);
  LOG_TRACE("ROM size is: %liMBs\n", data_size / 1024 / 1024);

  if (data_size == -1L) goto close_file;
  if (data_size < MIN_ROM_SIZE) {
    LOG_ERROR("File is too small for a N64 ROM:%lu bytes\n", data_size);
    goto close_file;
  }
  if (data_size > MAX_ROM_SIZE) {
    LOG_ERROR("File is too big for a N64 ROM:%lu bytes\n", data_size);
    goto close_file;
  }

  // load the ROM in memory
  if (fseek(file, 0L, SEEK_SET) != 0) goto close_file;
  data = (byte*) malloc(data_size);
  if (data == NULL) goto close_file;
  if (fread(data, sizeof(byte), data_size, file) != (size_t) data_size) goto unload;

  // sanity check
  if (!check_format()) goto unload;
  if (!parse_header()) goto unload;
  if (!verify_header()) goto unload;
  if (!find_binary()) goto unload;

  ok = true;
  goto close_file;

unload:
  free(data);
  free(rom_name);

close_file:
  fclose(file);
  return ok;
}

static const uint32_t Z64_MAGIC = 0x80371240;
static const uint32_t N64_MAGIC = 0x40123780;
static const uint32_t V64_MAGIC = 0x37804012;

bool Rom::check_format() const {
  uint32_t endianness;
  read((byte*)&endianness, 0x00, 4);
  const uint32_t magic_number = be32toh(endianness);
  switch (magic_number) {
      case Z64_MAGIC: return true;
      case N64_MAGIC: // fallthrough
      case V64_MAGIC: // fallthrough
      default:
        // TODO byteswap the other formats
        LOG_ERROR("Only Z64 format is supported.\n");
        return false;
  }
}

bool Rom::verify_header() {
  LOG("ROM Name: ");
  // Japanese uses Shift JIS
  if (country == 0x4A) {
    rom_name = sj2utf8_alloc(title, TITLE_SIZE);
    LOG("%s\n", rom_name);
  } else {
    // Otherwise it's just ASCII
    // so create an easy to use string
    rom_name = (char*) malloc(sizeof(char)*TITLE_SIZE);
    memcpy(rom_name, title, TITLE_SIZE);
    size_t i = TITLE_SIZE;
    while (i != 0) {
      if (rom_name[i] == ' ') rom_name[i] = '\0';
      else if (rom_name[i] != '\0') break;
      --i;
    }
    LOG("%s\n", rom_name);
  }

  const char* country_str = country_string(country);
  if (country_str == NULL) {
    LOG_ERROR("Bad country byte:%x\n", country);
    return false;
  }
  LOG("Country: %s\n", country_str);

  const char* cart_str = rom_type_string(media_format[3]);
  if (cart_str == NULL) {
    LOG_ERROR("Bad media format byte:%x\n", media_format[3]);
    return false;
  }
  LOG("Cart type: %s\n", cart_str);
  LOG("Version: %x\n", version);

  // check crc
  uint32_t crc[2];
  bootcode = calc_crc(crc, data);
  if (bootcode == 0) {
    LOG_ERROR("Error calculating the cart CRC!\n");
    return false;
  }
  if (crc[0] != crc1) {
    LOG_ERROR("CRC1 doesn't match, bad ROM.\n");
    return false;
  }
  if (crc[1] != crc2) {
    LOG_ERROR("CRC2 doesn't match, bad ROM.\n");
    return false;
  }
  LOG_TRACE("CRC1:%08x\nCRC2:%08x\n", crc1, crc2);

  return true;
}

void Rom::read(byte* target, const uint32_t from, const uint32_t size) const {
  memcpy(target, &data[from], size);
}

bool Rom::parse_header() {
  crc1 = data[0x10] << 24 | data[0x11] << 16 | data[0x12] << 8 | data[0x13];
  crc2 = data[0x14] << 24 | data[0x15] << 16 | data[0x16] << 8 | data[0x17];

  program_counter = data[0x08] << 24 | data[0x09] << 16 | data[0x0A] << 8 | data[0x0B];

  read(title, 0x20, TITLE_SIZE);
  read(media_format, 0x38, FORMAT_SIZE);
  read(cart_id, 0x3C, ID_SIZE);

  country = data[0x3E];
  version = data[0x3F];

  return true;
}

uint32_t Rom::entry_point() const {
  switch (bootcode) {
    case 6101: return program_counter;
    case 6102: return program_counter;
    case 6103: return program_counter - 0x100000;
    case 6105: return program_counter;
    case 6106: return program_counter - 0x200000;
    default: return program_counter;
  }
}

#define DUMP_BINARY 0

void reverse_copy(byte* to, const byte* from, const size_t n) {
  for (size_t i = 0; i < n; ++i) to[n-1-i] = from[i];

#if DUMP_BINARY
  for (int a = 0; a < n; ++a) {
    for (int i = 0; i < 8; i++) LOG("%d", !!((from[a] << i) & 0x80));
  }
  LOG("    ");
#endif
}

bool Rom::find_binary() {
  const uint32_t entry = entry_point();
  LOG("bootcode %i\n", bootcode);
  LOG("program_counter 0x%x\n", program_counter);
  LOG("entry 0x%x\n", entry);

  int jump_count = 0;
  int incond_branch = 0;
  uint32_t asm_end = 0;

  mips_set_file(rom_name);

  // We process each 32 bits as Mips instructions until we hit
  // something malformed, then assume ASM stops there.
  // This is not foolproof, as non ASM binary data could still be
  // valid MIPS. This is as good as we can get when decompiling.
  Instruction mips_inst;
  for (uint32_t at = BOOTCODE_ENDS; at < data_size; at += sizeof(mips_inst)) {
    reverse_copy((byte*)&mips_inst, &data[at], sizeof(mips_inst));
    bool ok;
    switch (mips_inst.r.opcode) {
      case 0:
        ok = handle_r(program_counter, mips_inst.r);
        break;
      case 2:
      case 3:
        ok = handle_j(program_counter, mips_inst.j);
        if (ok && mips_is_j(mips_inst.j)) ++jump_count;
        break;
      default:
        ok = handle_i(program_counter, mips_inst.i);
        if (ok && mips_is_b(mips_inst.i)) ++incond_branch;
        break;
    }
    if (!ok) {
      asm_end = at - sizeof(mips_inst);
      break;
    }
    program_counter += sizeof(mips_inst);
  }
  binary_start = asm_end + 4;
  LOG("# inconditional jumps: %i\n", jump_count);
  LOG("# inconditional branches: %i\n", incond_branch);
  LOG("asm code ends at: 0x%x\n", asm_end);
  LOG("binary starts at: 0x%x\n", binary_start);

  mips_close_file();

  return true;
}

void Rom::dump_text() {
  // Now the real fun begins, we must figure out whatever
  // is encoded in that binary thingie.
  // TODO, check for common format
  // TODO, check what is the asm doing/loading



}
