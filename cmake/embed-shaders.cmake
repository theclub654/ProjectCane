file(GLOB shaders "${SOURCE}/*.vert" "${SOURCE}/*.frag" "${SOURCE}/*.geom")
file(WRITE "${OUTPUT}"
  "#pragma once\n#include <stdexcept>\n#include <string>\n"
  "inline std::string CaneEmbeddedShader(const std::string& name) {\n")

foreach(shader IN LISTS shaders)
  get_filename_component(name "${shader}" NAME)
  # macOS only: GL41_SOURCE (Sly1-Release/gl41) holds GL 4.1 copies that are
  # embedded under the shared shader's name in its place.
  if(DEFINED GL41_SOURCE AND EXISTS "${GL41_SOURCE}/${name}")
    set(shader "${GL41_SOURCE}/${name}")
  endif()
  file(READ "${shader}" source)
  string(ASCII 239 187 191 bom)
  string(REGEX REPLACE "^${bom}" "" source "${source}")
  file(APPEND "${OUTPUT}"
    "if (name == \"${name}\") return R\"CANESHADER(${source})CANESHADER\";\n")
endforeach()

# macOS only: GL41_SOURCE files with no shared counterpart (Mac additions such
# as celborder.geom) are embedded under their own name.
if(DEFINED GL41_SOURCE)
  file(GLOB gl41_only "${GL41_SOURCE}/*.vert" "${GL41_SOURCE}/*.frag" "${GL41_SOURCE}/*.geom")
  foreach(shader IN LISTS gl41_only)
    get_filename_component(name "${shader}" NAME)
    if(NOT EXISTS "${SOURCE}/${name}")
      file(READ "${shader}" source)
      string(ASCII 239 187 191 bom)
      string(REGEX REPLACE "^${bom}" "" source "${source}")
      file(APPEND "${OUTPUT}"
        "if (name == \"${name}\") return R\"CANESHADER(${source})CANESHADER\";\n")
    endif()
  endforeach()
endif()

file(APPEND "${OUTPUT}"
  "throw std::runtime_error(\"Unknown embedded shader: \" + name);\n}\n")
