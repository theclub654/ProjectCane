#!/usr/bin/env bash
set -euo pipefail

# Builds dist/ProjectCane.app for Apple Silicon Macs running macOS 27 or newer.
#
# The normal macos-release build links FFmpeg and sdl2-compat from Homebrew by
# absolute /opt/homebrew paths, so it only starts on a Mac with the same
# Homebrew packages installed. This script copies those libraries (and the
# libraries they need) into the bundle, points every reference at the copies,
# signs the result ad hoc and checks that nothing outside the bundle is used.
# It only uses tools that ship with macOS and the Xcode command line tools.

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="$(cd "${script_dir}/../.." && pwd)"
build_dir="${source_dir}/build/macos-release"
output_dir="${PROJECTCANE_OUTPUT_DIR:-${source_dir}/dist}"
app="${output_dir}/ProjectCane.app"
game_binary="${build_dir}/bin/ProjectCane"

if [[ "$(uname -s)" != "Darwin" || "$(uname -m)" != "arm64" ]]; then
    echo "The macOS app can only be built on an Apple Silicon Mac." >&2
    exit 1
fi

for tool in cmake ninja otool install_name_tool codesign plutil sips iconutil lipo file realpath; do
    if ! command -v "${tool}" >/dev/null 2>&1; then
        echo "${tool} is required to build the macOS app." >&2
        echo "Install the Xcode command line tools (xcode-select --install) and" >&2
        echo "Homebrew's cmake and ninja (brew install cmake ninja)." >&2
        exit 1
    fi
done

if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is required: ProjectCane uses its ffmpeg, sdl2-compat and sdl3 packages." >&2
    exit 1
fi
missing_packages=()
for package in ffmpeg sdl2-compat sdl3; do
    if ! brew list --versions "${package}" >/dev/null 2>&1; then
        missing_packages+=("${package}")
    fi
