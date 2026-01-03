#include "UI.h"
#include "imgui.h"

UI::UI(MazeGenerator& gen) : generator(gen) {
    width = 20;
    height = 20;
    innerWidth = 0;
    innerHeight = 0;
    shapeIndex = 0;
    styleIndex = 0;
    startIndex = 0;
    elaborateness = 100;
    removeDeadEnds = 0;
    proceedToIDE = false;
}

void UI::Draw() {
    ImGui::SetNextWindowPos({20, 20}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({350, 600}, ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Maze Generator", nullptr, ImGuiWindowFlags_NoCollapse);
    
    // Header
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("Maze Generator");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Separator();
    ImGui::Spacing();
    
    // Form
    const char* shapes[] = { "Rectangular" };
    ImGui::Combo("Shape", &shapeIndex, shapes, IM_ARRAYSIZE(shapes));
    
    const char* styles[] = { "Orthogonal", "Hexagonal (TODO)" };
    ImGui::Combo("Style", &styleIndex, styles, IM_ARRAYSIZE(styles));
    
    ImGui::InputInt("Width", &width);
    if (width < 2) width = 2;
    if (width > 200) width = 200;
    
    ImGui::InputInt("Height", &height);
    if (height < 2) height = 2;
    if (height > 200) height = 200;
    
    ImGui::InputInt("Inner Width", &innerWidth);
    ImGui::InputInt("Inner Height", &innerHeight);
    
    const char* starts[] = { "Bottom Center" };
    ImGui::Combo("Starts at", &startIndex, starts, IM_ARRAYSIZE(starts));
    
    ImGui::Separator();
    ImGui::Text("Advanced (E/R)");
    
    ImGui::SliderInt("Elaborateness", &elaborateness, 0, 100);
    ImGui::SliderInt("Remove dead ends", &removeDeadEnds, 0, 100);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Generate new", { -1, 40 })) {
        generator.Generate(width, height);
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Proceed to Programming", { -1, 40 })) {
        proceedToIDE = true;
    }
    
    ImGui::End();
}
