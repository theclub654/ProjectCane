cmake_minimum_required(VERSION 3.22)
project(ProjectCane VERSION 0.4.0 LANGUAGES C CXX)

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
if(APPLE)
    # macOS caps OpenGL at 4.1 core; code keys the 4.1 path off CANE_GL41, not __APPLE__.
    target_compile_definitions(ProjectCane PRIVATE CANE_GL41)
endif()
target_include_directories(ProjectCane PRIVATE "${CMAKE_BINARY_DIR}/generated")

if(NOT MSVC)
    target_compile_options(ProjectCane PRIVATE -fno-strict-aliasing -Wno-invalid-offsetof)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
file(GLOB shader_inputs CONFIGURE_DEPENDS
    "Sly1-Release/*.vert" "Sly1-Release/*.frag" "Sly1-Release/*.geom")
set(gl41_shader_args "")
if(APPLE)
    # The GL 4.1 copies in Sly1-Release/gl41 replace their shared shaders in
    # the macOS build only.
    file(GLOB gl41_shader_inputs CONFIGURE_DEPENDS
        "Sly1-Release/gl41/*.vert" "Sly1-Release/gl41/*.frag" "Sly1-Release/gl41/*.geom")
    list(APPEND shader_inputs ${gl41_shader_inputs})
    set(gl41_shader_args "-DGL41_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release/gl41")
endif()
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/embedded_shaders.h"
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release
        ${gl41_shader_args}
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

