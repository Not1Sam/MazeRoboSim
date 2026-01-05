#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "UI.h"
#include "MazeGenerator.h"
#include "IDE.h"
#include "Simulation.h"

enum AppState {
    STATE_DESIGNER,
    STATE_IDE,
    STATE_SIMULATION
};

int main() {
    // Initialization
    const int screenWidth = 1280;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "MazeRoboSim");
    SetTargetFPS(60);
    
    rlImGuiSetup(true);
    
    // Application State
    AppState currentState = STATE_DESIGNER;
    
    MazeGenerator generator;
    UI ui(generator);
    IDE ide;
    Simulation simulation;

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        switch (currentState) {
            case STATE_DESIGNER:
                if (ui.ShouldProceed()) {
                    currentState = STATE_IDE;
                    ui.Reset(); // Reset flag so we don't auto-switch back if we return
                    ide.Reset();
                }
                break;
            case STATE_IDE:
                if (ide.ShouldGoBack()) {
                    currentState = STATE_DESIGNER;
                    ide.Reset();
                }
                break;
            case STATE_SIMULATION:
                simulation.Update();
                break;
        }
        
        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            switch (currentState) {
                case STATE_DESIGNER:
                    generator.Draw();
                    rlImGuiBegin();
                    ui.Draw();
                    rlImGuiEnd();
                    break;
                    
                case STATE_IDE:
                    rlImGuiBegin();
                    ide.Draw(generator, simulation);
                    
                    // Button to start simulation (Overlay)
                    ImGui::SetNextWindowPos({(float)screenWidth - 220, (float)screenHeight - 60});
                    ImGui::Begin("SimControl", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
                    if (ImGui::Button("Start Simulation", {200, 40})) {
                        simulation.Init(generator, ide.code);
                        currentState = STATE_SIMULATION;
                    }
                    ImGui::End();
                    
                    rlImGuiEnd();
                    break;
                    
                case STATE_SIMULATION:
                    generator.Draw(); // Draw maze background
                    simulation.Draw();
                    
                    rlImGuiBegin();
                    // Back button
                    ImGui::SetNextWindowPos({10, 10});
                    ImGui::Begin("SimUI", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
                    if (ImGui::Button("Back to IDE")) {
                        currentState = STATE_IDE;
                    }
                    ImGui::End();
                    rlImGuiEnd();
                    break;
            }

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
