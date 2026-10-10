<p align="center">
  <img alt="ProjectCane" src="README-assets/projectcane-logo.png" />
</p>

# ProjectCane

ProjectCane is a native PC port of the NTSC release of *Sly Cooper and the
Thievius Raccoonus*. It began as a map viewer and has grown into a complete,
playable port for Windows and Linux.

> [!IMPORTANT]
> ProjectCane does not include the original game's copyrighted data. You must
> provide a legally obtained NTSC copy of the game and extract its data with
> the included File Extractor GUI.

## Download

Prebuilt Windows and Linux packages are available from the
[GitHub Releases page](https://github.com/theclub654/ProjectCane/releases).

The Linux release is distributed as a portable x86-64 AppImage and is built
against Ubuntu 22.04 / glibc 2.35 for compatibility with SteamOS and other
modern Linux distributions.

## Installation

### Windows

1. Download and extract the Windows release into a normal writable folder.
2. Run the included **File Extractor GUI**.
3. Select your legally obtained NTSC ISO.
4. Choose the same folder that contains ProjectCane as the installation
   destination.
5. Wait for extraction to finish, then launch
   `Sly Cooper and the Thievius Raccoonus.exe`.

The extractor creates the required `Worlds` and `Cutscenes` directories.
Keep these directories beside the game executable.

### Linux and Steam Deck

1. Download and extract the complete Linux package into a normal writable
   folder. Do not run it from inside the archive.
2. Open a terminal in that folder. Steam Deck users can use **Konsole** in
   Desktop Mode.
3. Start the extractor:

   ```bash
   bash "Run File Extractor.sh"
   ```

4. Select your NTSC ISO and extract the game data into the same folder as
   `ProjectCane-x86_64.AppImage`.
5. Start the game:

   ```bash
   bash "Run ProjectCane.sh"
   ```

The launch scripts automatically set the required executable permissions.
The AppImage includes the non-system runtime libraries needed by ProjectCane;
graphics drivers and normal operating-system libraries are supplied by the
host system.

## Save Data and Settings

On Windows, writable game data is stored beside the executable, including the
`Saves` directory.

On Linux, they are stored in:

```text
~/.local/share/ProjectCane
```

If `XDG_DATA_HOME` is configured, ProjectCane uses
`$XDG_DATA_HOME/ProjectCane` instead.

## Troubleshooting

Make sure the extracted `Worlds` and `Cutscenes` directories are beside the
game executable or AppImage.

To capture a Linux startup log, run:

```bash
./ProjectCane-x86_64.AppImage 2>&1 | tee projectcane_startup.log
```

When reporting a problem, include the log, your operating system, GPU, CPU,
audio device, and the steps needed to reproduce the issue.

## Building from Source

### Windows with Visual Studio

Requirements:

- Visual Studio 2022
- **Desktop development with C++** workload
- .NET 9 SDK for the File Extractor GUI

Open `ProjectCane.sln`, select the `Release` and `x64` configuration, and
build the `Sly1-Release` project. The shaders are embedded into the executable
at build time, so external shader files are not required.

The cross-platform extractor is the `File Extractor GUI` project in the same
solution.

### Windows with CMake

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
```

### Linux development build

The Linux build requires CMake 3.22 or newer, Ninja, a C++20 compiler,
PkgConfig, and FFmpeg development packages providing `libavformat`,
`libavcodec`, `libavutil`, and `libswscale`.

```bash
cmake --preset linux-release
cmake --build --preset linux-release
```

The executable is written to `build/linux-release/bin/ProjectCane`.

### Portable Linux AppImage

Distribution builds must use the Ubuntu 22.04 / glibc 2.35 baseline. Detailed
Docker and packaging instructions are available in
[`packaging/linux/README.md`](packaging/linux/README.md).

## Project Status

The full game is playable. Development continues to focus on accuracy,
performance, controller support, platform compatibility, and remaining
differences from the original PlayStation 2 release.

## Help Bring Sly 2 and Sly 3 to PC

ProjectCane is not intended to stop with *Sly Cooper and the Thievius
Raccoonus*. The long-term goal is to bring *Sly 2: Band of Thieves* and
*Sly 3: Honor Among Thieves* to PC as native ports as well.

If you know where to find a debug build of *Sly 2* or *Sly 3* containing debug
symbols, please contact [projectcane182@gmail.com](mailto:projectcane182@gmail.com).

## Legal

ProjectCane is an unofficial fan project and is not affiliated with or
endorsed by Sony Interactive Entertainment or Sucker Punch Productions.
*Sly Cooper* and related names and assets belong to their respective owners.
No original game data is distributed with this repository or its releases.
