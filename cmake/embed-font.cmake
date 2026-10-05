file(READ "${SOURCE}" bytes HEX)
string(REGEX REPLACE "(..)" "0x\\1," bytes "${bytes}")
file(WRITE "${OUTPUT}" "#pragma once\ninline const unsigned char cane_font[] = {${bytes}};\n")
