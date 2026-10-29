#!/usr/bin/env bash
set -euo pipefail

# Builds "dist/ProjectCane File Extractor.app" for Apple Silicon Macs.
#
# The File Extractor is a .NET app. It is published self-contained, so the
# bundle carries its own .NET runtime, and as a single file, so that
# Contents/MacOS only holds Mach-O files: codesign treats everything in that
# folder as code and cannot seal the loose .dll and .json files of a normal
# publish. The two native tools it runs, Sly1BrxRebuilder and
# Sly1PalExtractor, come from the macos-release CMake build and sit beside
# the executable, where the extractor looks for them.

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="$(cd "${script_dir}/../.." && pwd)"
build_dir="${source_dir}/build/macos-release"
project="${source_dir}/File Extractor GUI/File Extractor GUI.csproj"
output_dir="${PROJECTCANE_OUTPUT_DIR:-${source_dir}/dist}"
app="${output_dir}/ProjectCane File Extractor.app"
executable="File Extractor GUI"
tools=(Sly1BrxRebuilder Sly1PalExtractor)

if [[ "$(uname -s)" != "Darwin" || "$(uname -m)" != "arm64" ]]; then
    echo "The macOS app can only be built on an Apple Silicon Mac." >&2
    exit 1
fi

if ! command -v dotnet >/dev/null 2>&1; then
    echo "dotnet is required to build the File Extractor." >&2
    echo "Install the .NET SDK (9 or newer) from https://dotnet.microsoft.com/download" >&2
    exit 1
fi
for tool in cmake ninja otool install_name_tool codesign plutil sips iconutil lipo file; do
    if ! command -v "${tool}" >/dev/null 2>&1; then
        echo "${tool} is required to build the File Extractor app." >&2
        echo "Install the Xcode command line tools (xcode-select --install) and" >&2
        echo "Homebrew's cmake and ninja (brew install cmake ninja)." >&2
        exit 1
    fi
done

version="$(sed -nE 's#.*<Version>([^<]+)</Version>.*#\1#p' "${project}" | head -n 1)"
if [[ -z "${version}" ]]; then
    echo "Could not read the File Extractor version from ${project}." >&2
    exit 1
fi

(
    cd "${source_dir}"
    cmake --preset macos-release
    cmake --build --preset macos-release --target "${tools[@]}"
)

for tool in "${tools[@]}"; do
    if [[ ! -x "${build_dir}/bin/${tool}" ]]; then
        echo "The macOS build did not produce ${build_dir}/bin/${tool}." >&2
        exit 1
    fi
done

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT

publish_dir="${work_dir}/publish"
dotnet publish "${project}" -c Release -r osx-arm64 --self-contained true \
    -p:PublishSingleFile=true -p:DebugType=None -o "${publish_dir}"
if [[ ! -x "${publish_dir}/${executable}" ]]; then
    echo "dotnet publish did not produce ${publish_dir}/${executable}." >&2
    exit 1
fi

# Packaging output is recreated on every run. The bundle is assembled in the
# work folder and only moved to the output folder once it passes the check,
# so a failed run leaves no app behind.
rm -rf "${app}"
work_app="${work_dir}/ProjectCane File Extractor.app"
contents="${work_app}/Contents"
mkdir -p "${contents}/MacOS" "${contents}/Resources"
cp -R "${publish_dir}/." "${contents}/MacOS/"
for tool in "${tools[@]}"; do
    install -m 755 "${build_dir}/bin/${tool}" "${contents}/MacOS/${tool}"
done

sed "s/@EXTRACTOR_VERSION@/${version}/g" \
    "${script_dir}/Extractor-Info.plist.in" > "${contents}/Info.plist"
printf 'APPL????' > "${contents}/PkgInfo"

# The icon artwork is 1024x1024, so every iconset entry is scaled down from it.
icon_source="${source_dir}/File Extractor GUI/Assets/file-extractor-icon.png"
iconset="${work_dir}/FileExtractor.iconset"
mkdir -p "${iconset}"
for size in 16 32 128 256 512; do
    sips -z "${size}" "${size}" "${icon_source}" \
        --out "${iconset}/icon_${size}x${size}.png" >/dev/null
    sips -z $((size * 2)) $((size * 2)) "${icon_source}" \
        --out "${iconset}/icon_${size}x${size}@2x.png" >/dev/null
done
iconutil -c icns "${iconset}" -o "${contents}/Resources/FileExtractor.icns"

# Applies install_name_tool edits and stops if they fail. Every edit
# invalidates the existing signature; the bundle is re-signed below.
edit_load_commands() {
    local target="${!#}" output
    if ! output="$(install_name_tool "$@" 2>&1)"; then
        echo "${output}" >&2
        echo "install_name_tool could not rewrite ${target}." >&2
        echo "The bundle was not created." >&2
        exit 1
    fi
    if [[ -n "${output}" ]]; then
        grep -v 'invalidate the code signature' <<<"${output}" >&2 || true
    fi
}

# Avalonia ships libAvaloniaNative.dylib with the install name it had on its
# build machine (/usr/local/lib/...). Nothing links against it, since .NET
# opens it by path, but give it the same @rpath form as the other libraries
# so that no name in the bundle points outside it. The file list goes
# through a file so that a find failure stops the script.
file_list="${work_dir}/files"
find "${contents}/MacOS" -type f ! -name "${executable}" -print0 > "${file_list}"
macho_files=()
while IFS= read -r -d '' file; do
    kind="$(file -b "${file}")"
    if [[ "${kind}" == *Mach-O* ]]; then
        macho_files+=("${file}")
    fi
done < "${file_list}"
for file in "${macho_files[@]}"; do
    install_name="$(otool -arch arm64 -D "${file}")"
    install_name="$(tail -n +2 <<<"${install_name}")"
    if [[ "${install_name}" == /* && "${install_name}" != /usr/lib/* &&
          "${install_name}" != /System/Library/* ]]; then
        edit_load_commands -id "@rpath/$(basename "${file}")" "${file}"
    fi
done

# Sign from the inside out: libraries and tools, then the executable, then
# the bundle. Ad-hoc signatures are enough to run on Apple Silicon; the app
# is not notarised, so Gatekeeper asks the user to confirm the first launch.
for file in "${macho_files[@]}"; do
    codesign --force --sign - "${file}"
done
codesign --force --sign - "${contents}/MacOS/${executable}"
codesign --force --sign - "${work_app}"

"${script_dir}/check-app.sh" \
    --require Contents/MacOS/Sly1BrxRebuilder \
    --require Contents/MacOS/Sly1PalExtractor \
    "${work_app}"

mkdir -p "${output_dir}"
mv "${work_app}" "${app}"

echo
echo "macOS app created: ${app}"
echo "Version ${version}; Apple Silicon."
