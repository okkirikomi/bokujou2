#ifndef ROM_H
#define ROM_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "defs.h"

static const size_t TITLE_SIZE = 20;
static const size_t FORMAT_SIZE = 4;
static const size_t ID_SIZE = 4;

struct Rom {
  bool load(const char* path);
  void unload();
  void dump_text();

  unsigned char operator[] (size_t i) const { return data[i]; }
  unsigned char& operator[] (size_t i) { return data[i]; }

  char* rom_name;
  byte* data;
  long data_size;
  uint32_t binary_start;

  int32_t bootcode;
  uint32_t crc1;
  uint32_t crc2;
  uint32_t program_counter;
  byte title[TITLE_SIZE];
  byte media_format[FORMAT_SIZE];
  byte cart_id[ID_SIZE];
  byte country;
  byte version;

private:
  bool parse_header();
  bool check_format() const;
  bool verify_header();
  bool find_binary();
  void read(byte* target, const uint32_t from, const uint32_t size) const;

  uint32_t entry_point() const;

};

#endif // ROM_H
