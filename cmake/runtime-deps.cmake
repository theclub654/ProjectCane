# Inspect build artifacts together before copying any library, avoiding
# resolution conflicts between newly staged libraries and their originals.
file(GET_RUNTIME_DEPENDENCIES
 EXECUTABLES ${cane_runtime_executables}
 RESOLVED_DEPENDENCIES_VAR deps
 PRE_EXCLUDE_REGEXES "^linux-vdso" "^ld-linux" "^libstdc" "^libgcc_s" "^libc[.]so" "^libm[.]so" "^libdl[.]so" "^libpthread[.]so" "^librt[.]so" "^libGL" "^libEGL" "^libOpenGL")
foreach(dep IN LISTS deps)
 file(INSTALL "${dep}" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" FOLLOW_SYMLINK_CHAIN)
endforeach()
