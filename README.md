# MazeRoboSim

**MazeRoboSim** is a native C++ 2D robotics simulator designed to teach basic maze-solving algorithms. It features a complete workflow from maze generation to coding and simulation.

## Features

### 1. Maze Generator
- **Procedural Generation**: Uses a Recursive Backtracker algorithm to create perfect mazes.
- **Customizable**: Adjust maze dimensions and settings.
- **Preview**: See the maze layout before programming.

### 2. Integrated Development Environment (IDE)
- **Simplified C++**: Write code using high-level commands like `forward()`, `left()`, and `right()`.
- **Robot Preview**: Visual representation of the robot chassis with sensors.
- **Command Reference**: Handy list of available commands and variables.
- **Split View**: Code on the left, Robot and Maze preview on the right.

### 3. Simulation
- **Real-Time Physics**: The robot moves and interacts with the maze walls.
- **Raycast Sensors**: Accurate simulation of ultrasonic sensors (`fdist`, `ldist`, `rdist`).
- **Visual Feedback**: See the robot navigate the maze in real-time.

## Prerequisites

- **CMake** (3.18 or newer)
- **C++ Compiler** (GCC, Clang, or MSVC)
- **Internet Connection** (For fetching Raylib and ImGui dependencies)

## Build and Run Instructions

This project uses CMake to automatically fetch dependencies (Raylib, ImGui) and build the project.

### Windows (with Visual Studio)

1.  **Prerequisites**:
    *   [Visual Studio](https://visualstudio.microsoft.com/vs/community/) with the "Desktop development with C++" workload.
    *   [CMake](https://cmake.org/download/) (select "Add CMake to the system PATH").
    *   [Git](https://git-scm.com/download/win) (select "Add Git to system PATH").

2.  **Clone the repository**:
    ```bash
    git clone https://github.com/Not1Sam/MazeRoboSim.git
    cd MazeRoboSim
    ```

3.  **Configure and Build**:
    ```bash
    mkdir build
    cd build
    cmake .. -G "Visual Studio 17 2022" 
    cmake --build . --config Release
    ```
    *Note: Adjust the generator `-G` if you are using a different version of Visual Studio. If you encounter a `WinMain` linker error, this is already handled in the `CMakeLists.txt` for you.*

4.  **Run**:
    The executable `MazeRoboSim.exe` will be in the `build/Release` folder.

### Linux (Debian/Ubuntu)

1.  **Prerequisites**:
    ```bash
    sudo apt update
    sudo apt install build-essential cmake libgl1-mesa-dev libxi-dev libxcursor-dev libxrandr-dev libxinerama-dev libx11-dev libglfw3-dev libopenal-dev
    ```

### Linux (Arch)

1.  **Prerequisites**:
    ```bash
    sudo pacman -Syu
    sudo pacman -S base-devel cmake mesa libxi libxcursor libxrandr libxinerama libx11 glfw-x11 openal
    ```

2.  **Clone the repository**:
    ```bash
    git clone https://github.com/Not1Sam/MazeRoboSim.git
    cd MazeRoboSim
    ```
    
3.  **Configure and Build**:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

4.  **Run**:
    ```bash
    ./MazeRoboSim
    ```

## Usage Guide

1.  **Design**: Configure your maze settings and click **"Proceed to Programming"**.
2.  **Code**: Write your logic in the IDE.
    *   **Commands**: `forward()`, `backward()`, `left()` (90° Snap), `right()` (90° Snap), `stop()`.
    *   **Variables**: `fdist` (Front), `ldist` (Left), `rdist` (Right), `int` variables.
    *   **Operators**: `+`, `-`, `<`, `>`, `? :` (Ternary).
    *   **Execution**: Commands run one at a time with a **1-second delay** for easy debugging.
    *   **Example** (Left-Hand Rule):
        ```cpp
        void loop() {
          if (ldist > 20) {
            left();
            forward();
          } else if (fdist > 20) {
            forward();
          } else {
            right();
          }
        }
        ```
3.  **Simulate**: Click **"Start Simulation"** to watch your code run!