if(APPLE)
    # The tests cover the macOS port only, so other targets build exactly as before.
    enable_testing()
    add_executable(cane_platform_tests tests/platform_test.cpp)
    target_link_libraries(cane_platform_tests PRIVATE cane_platform)
    add_test(NAME cane_platform_tests COMMAND cane_platform_tests)

    # The shader tests cover the GL 4.1 path (CANE_GL41) and its gl41 copies.
    # CANE_SHADER_TOOLS_ONLY limits glshaders.h to its GL-free helpers so these
    # tests don't drag in the game headers.
    add_executable(cane_shader_source_tests tests/shader_source_test.cpp)
    target_link_libraries(cane_shader_source_tests PRIVATE cane_platform)
    target_compile_definitions(cane_shader_source_tests PRIVATE
        CANE_GL41
        CANE_SHADER_TOOLS_ONLY
        "CANE_SHADER_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/Sly1-Release\"")
    add_test(NAME cane_shader_source_tests COMMAND cane_shader_source_tests)

    add_executable(cane_shader_tests tests/shader_gl_test.cpp tests/background_app.cpp
        "${CMAKE_BINARY_DIR}/generated/embedded_shaders.h")
    target_link_libraries(cane_shader_tests PRIVATE cane_ui)
    target_compile_definitions(cane_shader_tests PRIVATE CANE_GL41 CANE_SHADER_TOOLS_ONLY)
    target_include_directories(cane_shader_tests PRIVATE "${CMAKE_BINARY_DIR}/generated")
    add_test(NAME cane_shader_tests COMMAND cane_shader_tests)
    # Exit 77 means no display or no 4.1 context, e.g. headless CI.
    set_tests_properties(cane_shader_tests PROPERTIES SKIP_RETURN_CODE 77)

    # World sweep helpers (image compare, hashes, report rows), no data needed.
    add_executable(cane_world_sweep_selftest
        tests/world_sweep/sweep_behaviour.cpp
        tests/world_sweep/sweep_core.cpp
        tests/world_sweep/sweep_selftest.cpp)
    target_link_libraries(cane_world_sweep_selftest PRIVATE cane_platform)
    add_test(NAME cane_world_sweep_selftest COMMAND cane_world_sweep_selftest)

    # The world sweep drives the real game against extracted game data given
    # at run time (CANE_TEST_DATA_DIR); without it every test below skips.
    # It links ProjectCane's own object files, minus main.cpp, which
    # game_main.cpp compiles again with its entry point renamed, and minus
    # basic.cpp and sidebag.cpp, which sidebag_probe.cpp compiles again with
    # EnsureBasicSidebag and PsidebagNew wrapped; ProjectCane itself builds
    # exactly as before.
    add_executable(cane_world_sweep
        tests/world_sweep/game_main.cpp
        tests/world_sweep/sidebag_probe.cpp
        tests/world_sweep/sweep_behaviour.cpp
        tests/world_sweep/sweep_behaviour_child.cpp
        tests/world_sweep/sweep_child.cpp
        tests/world_sweep/sweep_core.cpp
        tests/world_sweep/sweep_main.cpp
        tests/world_sweep/sweep_units.cpp
        tests/background_app.cpp
        # Anchored on the basename so any generator's object naming
        # (main.cpp.o, main.o, main.obj) matches. If it ever stops matching,
        # the link fails loudly on duplicate symbols.
        "$<FILTER:$<TARGET_OBJECTS:ProjectCane>,EXCLUDE,[/\\](main|basic|sidebag)\\.(cpp\\.)?o(bj)?$>")
    target_compile_definitions(cane_world_sweep PRIVATE
        $<TARGET_PROPERTY:ProjectCane,COMPILE_DEFINITIONS>)
    target_compile_options(cane_world_sweep PRIVATE
        $<TARGET_PROPERTY:ProjectCane,COMPILE_OPTIONS>)
    target_include_directories(cane_world_sweep PRIVATE
        $<TARGET_PROPERTY:ProjectCane,INCLUDE_DIRECTORIES>)
    target_link_libraries(cane_world_sweep PRIVATE
        cane_ui Threads::Threads SDL2::SDL2 PkgConfig::FFMPEG ${cane_audio_target})
    target_precompile_headers(cane_world_sweep REUSE_FROM ProjectCane)
    # Objective-C runtime code: objc.h's BOOL clashes with the game headers.
    set_source_files_properties(tests/background_app.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    add_dependencies(cane_world_sweep ProjectCane)

    # Game-code checks that need no data (sweep_units.cpp).
    add_test(NAME cane_world_sweep_units COMMAND cane_world_sweep units)

    set(world_sweep_out "${CMAKE_BINARY_DIR}/world-sweep")
    foreach(check levels cutscenes audio saves determinism display display-fullscreen van-door hub-tour
            charm-exit behaviour unlock fuse)
        set(check_args)
        if(check STREQUAL "behaviour")
            set(check_args --must-happen "${CMAKE_CURRENT_SOURCE_DIR}/tests/world_sweep/must_happen.json")
        endif()
        if(check STREQUAL "behaviour" OR check STREQUAL "unlock" OR check STREQUAL "fuse")
            # Failures written up for upstream, reported as known (like the
            # sanitizer suppressions).
            list(APPEND check_args --known "${CMAKE_CURRENT_SOURCE_DIR}/tests/world_sweep/known_findings.json")
        endif()
        add_test(NAME cane_world_sweep_${check}
            COMMAND cane_world_sweep ${check} --out "${world_sweep_out}" ${check_args}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
        set_tests_properties(cane_world_sweep_${check} PROPERTIES
            SKIP_RETURN_CODE 77
            # Test processes must never read the user's controllers (no
            # device is VID/PID 0000/0000, so SDL ignores them all).
            ENVIRONMENT "SDL_AUDIO_DRIVER=dummy;SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT=0x0000/0x0000"
            RUN_SERIAL TRUE)
    endforeach()
    # Whole movies run in real time (the longest is about 7 minutes).
    set_tests_properties(cane_world_sweep_levels PROPERTIES TIMEOUT 5400)
    set_tests_properties(cane_world_sweep_cutscenes PROPERTIES TIMEOUT 1800)
    set_tests_properties(cane_world_sweep_audio cane_world_sweep_saves
        cane_world_sweep_determinism cane_world_sweep_display cane_world_sweep_display-fullscreen
        cane_world_sweep_van-door cane_world_sweep_hub-tour cane_world_sweep_charm-exit cane_world_sweep_fuse
        PROPERTIES TIMEOUT 900)
    # 46 levels for 15 s of play each (about 7 minutes), and 46 levels driven,
    # saved and reloaded (about 7 minutes).
    set_tests_properties(cane_world_sweep_behaviour cane_world_sweep_unlock PROPERTIES TIMEOUT 2700)

    # The macos-asan preset compiles everything built from source with
    # AddressSanitizer and UndefinedBehaviorSanitizer and sets CANE_SANITIZE.
    # Reports go to stderr: a test that prints one fails on the regular
    # expression below, and the world sweep's children write theirs to their
    # per-level logs, which the sweep counts as failures. LeakSanitizer is not
    # supported on arm64 macOS. The suppressions list only known findings in
    # third-party or system code and in the game's shared code (written up
    # for upstream); Mac-only code is never suppressed. See tests/README.md.
    option(CANE_SANITIZE "Set sanitizer runtime options on the tests" OFF)
    if(CANE_SANITIZE)
        set(sanitizer_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/sanitizers")
        get_property(cane_tests DIRECTORY PROPERTY TESTS)
        set_property(TEST ${cane_tests} APPEND PROPERTY ENVIRONMENT
            "ASAN_OPTIONS=detect_leaks=0:halt_on_error=0:abort_on_error=1:allow_user_segv_handler=0:check_initialization_order=1:strict_init_order=1:detect_stack_use_after_return=1:print_suppressions=0:suppressions=${sanitizer_dir}/asan.supp"
            "UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0:report_error_type=1:print_suppressions=0:suppressions=${sanitizer_dir}/ubsan.supp")
        # With halt_on_error=0 a test keeps running and can exit 0 after a report.
        set_tests_properties(${cane_tests} PROPERTIES
            FAIL_REGULAR_EXPRESSION "runtime error: |==ERROR: (Address|UndefinedBehavior)Sanitizer: ")
    endif()
endif()

install(TARGETS ProjectCane Sly1BrxRebuilder Sly1PalExtractor RUNTIME DESTINATION bin)
