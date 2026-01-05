#include "IDE.h"
#include "Simulation.h"
#include "imgui.h"
#include "rlImGui.h"
#include <cstring>

IDE::IDE() {
    // Default code
    code = "void loop() {\n"
           "    if (ldist > 30) {\n"
           "        left();\n"
           "        forward();\n"
           "    } else if (rdist > 30) {\n"
           "        right();\n"
           "        forward();\n"
           "    } else if (Fdist > 30) {\n"
           "        forward();\n"
           "    } else if (Fdist < 30 && ldist < 30 && rdist < 30) {\n"
           "        right();\n"
           "        right();\n"
           "    }\n"
           "}\n";
    goBack = false;
}

void IDE::AutoFormat() {
    std::string formatted;
    int indent = 0;
    bool newLine = true;
    
    for (int i = 0; i < code.length(); i++) {
        char c = code[i];
        
        if (c == '{') {
            formatted += " {\n";
            indent++;
            newLine = true;
        } else if (c == '}') {
            indent--;
            if (!newLine) formatted += "\n";
            for (int j = 0; j < indent; j++) formatted += "    ";
            formatted += "}\n";
            newLine = true;
        } else if (c == ';') {
            formatted += ";\n";
            newLine = true;
        } else if (c == '\n') {
            // Skip existing newlines to avoid double spacing
        } else {
            if (newLine) {
                for (int j = 0; j < indent; j++) formatted += "    ";
                newLine = false;
            }
            // Skip multiple spaces
            if (c == ' ' && i + 1 < code.length() && code[i+1] == ' ') continue;
            formatted += c;
        }
    }
    code = formatted;
}

void IDE::Draw(MazeGenerator& maze, Simulation& simulation) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Layout:
    // Left Half: Code Editor
    // Right Half: Split Vertically
    //   - Top Right: Robot Preview
    //   - Bottom Right: Maze Preview
    
    int halfWidth = screenWidth / 2;
    int halfHeight = screenHeight / 2;
    
    // --- Left Side: Code Editor ---
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)halfWidth, (float)screenHeight});
    ImGui::Begin("Code Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Write your robot code here:");
    // InputTextMultiline (using std::string buffer wrapper would be better, but fixed size for now)
    static char codeBuffer[4096];
    if (code.length() < 4096) strcpy(codeBuffer, code.c_str());
    
    if (ImGui::InputTextMultiline("##code", codeBuffer, 4096, {(float)halfWidth - 20, (float)screenHeight - 140}, ImGuiInputTextFlags_AllowTabInput)) {
        code = std::string(codeBuffer);
    }
    
    // Speed Control
    ImGui::Text("Simulation Speed (Step Delay):");
    ImGui::SliderFloat("##speed", &simulation.stepDelay, 0.1f, 2.0f, "%.1f s");
    
    if (ImGui::Button("<- Back to Maze Generator")) {
        goBack = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Format Code")) {
        AutoFormat();
    }
    
    ImGui::End();
    
    // --- Top Right: Robot Preview ---
    DrawRobotPreview(halfWidth, 0, halfWidth, halfHeight);
    
    // --- Bottom Right: Maze Preview ---
    DrawMazePreview(maze, halfWidth, halfHeight, halfWidth, halfHeight);
}

