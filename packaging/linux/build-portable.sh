#!/usr/bin/env bash
set -euo pipefail

# This package targets SteamOS and mainstream Linux distributions.  Building
# it on a newer rolling distribution silently raises the required glibc
# version even though linuxdeploy succeeds, producing an AppImage which cannot
# start on Steam Deck.  Fail before compiling instead of shipping such an
# image accidentally.
host_glibc_version="$(ldd --version 2>&1 | head -n 1 | grep -oE '[0-9]+\.[0-9]+' | tail -n 1)"
if [[ -z "${host_glibc_version}" ]]; then
    echo "Could not determine the build host's glibc version." >&2
    exit 1
fi
host_glibc_major="${host_glibc_version%%.*}"
host_glibc_minor="${host_glibc_version#*.}"
if (( host_glibc_major > 2 || (host_glibc_major == 2 && host_glibc_minor > 35) )); then
    echo "This host uses glibc ${host_glibc_version}; the portable build requires glibc 2.35 or older." >&2
    echo "Run this script inside Ubuntu 22.04 (wsl -d Ubuntu-22.04), not the default WSL distribution." >&2
    exit 1
fi

source_dir="${PROJECTCANE_SOURCE_DIR:-/src}"
output_dir="${PROJECTCANE_OUTPUT_DIR:-/out}"
work_dir="${PROJECTCANE_WORK_DIR:-/work/projectcane}"
build_dir="${work_dir}/build"
app_dir="${work_dir}/ProjectCane.AppDir"
linuxdeploy_dir="${work_dir}/linuxdeploy"

if [[ ! -f "${source_dir}/CMakeLists.txt" ]]; then
    echo "ProjectCane source was not mounted at ${source_dir}." >&2
    exit 1
fi

if [[ "${PROJECTCANE_CLEAN_BUILD:-0}" == "1" ]]; then
    rm -rf "${work_dir}"
fi

# Preserve the CMake/Ninja directory by default so a failed portability build
# can resume. Packaging output is recreated on every run.
rm -rf "${app_dir}" "${linuxdeploy_dir}"
mkdir -p \
    "${build_dir}" \
    "${app_dir}/usr/bin" \
    "${app_dir}/usr/share/applications" \
    "${app_dir}/usr/share/icons/hicolor/512x512/apps" \
    "${output_dir}" \
    "${linuxdeploy_dir}"

cmake -S "${source_dir}" -B "${build_dir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${build_dir}" --parallel "$(nproc)"

game_binary="${build_dir}/bin/ProjectCane"
if [[ ! -x "${game_binary}" ]]; then
    echo "The Linux build did not produce ${game_binary}." >&2
    exit 1
fi

# Ubuntu 22.04 provides glibc 2.35. Building here keeps the executable usable
# on SteamOS and newer mainstream distributions without shipping glibc.
if readelf --version-info "${game_binary}" | grep -Eq 'GLIBC_2\.(3[6-9]|[4-9][0-9])'; then
    echo "The executable unexpectedly requires a glibc newer than 2.35." >&2
    readelf --version-info "${game_binary}" | grep -o 'GLIBC_[0-9.]*' | sort -Vu >&2
    exit 1
fi

install -Dm755 "${game_binary}" "${app_dir}/usr/bin/ProjectCane"
install -Dm644 "${source_dir}/packaging/linux/projectcane.desktop" \
    "${app_dir}/usr/share/applications/projectcane.desktop"

if ! command -v convert >/dev/null 2>&1; then
    echo "ImageMagick is required to create the square AppImage icon." >&2
    echo "Install it with: sudo apt install imagemagick" >&2
    exit 1
fi

# The branding artwork is intentionally wide. AppImage icons must be square
# and use one of the freedesktop standard dimensions, so preserve the artwork's
# aspect ratio and center it on a transparent 512x512 canvas.
convert "${source_dir}/README-assets/projectcane-logo.png" \
    -resize '496x496>' \
    -background none \
    -gravity center \
    -extent 512x512 \
    "${app_dir}/usr/share/icons/hicolor/512x512/apps/projectcane.png"

linuxdeploy_appimage="${linuxdeploy_dir}/linuxdeploy-x86_64.AppImage"
curl --fail --location --retry 3 \
    --output "${linuxdeploy_appimage}" \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod +x "${linuxdeploy_appimage}"

# AppImages can be extracted and run without FUSE, which also works inside the
# build container and on restricted CI hosts.
(
    cd "${linuxdeploy_dir}"
    "${linuxdeploy_appimage}" --appimage-extract >/dev/null
)

portable_image="${work_dir}/ProjectCane-x86_64.AppImage"
export OUTPUT="${portable_image}"
"${linuxdeploy_dir}/squashfs-root/AppRun" \
    --appdir "${app_dir}" \
    --executable "${app_dir}/usr/bin/ProjectCane" \
    --desktop-file "${app_dir}/usr/share/applications/projectcane.desktop" \
    --icon-file "${app_dir}/usr/share/icons/hicolor/512x512/apps/projectcane.png" \
    --output appimage

chmod +x "${portable_image}"
cp -f "${portable_image}" "${output_dir}/ProjectCane-x86_64.AppImage"

echo
echo "Portable Linux build created: ${output_dir}/ProjectCane-x86_64.AppImage"
echo "Build baseline: Ubuntu 22.04 / glibc 2.35"
