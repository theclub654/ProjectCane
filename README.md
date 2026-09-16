<<<<<<< HEAD
<img width="1919" height="1029" alt="Screenshot 2026-06-08 140318" src="https://github.com/user-attachments/assets/ea5af858-ed42-49d4-b18b-cbc0ca772c9b" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135833" src="https://github.com/user-attachments/assets/9d14a2b3-360a-4da5-8574-07e0a42e7511" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135925" src="https://github.com/user-attachments/assets/8b8e737b-4283-429b-924f-47235f696829" />
<img width="1919" height="1029" alt="Screenshot 2026-06-08 135942" src="https://github.com/user-attachments/assets/198ce439-b7fc-423b-acab-14dee85ccf04" />
<img width="1919" height="1034" alt="Screenshot 2026-06-08 140000" src="https://github.com/user-attachments/assets/f43787b3-8a96-4e46-bf36-5be2287c8e7f" />
<img width="1919" height="1027" alt="Screenshot 2026-06-08 140043" src="https://github.com/user-attachments/assets/f1b00742-cbc1-4ef8-9485-fe2a15877fa8" />
<img width="1919" height="1027" alt="Screenshot 2026-06-08 140026" src="https://github.com/user-attachments/assets/56d6e3e4-db35-4bc3-879f-b41c119340ba" />
=======
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

## Screenshots

<img width="803" height="830" alt="Paris" src="https://github.com/user-attachments/assets/538d6877-42ef-4b98-b46a-839490ca85b7" />
<img width="799" height="826" alt="Tide of Terror" src="https://github.com/user-attachments/assets/a36af8e0-c375-4f6a-a7f3-ca591f53d646" />
<img width="804" height="831" alt="Muggshot" src="https://github.com/user-attachments/assets/87411ed4-5b23-4cc6-82d9-5d697f05479b" />
<img width="804" height="824" alt="Mz. Ruby" src="https://github.com/user-attachments/assets/90a8a31c-a45a-4008-ad24-a8582763a274" />
<img width="802" height="830" alt="Panda King" src="https://github.com/user-attachments/assets/7e464eaa-c344-4e0e-bb75-79d170119a6e" />
<img width="798" height="828" alt="Clockwerk" src="https://github.com/user-attachments/assets/88d73890-190e-490d-a3e5-52b6bc456181" />
>>>>>>> 35d389a (Complete playable Sly Cooper PC port)

## Status

<<<<<<< HEAD


![Logo 300](https://github.com/user-attachments/assets/49b1b7d4-87db-4134-8f52-dbef28a79d54)



# Sly Trilogy Map Viewer Project

## 📖 Project Description

Simple map viewer for sly 1 right now proto and release version

> 💻 **Note:** Only **Windows** is currently supported. Other platforms will be considered once reasonable progress is made.

---

## ⚙️ Building the Project

To compile the project, use the instructions below based on your desired target:

### 🧪 May 19, 2002 Prototype
1. Place the following shader files in the same directory as the executable (where the project will be built):
   - `blot.vert`
   - `blot.frag`
   - `glob.vert`
   - `glob.frag`
   - `screen.vert`
   - `screen.frag`
2. In **Solution Explorer**, right-click on `Sly1-Proto`
3. Select **Set as Startup Project**
4. Click **Build** → **Run**

### 📦 Retail Release
1. Use the [Sly 1 File Extractor](https://github.com/theclub654/Sly-1-File-Extractor) to extract level files from the NTSC ISO.
2. Place the following shader files in the same directory as the executable (where the project will be built):
   - `blot.vert`
   - `blot.frag`
   - `glob.vert`
   - `glob.frag`
   - `geom.vert`
   - `geom.frag`
   - `dysh.vert`
   - `dysh.frag`
   - `screen.vert`
   - `screen.frag`
3. In **Solution Explorer**, right-click on `Sly1-Release`
4. Select **Set as Startup Project**
5. Click **Build** → **Run**

---
=======
The full game is playable. Development is continuing with bug fixes, compatibility improvements, and future Linux support.
>>>>>>> 35d389a (Complete playable Sly Cooper PC port)
