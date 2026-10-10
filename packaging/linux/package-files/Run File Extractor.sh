#!/usr/bin/env bash
set -e

package_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
extractor_dir="${package_dir}/File Extractor GUI"
cd "${extractor_dir}"

chmod +x "File Extractor GUI"
export LD_LIBRARY_PATH="${extractor_dir}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
exec ./"File Extractor GUI" "$@"
