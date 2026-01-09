cmake_minimum_required(VERSION 3.18)
project(MazeRoboSim)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# --- Raylib ---
# Changed from URL 5.0 to GIT_REPOSITORY master to fix 'IsTextureValid' error
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/heads/master.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
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

# ImGui Sources
set(IMGUI_DIR ${imgui_SOURCE_DIR})
list(APPEND SOURCES 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# rlImGui Sources - Manual addition to ensure it compiles correctly with your project
list(APPEND SOURCES ${rlimgui_SOURCE_DIR}/rlImGui.cpp)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
    # Raylib requires these system libs on Windows
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32 opengl32 shell32)
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

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()