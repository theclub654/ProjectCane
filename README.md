# ProjectCane

<img width="1668" height="1134" alt="project_cane" src="https://github.com/user-attachments/assets/98ac0463-6c18-4cb0-92f8-2f460b03def3" />

ProjectCane is a native PC port of the USA release of **Sly Cooper and the Thievius Raccoonus**. It supports x86-64 Windows and Linux, including SteamOS, with an offline launcher.

The game requires OpenGL 4.3 and an original USA disc image (`SCUS_971.98`). Other regions, revised/rebuilt images, and compressed disc formats are not supported by this extractor. ProjectCane packages contain no original game data.

## Play with the launcher

1. Download or build a package for your platform.
2. On Linux, make the AppImage executable and open it. If FUSE is unavailable, run it with `--appimage-extract-and-run`, or extract the portable archive and open **Launch ProjectCane.sh**. On Windows, choose the **Setup EXE** to install/update the launcher, or extract the **Portable ZIP** and open **Launch ProjectCane.cmd**.
3. Select or drag in your Sly 1 USA ISO and choose **Install game**.
4. Wait for extraction and checksum verification, then choose **Play**.

ProjectCane does not include the original game's copyrighted data. You must extract the required files from your own NTSC copy of the game.

The launcher supports keyboard and controller navigation. On Steam Deck, install in Desktop Mode, add the launcher as a non-Steam game, and select a gamepad controller layout. Run it natively; do not force Proton for the Linux package.

Game data, saves, settings and logs use the following locations:

| Platform | Game data | Saves, configuration and logs |
| --- | --- | --- |
| Linux | `$XDG_DATA_HOME/projectcane/installations/sly1` (default `~/.local/share/projectcane/installations/sly1`) | `$XDG_CONFIG_HOME/projectcane` (default `~/.config/projectcane`); saves in `Saves`, logs in `logs` |
| Windows setup | `%LOCALAPPDATA%/ProjectCane/installations/sly1` | `%LOCALAPPDATA%/ProjectCane/user/Saves`, `config` and `logs` |
| Windows portable | `data/installations/sly1` inside the extracted package | `data/user/Saves`, `data/config` and `data/logs` inside the package |

Existing Linux saves from `~/.local/share/projectcane/user/Saves` and logs from `~/.local/state/projectcane/logs` are copied into the new configuration location on first launch, respecting XDG overrides. Originals are retained, and a populated destination is never overwritten. Adjacent legacy `Saves` are also imported only into an empty launcher profile. Source builds and installed custom runtimes remain in the data directory. Standalone runtime launches retain their adjacent save/data defaults. A package requiring a new asset format requests reinstallation. This first release has no online updater.

## Windows packages

- **ProjectCane-Windows-x86_64-Portable.zip**: extract to a writable folder and run **Launch ProjectCane.cmd**. Assets, saves, settings, logs and optional source-build output stay under the package's `data/` folder. Keep `portable.txt` beside `bin/`. The folder can be moved, including when a custom runtime is selected. To update, close the launcher and game, then copy the new application files over the existing folder while preserving `data/`.
- **ProjectCane-Windows-x86_64-Setup.exe**: installs for the current Windows account without administrator privileges, normally into `%LOCALAPPDATA%/Programs/ProjectCane`. Setup adds a Start Menu shortcut and offers an optional desktop shortcut. Run a newer setup to update the same installation. Close the launcher and game before updating. Saves and extracted assets remain in AppData through updates and uninstalling the launcher.

Both packages include the launcher, runtime, extractor and required DLLs for offline use. The setup version reuses the AppData profile from earlier Windows ZIP releases. The portable version has its own independent profile.

## Build

CMake 3.22+, a C++20 compiler, and Ninja (or Visual Studio 2022) are required. GLFW 3.4, FAudio 25.02, and the SDL 2.30.11 fallback archives are pinned with SHA-256 hashes. The launcher font, ImGui, GLAD, and supporting headers are included. First-time source builds may need internet access.

For Ubuntu 22.04 or newer (install `file` and `desktop-file-utils` as well when producing AppImages):

```sh
sudo apt install build-essential cmake ninja-build libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev wayland-protocols libsdl2-dev libavformat-dev libavcodec-dev libavutil-dev libswscale-dev
cmake --preset linux-release
cmake --build --preset linux-release --parallel 4
ctest --test-dir build/linux-release --output-on-failure
```

