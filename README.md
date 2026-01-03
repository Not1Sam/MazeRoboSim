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

## Build Instructions

### Linux (Arch/Ubuntu/Debian)

1.  **Install Dependencies** (if needed):
    *   Arch: `sudo pacman -S cmake base-devel`
    *   Ubuntu/Debian: `sudo apt install cmake build-essential libgl1-mesa-dev libxi-dev libxcursor-dev`

2.  **Build**:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3.  **Run**:
    ```bash
    ./MazeRoboSim
    ```

### Windows

1.  **Install CMake**: Download from [cmake.org](https://cmake.org/download/).
2.  **Install Compiler**: MinGW-w64 or Visual Studio (Desktop development with C++).
3.  **Build**:
    ```powershell
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```
4.  **Run**:
    Execute `MazeRoboSim.exe` from the `build/Debug` or `build` folder.

## Usage Guide

1.  **Design**: Configure your maze settings and click **"Proceed to Programming"**.
2.  **Code**: Write your logic in the IDE.
    *   **Commands**: `forward()`, `backward()`, `left()` (90° Snap), `right()` (90° Snap), `stop()`.
    *   **Variables**: `fdist` (Front), `ldist` (Left), `rdist` (Right).
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
