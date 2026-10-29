#!/usr/bin/env bash
set -euo pipefail

# Checks that a macOS app bundle is self-contained: every library it loads is
# inside the bundle, everything is arm64, and the bundle is signed. A bundle
# which still points at Homebrew starts fine on the build machine and fails
# on every other Mac, so the build scripts run this before reporting success.
#
# Usage: packaging/macos/check-app.sh [--require <path in the app> [--reason <text>]]... <app>
#
# --require names a file the bundle must contain that no load command shows,
# such as a library that is only opened at run time. It can be repeated.
# --reason, right after a --require, says why that file is needed; it is
# printed if the file is missing.

usage() {
    echo "Usage: $0 [--require <path in the app> [--reason <text>]]... <path to the .app>" >&2
    exit 2
}

required=()
reasons=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --require)
            [[ $# -ge 2 ]] || usage
            required+=("$2")
            reasons+=("")
            shift 2
            ;;
        --reason)
            [[ $# -ge 2 && ${#required[@]} -gt 0 ]] || usage
            reasons[${#reasons[@]} - 1]="$2"
            shift 2
            ;;
        -*) usage ;;
        *) break ;;
    esac
done
if [[ $# -ne 1 ]]; then
    usage
fi

app="${1%/}"
if [[ ! -d "${app}/Contents" ]]; then
    echo "${app} is not an application bundle." >&2
    exit 1
fi
# Symlinks are followed below; compare against the bundle's real path.
app_name="${app}"
app="$(realpath "${app}")"

executable_dir="${app}/Contents/MacOS"
failures=0

fail() {
    echo "FAIL: $*" >&2
    failures=$((failures + 1))
}

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT

# Only macOS itself may be referenced by absolute path. Anything else must be
# resolved relative to the bundle.
is_system_path() {
    [[ "$1" == /usr/lib/* || "$1" == /System/Library/* ]]
}

# Runs otool on the arm64 slice (universal files list every architecture
# separately; only arm64 is ever loaded here) and leaves its output in
# otool_output. A file otool cannot read fails the check instead of being
# skipped silently.
otool_output=""
read_otool() {
    local relative="$1"
    shift
    if ! otool_output="$(otool -arch arm64 "$@" 2>&1)"; then
        fail "otool $1 could not read ${relative}: ${otool_output}"
        return 1
    fi
}

# LC_RPATH entries in otool -l output, one per line. Paths may contain
# spaces: "path @loader_path/../My Frameworks (offset 12)".
rpaths_in() {
    sed -nE '/cmd LC_RPATH/{n;n;s/^[[:space:]]*path (.*) \(offset [0-9]+\)$/\1/p;}' <<<"$1"
}

# Load commands in otool -L output; for a dylib this includes its own
# install name, which callers skip.
loads_in() {
    tail -n +2 <<<"$1" |
        sed -E 's/^[[:space:]]+//; s/ \(compatibility version.*$//; s/ \(offset [0-9]+\)$//'
}

files_list="${work_dir}/files"
links_list="${work_dir}/links"
if ! find "${app}" -type f -print0 > "${files_list}" ||
    ! find "${app}" -type l -print0 > "${links_list}"; then
    echo "Could not list the files in ${app_name}." >&2
    exit 1
fi

# Every symlink must resolve to something inside the bundle.
while IFS= read -r -d '' link; do
    relative="${link#"${app}/"}"
    if ! target="$(realpath "${link}" 2>/dev/null)" || [[ ! -e "${target}" ]]; then
        fail "${relative} is a symlink to $(readlink "${link}"), which does not exist"
    elif [[ "${target}" != "${app}/"* ]]; then
        fail "${relative} is a symlink to ${target}, outside the bundle"
    fi
done < "${links_list}"

# Mach-O files, in parallel arrays (bash 3.2 has no associative arrays):
# path, whether it is an executable, its LC_RPATHs and its @rpath loads.
# @loader_path in an LC_RPATH is resolved here; @executable_path depends on
# the executable that loads the file and is resolved during the walk below.
macho_paths=()
macho_is_executable=()
macho_rpaths=()
macho_rpath_loads=()

while IFS= read -r -d '' binary; do
    relative="${binary#"${app}/"}"
    if ! kind="$(file -b "${binary}")"; then
        fail "file could not read ${relative}"
        continue
    fi
    [[ "${kind}" == *Mach-O* ]] || continue
    binary_dir="$(dirname "${binary}")"

    archs="$(lipo -archs "${binary}" 2>/dev/null || true)"
    if [[ " ${archs} " != *" arm64 "* ]]; then
        fail "${relative} is not arm64 (architectures: ${archs:-unknown})"
        continue
    fi

    read_otool "${relative}" -hv "${binary}" || continue
    header="${otool_output}"

    # A dylib's own install name is not something it loads, but one that
    # points outside the bundle means it was not relocated.
    read_otool "${relative}" -D "${binary}" || continue
    install_name="$(tail -n +2 <<<"${otool_output}")"
    if [[ "${install_name}" == /* ]] && ! is_system_path "${install_name}"; then
        fail "${relative} has install name ${install_name}, outside the bundle"
    fi

    read_otool "${relative}" -l "${binary}" || continue
    rpaths="$(rpaths_in "${otool_output}")"
    resolved_rpaths=""
    while IFS= read -r rpath; do
        [[ -n "${rpath}" ]] || continue
        case "${rpath}" in
            @executable_path|@executable_path/*|@loader_path|@loader_path/*) ;;
            *) fail "${relative} has LC_RPATH ${rpath}, outside the bundle" ;;
        esac
        resolved_rpaths+="${rpath/#@loader_path/${binary_dir}}"$'\n'
    done <<<"${rpaths}"

    read_otool "${relative}" -L "${binary}" || continue
    rpath_loads=""
    while IFS= read -r reference; do
        [[ -n "${reference}" && "${reference}" != "${install_name}" ]] || continue
        case "${reference}" in
            @rpath/*)
                rpath_loads+="${reference}"$'\n'
                ;;
            @loader_path/*)
                [[ -e "${binary_dir}/${reference#@loader_path/}" ]] ||
                    fail "${relative} needs ${reference}, which is not beside it"
                ;;
            @executable_path/*)
                [[ -e "${executable_dir}/${reference#@executable_path/}" ]] ||
                    fail "${relative} needs ${reference}, which is not in the bundle"
                ;;
            *)
                is_system_path "${reference}" ||
                    fail "${relative} loads ${reference} from outside the bundle"
                ;;
        esac
    done < <(loads_in "${otool_output}")

    macho_paths+=("${binary}")
    if [[ "$(awk '/^MH_MAGIC/ { print $5 }' <<<"${header}")" == EXECUTE ]]; then
        macho_is_executable+=(1)
    else
        macho_is_executable+=(0)
    fi
    macho_rpaths+=("${resolved_rpaths}")
    macho_rpath_loads+=("${rpath_loads}")
done < "${files_list}"

macho_index() {
    local i
    for (( i = 0; i < ${#macho_paths[@]}; ++i )); do
        if [[ "${macho_paths[i]}" == "$1" ]]; then
            echo "${i}"
            return
        fi
    done
    echo -1
}

# dyld looks up @rpath/name in the LC_RPATHs of the file that has the load
# command, then in those of the file that loaded it, and so on back to the
# executable. So an executable's LC_RPATHs apply to its own loads and to
# those of every library it loads, while a second executable in the bundle
# (a helper tool) only gets its own. Each executable's load tree is walked
# with the LC_RPATHs collected along the way.
visited=""
walk() {
    local index="$1" inherited="$2" executable="$3"
    local own="${macho_rpaths[index]}" search relative reference rpath found child
    [[ "${visited}" == *"|${index}|"* ]] && return
    visited+="|${index}|"
    own="${own//@executable_path/$(dirname "${executable}")}"
    search="${own}${inherited}"
    relative="${macho_paths[index]#"${app}/"}"
    while IFS= read -r reference; do
        [[ -n "${reference}" ]] || continue
        found=""
        while IFS= read -r rpath; do
            [[ -n "${rpath}" ]] || continue
            if [[ -e "${rpath}/${reference#@rpath/}" ]]; then
                found="$(realpath "${rpath}/${reference#@rpath/}")"
                break
            fi
        done <<<"${search}"
        if [[ -z "${found}" ]]; then
            fail "${relative} needs ${reference}, which no LC_RPATH finds in the bundle" \
                "(when loaded by ${executable#"${app}/"})"
            continue
        fi
        child="$(macho_index "${found}")"
        (( child < 0 )) || walk "${child}" "${search}" "${executable}"
    done <<<"${macho_rpath_loads[index]}"
}

reached=""
for (( index = 0; index < ${#macho_paths[@]}; ++index )); do
    (( macho_is_executable[index] )) || continue
    visited=""
    walk "${index}" "" "${macho_paths[index]}"
    reached+="${visited}"
done

# Libraries no executable links, such as ones opened with dlopen, are looked
# up from the main executable, which is the one that opens them.
main_rpaths=""
main_executable="$(plutil -extract CFBundleExecutable raw "${app}/Contents/Info.plist" 2>/dev/null || true)"
if [[ -n "${main_executable}" ]]; then
    main_index="$(macho_index "${executable_dir}/${main_executable}")"
    if (( main_index >= 0 )); then
        main_rpaths="${macho_rpaths[main_index]//@executable_path/${executable_dir}}"
    fi
fi
visited="${reached}"
for (( index = 0; index < ${#macho_paths[@]}; ++index )); do
    walk "${index}" "${main_rpaths}" "${executable_dir}/${main_executable:-main}"
done

for (( index = 0; index < ${#required[@]}; ++index )); do
    if [[ ! -e "${app}/${required[index]}" ]]; then
        if [[ -n "${reasons[index]}" ]]; then
            fail "${required[index]} is missing (${reasons[index]})"
        else
            fail "${required[index]} is missing"
        fi
    fi
done

if [[ ! -f "${app}/Contents/Info.plist" ]]; then
    fail "Contents/Info.plist is missing"
elif ! plutil -lint "${app}/Contents/Info.plist" >/dev/null; then
    fail "Contents/Info.plist is not a valid property list"
fi

if ! codesign --verify --strict --verbose=2 "${app}" 2>&1; then
    fail "codesign --verify --strict failed"
fi

if (( failures > 0 )); then
    echo "${app_name}: ${failures} problem(s) found." >&2
    exit 1
fi
echo "${app_name}: self-contained, arm64 and signed."
