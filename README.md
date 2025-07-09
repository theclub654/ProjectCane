
![Screenshot 2025-04-12 104934](https://github.com/user-attachments/assets/427a43f6-d1aa-49eb-83a7-8330da425520)
![Logo 300](https://github.com/user-attachments/assets/49b1b7d4-87db-4134-8f52-dbef28a79d54)



# Sly Trilogy Decompilation Project

## 📖 Project Description

This project aims to **decompile all three Sly Cooper games**, gain a deep understanding of their internal logic, and lay the groundwork for a functional **PC port**. By reverse engineering the games, we make modding significantly easier and prepare the codebase for future enhancements.

As part of this process, we will **rewrite parts of the code** where necessary to ensure clean structure, modern compatibility, and portability across platforms.

> 💻 **Note:** Only **Windows** is currently supported. Other platforms will be considered once reasonable progress is made.

---

## 🎯 Current Objective

Our current focus is to:
- Reverse engineer the **May 19, 2002 prototype** of Sly 1
- Use this prototype as a foundation to reconstruct and better understand the **retail release**
- Maintain and compare **both versions side by side** throughout development

---

## ⚙️ Building the Project

To compile the project, use the instructions below based on your desired target:

### 🧪 May 19, 2002 Prototype
1. Put `blot.vert` and `blot.frag` and `glob.vert` and `glob.frag` and `screen.vert` and `screen.frag` in the same folder as the exe of where its gonna be built at
2. In **Solution Explorer**, right-click on `Sly1-Proto`
3. Select **Set as Startup Project**
4. Click **Build** → **Run**

### 📦 Retail Release
1. Use level extractor to extract level files from NTSC ISO right here https://github.com/theclub654/Sly-1-File-Extractor
2. Put `blot.vert` and `blot.frag` and `glob.vert` and `glob.frag` and `screen.vert` and `screen.frag` in the same folder as the exe of where its gonna be built at
3. In **Solution Explorer**, right-click on `Sly1-Release`
4. Select **Set as Startup Project**
5. Click **Build** → **Run**

---

## ⚠️ Guidelines

Please follow these best practices when contributing:

- ✅ **Keep all game assets** intact across all branches
- ✅ **Avoid raw copy-paste from Ghidra** — always clean up and format code for readability
- ✅ **Understand what you're decompiling** — aim to interpret and translate behavior, not just transcribe
- ✅ **Preserve accuracy** while making code maintainable

Your effort in maintaining clean and understandable code will benefit the whole community and help build a strong foundation for future work — including full PC support and modding tools.

---

## 🛠️ Contributing

If you're interested in helping out, feel free to:
- Submit pull requests with your cleaned and understood code
- Share findings about game logic or behavior
- Help document function purposes, structures, and quirks

Let’s reverse-engineer some raccoon chaos! 🦝🔥
