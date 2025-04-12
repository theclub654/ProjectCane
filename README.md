
![Screenshot 2025-04-12 021516](https://github.com/user-attachments/assets/feb25918-f559-4501-b63a-0eaab2dfc097)

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
1. In **Solution Explorer**, right-click on `Sly1-Proto`
2. Select **Set as Startup Project**
3. Click **Build** → **Run**

### 📦 Retail Release
1. In **Solution Explorer**, right-click on `Sly1-Release`
2. Select **Set as Startup Project**
3. Click **Build** → **Run**

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