done
if (( ${#missing_packages[@]} > 0 )); then
    echo "Missing Homebrew packages: ${missing_packages[*]}" >&2
    echo "Install them with: brew install ${missing_packages[*]}" >&2
    exit 1
fi

# sdl2-compat does not link SDL3. It dlopens it at run time and looks for
# @loader_path/libSDL3.dylib first, so SDL3 has to be added by hand.
sdl3_prefix="$(brew --prefix sdl3)"
sdl3_library="$(realpath "${sdl3_prefix}/lib/libSDL3.dylib")"
sdl3_id="$(otool -D "${sdl3_library}")"
sdl3_name="$(basename "$(tail -n 1 <<<"${sdl3_id}")")"

version="$(sed -nE 's/^project\(ProjectCane VERSION ([0-9.]+).*/\1/p' "${source_dir}/CMakeLists.txt")"
if [[ -z "${version}" ]]; then
    echo "Could not read the project version from CMakeLists.txt." >&2
    exit 1
fi

(
    cd "${source_dir}"
    cmake --preset macos-release
    cmake --build --preset macos-release
)

if [[ ! -x "${game_binary}" ]]; then
    echo "The macOS build did not produce ${game_binary}." >&2
    exit 1
fi

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT

# Packaging output is recreated on every run. The bundle is assembled in the
# work folder and only moved to the output folder once it passes the check,
# so a failed run leaves no app behind.
rm -rf "${app}"
work_app="${work_dir}/ProjectCane.app"
contents="${work_app}/Contents"
frameworks="${contents}/Frameworks"
mkdir -p "${contents}/MacOS" "${contents}/Resources" "${frameworks}"
install -m 755 "${game_binary}" "${contents}/MacOS/ProjectCane"

is_system_path() {
    [[ "$1" == /usr/lib/* || "$1" == /System/Library/* ]]
}

# Reports an unreadable Mach-O file. The functions below run inside $(...),
# where bash turns set -e off, so they return a failure explicitly and the
# caller's assignment stops the script. Inside < <(...) the failure would be
# lost, so callers capture the output first.
unreadable() {
    echo "otool could not read the load commands of $1." >&2
    return 1
}

# Load commands of a Mach-O file: its install name (dylibs only) is skipped.
dependencies() {
    local id loads
    id="$(otool -D "$1")" || unreadable "$1" || return
    id="$(tail -n +2 <<<"${id}")"
    loads="$(otool -L "$1")" || unreadable "$1" || return
    tail -n +2 <<<"${loads}" |
        sed -E 's/^[[:space:]]+//; s/ \(compatibility version.*$//' |
        while IFS= read -r reference; do
            [[ "${reference}" == "${id}" ]] || echo "${reference}"
        done
}

# LC_RPATH entries, one per line. Paths may contain spaces:
# "path @loader_path/../My Frameworks (offset 12)".
rpaths() {
    local commands
    commands="$(otool -l "$1")" || unreadable "$1" || return
    sed -nE '/cmd LC_RPATH/{n;n;s/^[[:space:]]*path (.*) \(offset [0-9]+\)$/\1/p;}' <<<"${commands}"
}

executable_rpaths=()
executable_rpath_list="$(rpaths "${game_binary}")"
while IFS= read -r rpath; do
    if [[ -n "${rpath}" ]]; then
        executable_rpaths+=("${rpath}")
    fi
done <<<"${executable_rpath_list}"

# Finds the file a load command refers to, as dyld would for a binary
# installed at original_path. Homebrew mixes /opt/homebrew/opt, Cellar,
# @rpath and @loader_path references to the same libraries.
resolve_reference() {
    local reference="$1" original_path="$2"
    local loader_dir candidate rpath search
    loader_dir="$(dirname "${original_path}")"
    case "${reference}" in
        @loader_path/*)
            candidate="${loader_dir}/${reference#@loader_path/}"
            ;;
        @executable_path/*)
            candidate="$(dirname "${game_binary}")/${reference#@executable_path/}"
            ;;
        @rpath/*)
            candidate=""
            search="$(rpaths "${original_path}")" || exit 1
            while IFS= read -r rpath; do
                [[ -n "${rpath}" ]] || continue
                rpath="${rpath/#@loader_path/${loader_dir}}"
                rpath="${rpath/#@executable_path/$(dirname "${game_binary}")}"
                if [[ -e "${rpath}/${reference#@rpath/}" ]]; then
                    candidate="${rpath}/${reference#@rpath/}"
                    break
                fi
            done < <(printf '%s\n' "${search}" "${executable_rpaths[@]+"${executable_rpaths[@]}"}")
            ;;
        *)
            candidate="${reference}"
            ;;
    esac
    if [[ -z "${candidate}" || ! -e "${candidate}" ]]; then
        echo "Cannot find ${reference}, needed by ${original_path}." >&2
        exit 1
    fi
    realpath "${candidate}"
}

# Bundled libraries are named after the reference (libavutil.61.dylib), not
# the file it resolves to (libavutil.61.1.102.dylib). bash 3.2 has no
# associative arrays, so names and sources are parallel arrays.
bundled_names=()
bundled_sources=()

bundle_library() {
    local name="$1" source="$2" i
    for (( i = 0; i < ${#bundled_names[@]}; ++i )); do
        if [[ "${bundled_names[i]}" == "${name}" ]]; then
            if [[ "${bundled_sources[i]}" != "${source}" ]]; then
                echo "Two different libraries are both named ${name}:" >&2
                echo "  ${bundled_sources[i]}" >&2
                echo "  ${source}" >&2
                exit 1
            fi
            return
        fi
    done
    bundled_names+=("${name}")
    bundled_sources+=("${source}")
    cp "${source}" "${frameworks}/${name}"
    chmod 644 "${frameworks}/${name}"
}

# Applies install_name_tool edits. If the load commands no longer fit in the
# header, the library cannot be relocated without relinking it; stop rather
# than ship a bundle that still loads it from Homebrew.
edit_load_commands() {
    local target="${!#}" output
    if ! output="$(install_name_tool "$@" 2>&1)"; then
        echo "${output}" >&2
        echo "install_name_tool could not rewrite ${target}." >&2
        echo "No library may be skipped, so the bundle was not created." >&2
        exit 1
    fi
    # Every edit invalidates the existing signature; the bundle is re-signed below.
    if [[ -n "${output}" ]]; then
        grep -v 'invalidate the code signature' <<<"${output}" >&2 || true
    fi
}

# Copies the non-system dependencies of one binary into Frameworks and
# points the binary at the copies. prefix is @rpath for the executable and
# @loader_path for libraries, which all sit side by side in Frameworks.
relocate() {
    local target="$1" original_path="$2" prefix="$3"
    local reference resolved name rpath references target_rpaths
    local edits=()
    references="$(dependencies "${target}")"
    while IFS= read -r reference; do
        [[ -n "${reference}" ]] || continue
        is_system_path "${reference}" && continue
        resolved="$(resolve_reference "${reference}" "${original_path}")"
        name="$(basename "${reference}")"
        bundle_library "${name}" "${resolved}"
        if [[ "${reference}" != "${prefix}/${name}" ]]; then
            edits+=(-change "${reference}" "${prefix}/${name}")
        fi
    done <<<"${references}"
    target_rpaths="$(rpaths "${target}")"
    while IFS= read -r rpath; do
        case "${rpath}" in
            ""|@loader_path/*|@executable_path/*) ;;
            *) edits+=(-delete_rpath "${rpath}") ;;
        esac
    done <<<"${target_rpaths}"
    if (( ${#edits[@]} > 0 )); then
        edit_load_commands "${edits[@]}" "${target}"
    fi
}

relocate "${contents}/MacOS/ProjectCane" "${game_binary}" "@rpath"
edit_load_commands -add_rpath "@executable_path/../Frameworks" "${contents}/MacOS/ProjectCane"

bundle_library "${sdl3_name}" "${sdl3_library}"

# The list grows while it is walked, which collects the whole closure.
for (( index = 0; index < ${#bundled_names[@]}; ++index )); do
    library="${frameworks}/${bundled_names[index]}"
    relocate "${library}" "${bundled_sources[index]}" "@loader_path"
    edit_load_commands -id "@rpath/${bundled_names[index]}" "${library}"
done

if [[ "${sdl3_name}" != "libSDL3.dylib" ]]; then
    ln -s "${sdl3_name}" "${frameworks}/libSDL3.dylib"
fi

sed "s/@PROJECTCANE_VERSION@/${version}/g" \
    "${script_dir}/Info.plist.in" > "${contents}/Info.plist"
printf 'APPL????' > "${contents}/PkgInfo"

# The game's own icon is a Windows .ico (largest image 256x256); sips can read
# it. Iconset entries larger than the source image are left out rather than
# scaled up; macOS scales the largest one it has. If sips cannot read it, use
# the wide branding artwork centered on a square canvas, as the Linux package
# does.
icon_source="${work_dir}/icon.png"
if sips -s format png "${source_dir}/Sly1-Release/Assets/Sly1.ico" \
        --out "${icon_source}" >/dev/null 2>&1; then
    echo "App icon: Sly1-Release/Assets/Sly1.ico"
else
    sips -Z 496 "${source_dir}/README-assets/projectcane-logo.png" \
        --out "${work_dir}/logo.png" >/dev/null
    sips -p 512 512 "${work_dir}/logo.png" --out "${icon_source}" >/dev/null
    echo "App icon: README-assets/projectcane-logo.png (Sly1.ico could not be read)"
fi
icon_info="$(sips -g pixelWidth "${icon_source}")"
icon_size="$(awk '/pixelWidth/ { print $2 }' <<<"${icon_info}")"
iconset="${work_dir}/ProjectCane.iconset"
mkdir -p "${iconset}"
for size in 16 32 128 256 512; do
    if (( size <= icon_size )); then
        sips -z "${size}" "${size}" "${icon_source}" \
            --out "${iconset}/icon_${size}x${size}.png" >/dev/null
    fi
    if (( size * 2 <= icon_size )); then
        sips -z $((size * 2)) $((size * 2)) "${icon_source}" \
            --out "${iconset}/icon_${size}x${size}@2x.png" >/dev/null
    fi
done
iconutil -c icns "${iconset}" -o "${contents}/Resources/ProjectCane.icns"

# Sign from the inside out: libraries, then the executable, then the bundle.
# Ad-hoc signatures are enough to run on Apple Silicon; the app is not
# notarised, so Gatekeeper asks the user to confirm the first launch.
for name in "${bundled_names[@]}"; do
    codesign --force --sign - "${frameworks}/${name}"
done
codesign --force --sign - "${contents}/MacOS/ProjectCane"
codesign --force --sign - "${work_app}"

"${script_dir}/check-app.sh" \
    --require Contents/Frameworks/libSDL3.dylib \
    --reason "sdl2-compat loads SDL3 by this name at run time" \
    "${work_app}"

mkdir -p "${output_dir}"
mv "${work_app}" "${app}"

echo
echo "macOS app created: ${app}"
echo "Version ${version}; Apple Silicon, macOS 27 or newer; ${#bundled_names[@]} bundled libraries."
