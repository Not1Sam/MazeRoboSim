#pragma once
#include "raylib.h"
#include "MazeGenerator.h"
#include <string>

class IDE {
public:
    std::string code;
    
    IDE();
    void Draw(MazeGenerator& maze, class Simulation& simulation);
    
    bool ShouldGoBack() { return goBack; }
    void Reset() { goBack = false; }
    
private:
    bool goBack;
    void AutoFormat();
    void DrawRobotPreview(int x, int y, int w, int h);
    void DrawMazePreview(MazeGenerator& maze, int x, int y, int w, int h);
};
