#pragma once
#include "raylib.h"
#include "MazeGenerator.h"
#include "Interpreter.h"
#include <string>
#include <vector>

struct Robot {
    Vector2 position;
    float rotation; // Degrees
    float speedLeft;
    float speedRight;
    
    // Sensors
    float distFront;
    float distLeft;
    float distRight;
};

class Simulation {
public:
    Simulation();
    
    void Init(const MazeGenerator& maze, const std::string& code);
    void Update();
    void Draw();

private:
    const MazeGenerator* currentMaze;
    std::string currentCode;
    Robot robot;
    Interpreter interpreter;
    
    // Sensor Data
    float frontDist = 0;
    float leftDist = 0;
    float rightDist = 0;
    
    // Interpreter State
    void ExecuteCode();
    
    // Physics
    void UpdatePhysics();
    float CastRay(Vector2 start, Vector2 dir);
};
