file(GLOB shaders "${SOURCE}/*.vert" "${SOURCE}/*.frag" "${SOURCE}/*.geom")
file(WRITE "${OUTPUT}"
  "#pragma once\n#include <stdexcept>\n#include <string>\n"
  "inline std::string CaneEmbeddedShader(const std::string& name) {\n")

foreach(shader IN LISTS shaders)
  get_filename_component(name "${shader}" NAME)
  file(READ "${shader}" source)
  string(ASCII 239 187 191 bom)
  string(REGEX REPLACE "^${bom}" "" source "${source}")
  file(APPEND "${OUTPUT}"
    "if (name == \"${name}\") return R\"CANESHADER(${source})CANESHADER\";\n")
endforeach()

file(APPEND "${OUTPUT}"
  "throw std::runtime_error(\"Unknown embedded shader: \" + name);\n}\n")
