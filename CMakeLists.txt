cmake_minimum_required(VERSION 3.18)
project(MazeRoboSim)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# --- Raylib ---
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    PATCH_COMMAND sed -i "s/cmake_minimum_required(VERSION 3.0)/cmake_minimum_required(VERSION 3.5)/" <SOURCE_DIR>/CMakeLists.txt
)
FetchContent_MakeAvailable(raylib)

# --- ImGui ---
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/heads/docking.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(imgui)

# --- rlImGui ---
FetchContent_Declare(
    rlImGui
    URL https://github.com/raylib-extras/rlImGui/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(rlImGui)

# --- Sources ---
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add rlImGui source manually if needed or link library
# rlImGui usually provides a library target if built correctly, but sometimes needs manual source addition
# Let's check if rlImGui target exists, otherwise add sources
if(NOT TARGET rlImGui)
    # Fallback: Add sources manually
    set(RLIMGUI_DIR ${rlimgui_SOURCE_DIR})
    list(APPEND SOURCES ${RLIMGUI_DIR}/rlImGui.cpp)
endif()

# ImGui Sources
set(IMGUI_DIR ${imgui_SOURCE_DIR})
list(APPEND SOURCES 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32 opengl32)
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

# --- Includes ---
target_include_directories(${PROJECT_NAME} PRIVATE 
    src 
    ${IMGUI_DIR}
    ${rlimgui_SOURCE_DIR}
)

# --- Linking ---
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
if(TARGET rlImGui)
    target_link_libraries(${PROJECT_NAME} PRIVATE rlImGui)
endif()
