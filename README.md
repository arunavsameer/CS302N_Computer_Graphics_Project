# Crazy Hopper - CS302N Computer Graphics Project

> A computer graphics project built with OpenGL, CMake, FreeGLUT, GLEW, and GLM.

---

## Table of Contents

- [Prerequisites](#prerequisites)
  - [Linux](#linux)
  - [macOS](#macos)
  - [Windows](#windows)
- [Project Setup](#project-setup)
- [Building & Running](#building--running)
- [Fallback — Manual Compilation with g++](#fallback--manual-compilation-with-g)

---

## Prerequisites

Install the required dependencies for your operating system before proceeding.

### Linux

**Arch Linux**
```bash
sudo pacman -S base-devel cmake freeglut glew glm
```

**Debian / Ubuntu**
```bash
sudo apt update && sudo apt install build-essential cmake freeglut3-dev libglew-dev libglm-dev libgl1-mesa-dev libglu1-mesa-dev
```

---

### macOS

> **Requires [Homebrew](https://brew.sh/).** If you don't have it installed, run the following first:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then install the dependencies:

```bash
brew install cmake freeglut glew glm
```

> **Note:** Apple deprecated OpenGL in macOS 10.14 (Mojave). The project will still build and run on newer versions, but you may see deprecation warnings during compilation — these are safe to ignore.

---

### Windows

You have two options. Try **Option A** first; if it doesn't work, use **Option B**.

#### Option A — vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install freeglut:x64-windows glew:x64-windows glm:x64-windows
```

> **CMake Integration:** When generating build files, point CMake to your vcpkg toolchain by appending:
> ```
> -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
> ```

#### Option B — MSYS2 *(fallback)*

1. Download and install **MSYS2** from [https://www.msys2.org/](https://www.msys2.org/)
2. Open the **MSYS2 UCRT64** terminal
3. Run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc \
          mingw-w64-ucrt-x86_64-freeglut \
          mingw-w64-ucrt-x86_64-glew \
          mingw-w64-ucrt-x86_64-glm
```

---

## Project Setup

Follow these steps in order after installing all prerequisites.

**1. Navigate to your desired installation directory and open a terminal there.**

**2. Clone the repository:**
```bash
git clone https://github.com/arunavsameer/CS302N_Computer_Graphics_Project.git
```

**3. Enter the project directory:**
```bash
cd CS302N_Computer_Graphics_Project
```

**4. Create a build folder and navigate into it:**
```bash
mkdir build
cd build
```

---

## Building & Running

**5. Generate CMake build files:**

```bash
cmake ..
```

> **Windows (vcpkg users only):** Append the toolchain file path:
> ```bash
> cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
> ```

**6. Compile the project:**
```bash
cmake --build .
```

**7. Run the project:**

| Platform       | Command                         |
|----------------|---------------------------------|
| Linux / macOS  | `./bin/crazy_hopper`            |
| Windows        | `.\bin\Debug\crazy_hopper.exe`  |

---

## Fallback — Manual Compilation with g++

If the CMake build isn't working, you can compile directly using `g++` from the **project root directory**. Make sure you've completed all the prerequisite installation steps first.

**Linux**
```bash
g++ -std=c++17 -o ./crazy_hopper main.cpp src/*.cpp -Iinclude '-DASSET_DIR="./assets/"' -lGL -lGLU -lGLEW -lglut -lm
```

**macOS**
```bash
g++ -std=c++17 -o ./crazy_hopper main.cpp src/*.cpp -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib '-DASSET_DIR="./assets/"' -framework OpenGL -framework GLUT -lGLEW -lm
```
> **Note:** The paths `-I/opt/homebrew/include` and `-L/opt/homebrew/lib` are for **Apple Silicon** Macs. If you're on an **Intel Mac**, replace them with `-I/usr/local/include` and `-L/usr/local/lib`.

**Windows** *(requires the [MSYS2 fallback setup](#option-b--msys2-fallback) from the prerequisites)*
```bash
g++ -std=c++17 -o ./crazy_hopper.exe main.cpp src/*.cpp -Iinclude "-DASSET_DIR=\"./assets/\"" -lopengl32 -lfreeglut -lglew32 -lglu32
```

Once compiled, run the game from the project root directory:

| Platform       | Command              |
|----------------|----------------------|
| Linux / macOS  | `./crazy_hopper`     |
| Windows        | `./crazy_hopper.exe` |

---

<div align="center">

Made with love for CS302N — Computer Graphics

</div>
