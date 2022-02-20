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
#include "shift_js.h"

// Biggest possible ROM is 512 megabits
static const long MAX_ROM_SIZE = 0x3D09000;
// Smallest possible ROM is 32 megabits
static const long MIN_ROM_SIZE = 0x3D0900;

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
}

bool Rom::load(const char* path) {
  bool ok = false;

  // open the file
  FILE* file = fopen(path, "rb");
  if (!file) {
      LOG_ERROR("Can't open file:%s\n", path);
      return ok;
  }

  // figure out the file size
  if (fseek(file, 0L, SEEK_END) != 0) goto close_file;

  data_size = ftell(file);
  LOG_TRACE("File size is: %li\n", data_size);

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

  ok = true;
  goto close_file;

unload:
  free(data);

close_file:
  fclose(file);
  return ok;
}

static const uint32_t Z64_MAGIC = 0x80371240;
static const uint32_t N64_MAGIC = 0x40123780;
static const uint32_t V64_MAGIC = 0x37804012;

bool Rom::check_format() {
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
    char* shift_js_str = sj2utf8_alloc(title, TITLE_SIZE);
    LOG("%s\n", shift_js_str);
    free(shift_js_str);
  } else {
    // Otherwise it's just ASCII
    for (unsigned int i = 0; i < TITLE_SIZE; i++) LOG("%c", title[i]);
    LOG("\n");
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
  if (!calc_crc(crc, data)) {
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

void Rom::read(byte* target, const uint32_t from, const uint32_t size) {
  memcpy(target, &data[from], size);
}

bool Rom::parse_header() {
  crc1 = data[0x10] << 24 | data[0x11] << 16 | data[0x12] << 8 | data[0x13];
  crc2 = data[0x14] << 24 | data[0x15] << 16 | data[0x16] << 8 | data[0x17];

  read(title, 0x20, TITLE_SIZE);
  read(media_format, 0x38, FORMAT_SIZE);
  read(cart_id, 0x3C, ID_SIZE);

  country = data[0x3E];
  version = data[0x3F];

  return true;
}


void Rom::dump_text() {
  
}
