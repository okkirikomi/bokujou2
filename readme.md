This is a tool to extract the text of the Nintendo 64 ROM of 牧場物語2 (Harvest Moon 64, Japanese version). The text is extracted in UTF8.

Tested on Linux 4.15.x. Should work on anything with C++11 support. The only non portable API used is be32toh. You'll have to define yours if you're not on Linux or Windows.

Dependencies:
* C++11 compiler
* Your own ROM of 牧場物語2. (Retrode is an easy way to dump your cartridge)

Build:
`make`

Usage:
`./textdump PATH_TO_ROM.z64`

If `PRINT_MIPS` is on, this will also dump the ROM's assembly code.

Limitations:
* Only Z64 ROM format is currently supported.
* Only the first version of 牧場物語2 cart is supported. It *should* work if you change the expected checksum value. I do not have revision 1 or 2 cart so I don't know their checksum.

### Gist

The program starts by parsing the ROM's header.
Then it read each 32 bits as MIPS instruction until a
malformed instruction is hit. The rest of the data is assumed to be
binary content (assets, anything else). This is not 100% foolproof
method since non MIPS binary can still be valid MIPS.
