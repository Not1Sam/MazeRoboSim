#pragma once
#include "MazeGenerator.h"
#include "Interpreter.h"
#include "raylib.h"
#include <string>

class Simulation {
public:
    Simulation();
    
    void Init(const MazeGenerator& maze, const std::string& code);
    void Update();
    void Draw();
    
    // Robot State
    struct Robot {
        Vector2 position; // Grid coordinates
        float rotation;   // Degrees (0 = East, 90 = South)
        float speedLeft;
        float speedRight;
    };
    Robot robot;
    
    float frontDist;
    float leftDist;
    float rightDist;
    
    // Config
    float stepDelay = 1.0f; // Seconds per step
    
private:
    const MazeGenerator* currentMaze;
    std::string currentCode;
    Interpreter interpreter;
    
    float executionTimer = 0.0f;
    
    void UpdatePhysics();
    void ExecuteCode();
    void ReadPins();
    float CastRay(Vector2 start, Vector2 dir);
};
