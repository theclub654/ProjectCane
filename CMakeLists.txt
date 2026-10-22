cmake_minimum_required(VERSION 3.22)
project(ProjectCane VERSION 0.4.0 LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER MATCHES "aarch64-none-elf")
    include(cmake/Switch.cmake)
    return()
endif()

# CMake 4 no longer enables compatibility policies older than 3.5 by default.
# Some pinned third-party projects (currently FAudio 25.02) still declare an
# older minimum, so provide the supported policy floor to those subprojects.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING
    "Minimum policy compatibility for bundled third-party projects")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(brx_rebuilder_sources
    "File Extractor GUI/NativeTools/BrxRebuilder/AssetWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/BinaryInputStream.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/BrxWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/GeometryWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/main.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/ObjectWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/OptionWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/SpliceWalker.cpp"
    "File Extractor GUI/NativeTools/BrxRebuilder/TriangleList.cpp")
add_executable(Sly1BrxRebuilder ${brx_rebuilder_sources})
target_compile_definitions(Sly1BrxRebuilder PRIVATE _CRT_SECURE_NO_WARNINGS)

set(pal_extractor_sources
    "File Extractor GUI/NativeTools/PalExtractor/AssetWalker.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/BinaryInputStream.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/BrxWalker.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/GeometryWalker.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/main.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/ObjectWalker.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/OptionWalker.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/PalIsoExtractor.cpp"
    "File Extractor GUI/NativeTools/PalExtractor/SpliceWalker.cpp")
add_executable(Sly1PalExtractor ${pal_extractor_sources})
target_compile_definitions(Sly1PalExtractor PRIVATE _CRT_SECURE_NO_WARNINGS)

set(CANE_FFMPEG_ROOT "" CACHE PATH "FFmpeg development prefix")
if(CANE_FFMPEG_ROOT AND NOT WIN32)
    list(PREPEND CMAKE_PREFIX_PATH "${CANE_FFMPEG_ROOT}")
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
find_package(Threads REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL https://codeload.github.com/glfw/glfw/tar.gz/refs/tags/3.4
    URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01)
FetchContent_MakeAvailable(glfw)

find_package(SDL2 2.24 CONFIG QUIET)
if(NOT SDL2_FOUND)
    if(WIN32)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
    else()
        set(SDL_SHARED ON CACHE BOOL "" FORCE)
        set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    endif()
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(sdl2
        URL https://codeload.github.com/libsdl-org/SDL/tar.gz/refs/tags/release-2.30.11
        URL_HASH SHA256=cc6136dd964854e8846c679703322f3e2a341d27a06a53f8b3f642c26f1b0cfd)
    FetchContent_MakeAvailable(sdl2)
endif()

add_library(cane_platform INTERFACE)
target_include_directories(cane_platform INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform")
if(WIN32)
    target_link_libraries(cane_platform INTERFACE shell32)
endif()

set(imgui_root "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include/imgui")
add_library(cane_ui STATIC
    "${imgui_root}/imgui.cpp"
    "${imgui_root}/imgui_draw.cpp"
    "${imgui_root}/imgui_tables.cpp"
    "${imgui_root}/imgui_widgets.cpp"
    "${imgui_root}/imgui_impl_glfw.cpp"
    "${imgui_root}/imgui_impl_opengl3.cpp"
    "${imgui_root}/ImGuiFileDialog.cpp"
    Sly1-Release/glad.c)
target_link_libraries(cane_ui PUBLIC cane_platform glfw ${CMAKE_DL_LIBS})
target_compile_definitions(cane_ui PUBLIC IMGUI_DEFINE_MATH_OPERATORS GLFW_INCLUDE_NONE)

# Keep Visual Studio's source list authoritative while the two build systems coexist.
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release/Sly1-Release.vcxproj"
    source_lines REGEX "<ClCompile Include=")
set(game_sources "")
foreach(line IN LISTS source_lines)
    string(REGEX REPLACE ".*Include=\"([^\"]+)\".*" "\\1" source "${line}")
    string(REPLACE "\\" "/" source "${source}")
    if(NOT source MATCHES "^\\.\\." AND NOT source STREQUAL "glad.c")
        list(APPEND game_sources "Sly1-Release/${source}")
    endif()
endforeach()

if(NOT WIN32)
    list(REMOVE_ITEM game_sources "Sly1-Release/spu_reverb_xapo.cpp")
endif()

add_executable(ProjectCane ${game_sources} "${imgui_root}/imgui_demo.cpp")
target_link_libraries(ProjectCane PRIVATE cane_ui Threads::Threads SDL2::SDL2)
target_compile_definitions(ProjectCane PRIVATE
    CANE_PORTABLE_SHADERS
    _CRT_SECURE_NO_WARNINGS
    _ENABLE_EXTENDED_ALIGNED_STORAGE)
target_include_directories(ProjectCane PRIVATE "${CMAKE_BINARY_DIR}/generated")

if(NOT MSVC)
    target_compile_options(ProjectCane PRIVATE -fno-strict-aliasing -Wno-invalid-offsetof)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
file(GLOB shader_inputs CONFIGURE_DEPENDS
    "Sly1-Release/*.vert" "Sly1-Release/*.frag" "Sly1-Release/*.geom")
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/embedded_shaders.h"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release
        -DOUTPUT=${CMAKE_BINARY_DIR}/generated/embedded_shaders.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed-shaders.cmake
    DEPENDS ${shader_inputs} cmake/embed-shaders.cmake
    VERBATIM)
target_sources(ProjectCane PRIVATE "${CMAKE_BINARY_DIR}/generated/embedded_shaders.h")

foreach(header
    platform/platform.h
    Sly1-Release/sw.h Sly1-Release/save.h Sly1-Release/freeze.h
    Sly1-Release/act.h Sly1-Release/wr.h Sly1-Release/shadow.h
    Sly1-Release/fader.h Sly1-Release/splicemap.h Sly1-Release/gc.h)
    target_precompile_headers(ProjectCane PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/${header}>")
endforeach()
set_source_files_properties(Sly1-Release/alo.cpp PROPERTIES
    COMPILE_DEFINITIONS ALO_IMPLEMENTATION SKIP_PRECOMPILE_HEADERS ON)

if(WIN32)
    if(NOT CANE_FFMPEG_ROOT)
        set(CANE_FFMPEG_ROOT
            "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/ffmpeg/ffmpeg-master-latest-win64-lgpl-shared")
    endif()
    target_include_directories(ProjectCane PRIVATE "${CANE_FFMPEG_ROOT}/include")
    target_link_directories(ProjectCane PRIVATE "${CANE_FFMPEG_ROOT}/lib")
    target_link_libraries(ProjectCane PRIVATE
        opengl32 winmm xaudio2 avformat avcodec avutil swscale)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavformat libavcodec libavutil libswscale)

    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(faudio
        URL https://codeload.github.com/FNA-XNA/FAudio/tar.gz/refs/tags/25.02
        URL_HASH SHA256=eab667d231e5036b0e5c3c6f89fb4ea792284493d46161bc056a88a6dadc2683)
    FetchContent_MakeAvailable(faudio)
    if(TARGET FAudio-shared)
        set(cane_audio_target FAudio-shared)
    else()
        set(cane_audio_target FAudio-static)
    endif()

    target_link_libraries(ProjectCane PRIVATE PkgConfig::FFMPEG ${cane_audio_target})
endif()

if(MSVC)
    set_property(TARGET ProjectCane cane_ui glfw PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS ProjectCane Sly1BrxRebuilder Sly1PalExtractor RUNTIME DESTINATION bin)
