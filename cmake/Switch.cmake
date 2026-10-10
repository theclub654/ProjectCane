set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PORTLIBS "${DEVKITPRO}/portlibs/switch")

file(READ "Sly1-Release/Sly1-Release.vcxproj" vcxproj)
string(REGEX MATCHALL "<ClCompile Include=\"[^\"]+\"" compile_entries "${vcxproj}")
set(cane_sources)
foreach(entry IN LISTS compile_entries)
    string(REGEX REPLACE "<ClCompile Include=\"([^\"]+)\"" "\\1" source "${entry}")
    string(REPLACE "\\" "/" source "${source}")
    list(APPEND cane_sources "${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release/${source}")
endforeach()
list(FILTER cane_sources EXCLUDE REGEX "imgui_impl_opengl3\\.cpp$")
list(FILTER cane_sources EXCLUDE REGEX "spu_reverb_xapo\\.cpp$")
set(platform_sources
    platform/switch/runtime.cpp platform/switch/glfw_libnx.cpp
    platform/switch/audio.cpp platform/switch/audio_device_libnx.cpp platform/switch/audio_prefetch.cpp
    platform/switch/vtables.cpp platform/switch/threading.cpp
    platform/switch/io_worker.cpp platform/switch/deko/backend.cpp
    platform/switch/deko/imgui.cpp platform/switch/installer.cpp
    platform/installer/iso_extractor.cpp)
set(brx_rebuilder "${CMAKE_CURRENT_SOURCE_DIR}/File Extractor GUI/NativeTools/BrxRebuilder")
set(brx_sources)
foreach(source AssetWalker BinaryInputStream BrxWalker GeometryWalker ObjectWalker OptionWalker SpliceWalker TriangleList)
    list(APPEND brx_sources "${brx_rebuilder}/${source}.cpp")
endforeach()
set_source_files_properties(${brx_sources} platform/installer/iso_extractor.cpp PROPERTIES
    COMPILE_DEFINITIONS CBinaryInputStream=CaneBrxInputStream SKIP_PRECOMPILE_HEADERS ON)
set(pal_extractor "${CMAKE_CURRENT_SOURCE_DIR}/File Extractor GUI/NativeTools/PalExtractor")
set(pal_sources)
foreach(source AssetWalker BinaryInputStream BrxWalker GeometryWalker ObjectWalker OptionWalker PalIsoExtractor SpliceWalker)
    list(APPEND pal_sources "${pal_extractor}/${source}.cpp")
endforeach()
set_source_files_properties(${pal_sources} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
add_executable(ProjectCane ${cane_sources} ${platform_sources} ${brx_sources} ${pal_sources})
set_target_properties(ProjectCane PROPERTIES SUFFIX ".elf")
set_property(TARGET ProjectCane PROPERTY CXX_LINKER_LAUNCHER "${CMAKE_COMMAND};-E;env;DEVKITPRO=${DEVKITPRO}")
target_include_directories(ProjectCane BEFORE PRIVATE platform/switch)
target_include_directories(ProjectCane PRIVATE Libraries/include Sly1-Release
    "${CMAKE_CURRENT_BINARY_DIR}/generated" "${PORTLIBS}/include")
target_compile_definitions(ProjectCane PRIVATE CANE_DEKO3D GLFW_INCLUDE_NONE GLFW_STATIC
    IMGUI_DEFINE_MATH_OPERATORS GLM_ENABLE_EXPERIMENTAL USE_STD_FILESYSTEM IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
set(game_headers "${CMAKE_CURRENT_SOURCE_DIR}/platform/switch/compat.h")
foreach(header sw.h save.h freeze.h act.h wr.h shadow.h fader.h splicemap.h gc.h)
    list(APPEND game_headers "${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release/${header}")
endforeach()
target_precompile_headers(ProjectCane PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${game_headers}>")
foreach(source IN LISTS cane_sources)
    if(NOT source MATCHES "/Sly1-Release/[^/]+\\.cpp$")
        set_property(SOURCE "${source}" PROPERTY SKIP_PRECOMPILE_HEADERS ON)
    endif()
endforeach()
set_source_files_properties(${platform_sources} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
target_compile_options(ProjectCane PRIVATE -pthread -fno-strict-aliasing "$<$<COMPILE_LANGUAGE:CXX>:-Wno-invalid-offsetof>")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_program(CANE_UAM NAMES uam HINTS "${DEVKITPRO}/tools/bin" REQUIRED)
file(GLOB shaders CONFIGURE_DEPENDS "Sly1-Release/*.vert" "Sly1-Release/*.frag")
file(GLOB deko_ui_shaders CONFIGURE_DEPENDS "platform/switch/deko/shaders/*")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/deko_shaders.h"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/switch_shaders.py"
        --source "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/generated" --uam "${CANE_UAM}"
    DEPENDS ${shaders} ${deko_ui_shaders} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/switch_shaders.py" VERBATIM)
add_custom_target(deko_shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/deko_shaders.h")
add_dependencies(ProjectCane deko_shaders)
set(installer_background "${CMAKE_CURRENT_SOURCE_DIR}/platform/switch/installer-background.png")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${installer_background}")
file(READ "${installer_background}" background_bytes HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," background_bytes "${background_bytes}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/installer_assets.h"
    CONTENT "#pragma once\nnamespace CaneSetup {\ninline constexpr unsigned char backgroundBytes[] = {${background_bytes}};\n}\n" @ONLY)
unset(background_bytes)

set(link_archives "${DEVKITPRO}/libnx/lib/libdeko3d.a")
foreach(lib avformat avcodec swscale swresample avutil dav1d elf expat zstd z bz2)
    if(NOT EXISTS "${PORTLIBS}/lib/lib${lib}.a")
        message(FATAL_ERROR "Missing Switch portlib: lib${lib}.a")
    endif()
    list(APPEND link_archives "${PORTLIBS}/lib/lib${lib}.a")
endforeach()
target_link_libraries(ProjectCane PRIVATE -Wl,--start-group ${link_archives} nx stdc++ m -Wl,--end-group)
target_link_options(ProjectCane PRIVATE -pthread "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.map")
find_program(CANE_ELF2NRO NAMES elf2nro HINTS "${DEVKITPRO}/tools/bin" REQUIRED)
find_program(CANE_NACPTOOL NAMES nacptool HINTS "${DEVKITPRO}/tools/bin" REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nacp"
    COMMAND "${CANE_NACPTOOL}" --create "ProjectCane" "ProjectCane, naga" "${PROJECT_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nacp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Switch.cmake" VERBATIM)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nro"
    COMMAND "${CANE_ELF2NRO}" "$<TARGET_FILE:ProjectCane>" "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nro"
        "--nacp=${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nacp" "--icon=${CMAKE_CURRENT_SOURCE_DIR}/platform/switch/icon.jpg"
    DEPENDS ProjectCane "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nacp" "${CMAKE_CURRENT_SOURCE_DIR}/platform/switch/icon.jpg" VERBATIM)
add_custom_target(nro ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ProjectCane.nro")
