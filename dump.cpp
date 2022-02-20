#include "log.h"
#include "rom.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    LOG_ERROR("Provide path to N64 rom file.\n");
    return -1;
  }

  Rom rom;
  if (rom.load(argv[1]) == false) {
    LOG_ERROR("Error loading the ROM.\n");
    return -1;
  }

  rom.dump_text();

  rom.unload();
  return 0;
}
