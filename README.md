# ProjectCane

![ProjectCane logo](https://github.com/user-attachments/assets/49b1b7d4-87db-4134-8f52-dbef28a79d54)

ProjectCane is a playable PC port of the NTSC release of *Sly Cooper and the Thievius Raccoonus*. The project originally began as a map viewer, but it has since grown into a complete playable port.

> **Platform support:** ProjectCane currently supports Windows only. Linux compatibility is planned for a future release.

## Requirements

- Windows
- Visual Studio 2022 with the **Desktop development with C++** workload
- A legally obtained NTSC ISO of *Sly Cooper and the Thievius Raccoonus*

ProjectCane does not include the original game's copyrighted data. You must extract the required files from your own NTSC copy of the game.

## Setup

1. Clone or download this repository.
2. Open `ProjectCane.sln` in Visual Studio 2022.
3. In **Solution Explorer**, right-click the **File Extractor** project and select **Set as Startup Project**.
4. Build the **File Extractor** project.
5. Drag your NTSC game ISO onto the built File Extractor executable.
6. Wait for the extraction process to finish completely.
7. Return to Visual Studio, right-click the **Sly1-Release** project, and select **Set as Startup Project**.
8. Build the **Sly1-Release** project.
9. Copy the built `Sly1-Release.exe` into the same directory as the extracted game files.
10. Run `Sly1-Release.exe` to play.

No external shader files are required—the shaders are embedded in the executable.

## Status

The full game is playable. Development is continuing with bug fixes, compatibility improvements, and future Linux support.