If your Wayland development packages are too old, configure with `-DGLFW_BUILD_WAYLAND=OFF` and use X11/XWayland. FAudio uses SDL2 for Linux audio; Windows retains XAudio2.

For Windows, install Visual Studio 2022's **Desktop development with C++** workload and CMake, then:

```sh
cmake --preset windows-release
cmake --build --preset windows-release --parallel 4
ctest --test-dir build/windows-release -C Release --output-on-failure
```

Windows FFmpeg headers and DLLs are included in the repository. To use another compatible build, set `CANE_FFMPEG_ROOT`. The original Visual Studio solution also remains available: build **File Extractor**, extract your ISO, and run **Sly1-Release** beside the extracted data.

For Linux-to-Windows cross builds, point `CANE_MINGW_ROOT` at LLVM/MinGW and use `windows-cross`. The cross build downloads Microsoft's pinned XAudio2 redistributable. Cross compilation verifies the build; native Windows gameplay testing is still required.

The launcher's **Build from source** screen accepts a portable ProjectCane checkout, builds and installs a matching runtime/extractor into your user profile, and selects them after successful installation. **Use bundled runtime** switches back. Build failures preserve the previous runtime and game assets.

CMake's `FETCHCONTENT_SOURCE_DIR_GLFW` and `FETCHCONTENT_SOURCE_DIR_FAUDIO` and `FETCHCONTENT_SOURCE_DIR_SDL2` overrides allow local dependency checkouts. SDL 2.24+ is required by FAudio; an older system SDL triggers the pinned fallback. Source builds still need the platform's development libraries.

## Packaging and diagnostics

```sh
python3 scripts/package.py --build build/linux-release
python3 scripts/package.py --build build/linux-release --appimagetool /path/to/appimagetool
python scripts/package.py --build build/windows-release --windows
# ZIP only, without an installer compiler:
python scripts/package.py --build build/windows-release --windows --windows-package portable
```

Windows setup packaging requires NSIS 3 (`choco install nsis` on Windows or `apt install nsis` on Ubuntu). Use `--makensis /path/to/makensis` for a custom compiler location. [NSIS build and command-line documentation](https://nsis.sourceforge.io/Docs/Chapter3.html) covers the compiler and silent setup options. Setup accepts `/S` for silent installation and `/DESKTOP` to request the optional desktop shortcut.

The outputs are in `dist/`. Linux bundles collect shared runtime dependencies but use the operating system's C/C++ runtimes and graphics drivers (Ubuntu 22.04 or newer). Release CI builds Linux on Ubuntu 22.04 and Windows with MSVC. Locally built bundles inherit their build machine's libc requirement.

The launcher also provides `--install ISO`, `--verify`, `--play`, and `--build-source CHECKOUT` for automation. The extractor supports:

```sh
cane-extractor --iso game.iso --output /path/to/data --non-interactive --progress=jsonl
cane-extractor --iso game.iso --validate-only
```

Structured progress goes to stdout; extraction diagnostics go to stderr. Explicit-output installations use staging, checksums, an installer lock, and interrupted-activation recovery. Legacy positional extraction continues to write beside the extractor.

Runtime options include `--data-dir PATH` and `--user-dir PATH`. For automated graphics checks, use `--smoke-test-seconds 10 --smoke-level Paris --screenshot frame.png`; the test renders at least 120 frames and reports its loaded world. Startup movies can extend the requested duration.

`ctest` checks extractor input rejection and launcher profile locations. Additional local tests are built when the ignored `tests/` sources are present. Save ABI sizes and offsets are checked at compilation. To validate a local disc without extracting it:

```sh
python3 scripts/test-extractor.py build/linux-release/bin/cane-extractor --iso /path/to/game.iso
```

## Status

The full game is playable. Development is continuing with bug fixes, compatibility improvements.

Dependency notices are shipped in `licenses/`. Credits to k1nomoto for the project logo.

## Help Bring Sly 2 and Sly 3 to PC

ProjectCane is not intended to stop with *Sly Cooper and the Thievius Raccoonus*. The goal is to bring *Sly 2: Band of Thieves* and *Sly 3: Honor Among Thieves* to PC as native ports as well.

If you know where to find a debug build of *Sly 2* or *Sly 3* that includes debug symbols, please let me know. You can contact me at [projectcane182@gmail.com](mailto:projectcane182@gmail.com).

If you would like to see native PC ports of all three games up and running, any information that can help make that possible would be greatly appreciated. Thank you!