void IDE::DrawRobotPreview(int x, int y, int w, int h) {
    // Background
    DrawRectangle(x, y, w, h, RAYWHITE);
    DrawRectangleLines(x, y, w, h, LIGHTGRAY);
    DrawText("Robot Preview", x + 10, y + 10, 20, DARKGRAY);
    
    // Center of this quadrant
    int centerX = x + w / 2;
    int centerY = y + h / 2;
    
    // Draw Robot (Top Down View)
    
    // Chassis (Acrylic Plate)
    DrawRectangle(centerX - 30, centerY - 80, 60, 160, Fade(LIGHTGRAY, 0.5f)); // Main body
    DrawRectangleLines(centerX - 30, centerY - 80, 60, 160, GRAY);
    
    // Motors (Yellow DC Motors)
    DrawRectangle(centerX - 45, centerY - 20, 15, 40, YELLOW); // Left Motor
    DrawRectangle(centerX + 30, centerY - 20, 15, 40, YELLOW); // Right Motor
    
    // Wheels (Black Tires + Yellow Rims)
    // Left Wheel
    DrawRectangle(centerX - 60, centerY - 30, 15, 60, BLACK);
    DrawRectangle(centerX - 55, centerY - 25, 5, 50, YELLOW);
    // Right Wheel
    DrawRectangle(centerX + 45, centerY - 30, 15, 60, BLACK);
    DrawRectangle(centerX + 50, centerY - 25, 5, 50, YELLOW);
    
    // Caster Wheel (Front)
    DrawCircle(centerX, centerY - 70, 8, BLACK);
    DrawCircle(centerX, centerY - 70, 4, LIGHTGRAY);
    
    // Components
    // Arduino Uno (Blue-ish)
    DrawRectangle(centerX - 25, centerY + 10, 50, 60, {0, 100, 100, 255});
    // Mini Breadboard (White)
    DrawRectangle(centerX - 20, centerY - 60, 40, 30, WHITE);
    
    // Sensors (Ultrasonic)
    // Front (Mounted on Front Edge)
    DrawRectangle(centerX - 20, centerY - 100, 40, 10, BLUE);
    DrawCircle(centerX - 10, centerY - 105, 5, LIGHTGRAY); // Eye L
    DrawCircle(centerX + 10, centerY - 105, 5, LIGHTGRAY); // Eye R
    
    // Left Sensor (Mounted on Left Edge)
    DrawRectangle(centerX - 40, centerY - 80, 10, 40, BLUE);
    DrawCircle(centerX - 45, centerY - 70, 5, LIGHTGRAY); // Eye F
    DrawCircle(centerX - 45, centerY - 50, 5, LIGHTGRAY); // Eye B
    
    // Right Sensor (Mounted on Right Edge)
    DrawRectangle(centerX + 30, centerY - 80, 10, 40, BLUE);
    DrawCircle(centerX + 45, centerY - 70, 5, LIGHTGRAY); // Eye F
    DrawCircle(centerX + 45, centerY - 50, 5, LIGHTGRAY); // Eye B
    
    // Command List (Bottom Left of Preview)
    int cmdX = x + 10;
    int cmdY = y + h - 140;
    DrawText("Commands:", cmdX, cmdY, 20, DARKGRAY);
    DrawText("- forward(), backward()", cmdX, cmdY + 25, 10, BLACK);
    DrawText("- left(), right() (90 deg)", cmdX, cmdY + 40, 10, BLACK);
    DrawText("- stop()", cmdX, cmdY + 55, 10, BLACK);
    DrawText("Variables:", cmdX, cmdY + 75, 20, DARKGRAY);
    DrawText("- fdist, ldist, rdist", cmdX, cmdY + 100, 10, BLACK);
}

void IDE::DrawMazePreview(MazeGenerator& maze, int x, int y, int w, int h) {
    // Background
    DrawRectangle(x, y, w, h, LIGHTGRAY);
    DrawRectangleLines(x, y, w, h, GRAY);
    DrawText("Maze Map", x + 10, y + 10, 20, DARKGRAY);
    
    // Draw Maze Thumbnail
    // We need to scale the maze to fit in (w-20, h-40)
    
    float cellSize = 10.0f; // Start small
    float availW = (float)w - 40;
    float availH = (float)h - 40;
    
    float scaleX = availW / (maze.width * cellSize);
    float scaleY = availH / (maze.height * cellSize);
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    
    float finalCellSize = cellSize * scale;
    
    float startX = x + 20;
    float startY = y + 30;
    
    for (const auto& cell : maze.grid) {
        float cx = startX + cell.x * finalCellSize;
        float cy = startY + cell.y * finalCellSize;
        
        if (cell.wallNorth) DrawLineEx({cx, cy}, {cx + finalCellSize, cy}, 1.0f, BLACK);
        if (cell.wallSouth) DrawLineEx({cx, cy + finalCellSize}, {cx + finalCellSize, cy + finalCellSize}, 1.0f, BLACK);
        if (cell.wallEast) DrawLineEx({cx + finalCellSize, cy}, {cx + finalCellSize, cy + finalCellSize}, 1.0f, BLACK);
        if (cell.wallWest) DrawLineEx({cx, cy}, {cx, cy + finalCellSize}, 1.0f, BLACK);
    }
    
    // Draw Start Position (Bottom Center)
    float robotX = startX + (maze.width / 2) * finalCellSize + finalCellSize/2;
    float robotY = startY + (maze.height - 1) * finalCellSize + finalCellSize/2;
    DrawCircle(robotX, robotY, finalCellSize/2, RED);
}
