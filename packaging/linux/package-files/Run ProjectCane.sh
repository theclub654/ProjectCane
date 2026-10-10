#!/usr/bin/env bash
set -e

package_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "${package_dir}"

user_data_root="${XDG_DATA_HOME:-${HOME}/.local/share}"
user_dir="${user_data_root}/ProjectCane"
mkdir -p "${user_dir}"

chmod +x ProjectCane-x86_64.AppImage
exec ./ProjectCane-x86_64.AppImage \
    --data-dir "${package_dir}" \
    --user-dir "${user_dir}" \
    "$@"
