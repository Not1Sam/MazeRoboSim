#include "Simulation.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <sstream>

Simulation::Simulation() {
    currentMaze = nullptr;
}

void Simulation::Init(const MazeGenerator& maze, const std::string& code) {
    interpreter.Stop(); // Stop existing thread if any
    
    currentMaze = &maze;
    currentCode = code;
    
    // Reset Robot at Bottom Center (Grid Coordinates)
    int startX = maze.width / 2;
    int startY = maze.height - 1;
    
    robot.position = {
        (float)startX + 0.5f,
        (float)startY + 0.5f
    };
    robot.rotation = -90.0f; // Facing Up
    robot.speedLeft = 0;
    robot.speedRight = 0;
    
    interpreter.Load(code);
    interpreter.Start(); // Start the thread
}

void Simulation::Update() {
    if (!currentMaze) return;
    
    UpdatePhysics();
    // ExecuteCode is now running in a thread.
    // We just read the pins in UpdatePhysics (or separate function).
    ReadPins();
}

void Simulation::ReadPins() {
    // Map Pins to Motors
    // User Code:
    // IN1 = 8, IN2 = 9 (Left Motor?)
    // IN3 = 10, IN4 = 11 (Right Motor?)
    // ENA = 12, ENB = 13
    
    int in1 = interpreter.GetPinValue(8);
    int in2 = interpreter.GetPinValue(9);
    int in3 = interpreter.GetPinValue(10);
    int in4 = interpreter.GetPinValue(11);
    
    // Logic:
    // Forward: IN1 High, IN2 Low
    // Backward: IN1 Low, IN2 High
    // Stop: Low, Low
    
    float leftSpeed = 0.0f;
    if (in1 && !in2) leftSpeed = 1.0f;
    else if (!in1 && in2) leftSpeed = -1.0f;
    
    float rightSpeed = 0.0f;
    if (in3 && !in4) rightSpeed = 1.0f;
    else if (!in3 && in4) rightSpeed = -1.0f;
    
    robot.speedLeft = leftSpeed;
    robot.speedRight = rightSpeed;
}

void Simulation::UpdatePhysics() {
    float dt = GetFrameTime();
    
    // Movement
    float speed = (robot.speedLeft + robot.speedRight) / 2.0f;
    float rotSpeed = (robot.speedRight - robot.speedLeft) / 2.0f; // Differential steering
    
    // Adjust speeds for simulation feel
    float moveSpeedScale = 2.0f; // Cells per second
    float rotSpeedScale = 180.0f; // Degrees per second
    
    robot.rotation += rotSpeed * rotSpeedScale * dt;
    
    Vector2 forward = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    Vector2 move = Vector2Scale(forward, speed * moveSpeedScale * dt);
    
    // Proposed new position
    Vector2 nextPos = Vector2Add(robot.position, move);
    
    // Collision Detection
    float radius = 0.3f; 
    
    int cx = (int)nextPos.x;
    int cy = (int)nextPos.y;
    
    const Cell* cell = currentMaze->GetCell(cx, cy);
    bool collision = false;
    
    if (cell) {
        float rx = nextPos.x - cx;
        float ry = nextPos.y - cy;
        
        if (cell->wallNorth && ry < radius) collision = true;
        if (cell->wallSouth && ry > (1.0f - radius)) collision = true;
        if (cell->wallWest && rx < radius) collision = true;
        if (cell->wallEast && rx > (1.0f - radius)) collision = true;
    } else {
        collision = true;
    }
    
    if (!collision) {
        robot.position = nextPos;
    }
    
    // Sensors (Raycast)
    float unitsPerCell = 40.0f; // cm per cell?
    
    Vector2 forwardDir = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    Vector2 leftDir = { cosf(DEG2RAD * (robot.rotation - 90)), sinf(DEG2RAD * (robot.rotation - 90)) };
    Vector2 rightDir = { cosf(DEG2RAD * (robot.rotation + 90)), sinf(DEG2RAD * (robot.rotation + 90)) };
    
    float frontDistVal = CastRay(robot.position, forwardDir) * unitsPerCell;
    float leftDistVal = CastRay(robot.position, leftDir) * unitsPerCell;
    float rightDistVal = CastRay(robot.position, rightDir) * unitsPerCell;
    
    frontDist = frontDistVal;
    leftDist = leftDistVal;
    rightDist = rightDistVal;
    
    // Inject variables
    // User code uses: distF, distL, distR (globals)
    // But it also calls readSensors() which calls readUltrasonic().
    // readUltrasonic calls pulseIn.
    // pulseIn reads sensorValues.
    
    // We need to set sensorValues for pulseIn to work.
    // User code:
    // trigF=2, echoF=3
    // trigL=4, echoL=5
    // trigR=6, echoR=7
    
    interpreter.SetSensorValue(2, 3, frontDist);
    interpreter.SetSensorValue(4, 5, leftDist);
    interpreter.SetSensorValue(6, 7, rightDist);
    
    // Also set globals if user code uses them directly (optional, but user code reads them via readSensors)
    // The user code: distF = readUltrasonic(...)
    // So we don't need to set distF directly.
}

float Simulation::CastRay(Vector2 start, Vector2 dir) {
    float dist = 0.0f;
    float step = 0.05f; 
    float maxDist = 5.0f; 
    
    Vector2 currentPos = start;
    
    while (dist < maxDist) {
        Vector2 nextPos = Vector2Add(currentPos, Vector2Scale(dir, step));
        
        int cx = (int)currentPos.x;
        int cy = (int)currentPos.y;
        int nx = (int)nextPos.x;
        int ny = (int)nextPos.y;
        
        const Cell* cell = currentMaze->GetCell(cx, cy);
        if (!cell) return dist; 
        
        if (cx != nx) {
            if (nx > cx) { if (cell->wallEast) return dist; }
            else { if (cell->wallWest) return dist; }
        }
        
        if (cy != ny) {
            if (ny > cy) { if (cell->wallSouth) return dist; }
            else { if (cell->wallNorth) return dist; }
        }
        
        if (!currentMaze->GetCell(nx, ny)) return dist;
        
        currentPos = nextPos;
        dist += step;
    }
    
    return maxDist;
}

void Simulation::Draw() {
    if (!currentMaze) return;
    
    Vector2 screenPos = currentMaze->GetScreenPos(robot.position.x, robot.position.y);
    float cellSize = currentMaze->GetRenderCellSize();
    float robotSize = cellSize * 0.3f; 
    
    DrawCircleV(screenPos, robotSize, RED);
    
    Vector2 forward = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    DrawLineV(screenPos, Vector2Add(screenPos, Vector2Scale(forward, robotSize * 1.5f)), BLACK);
    
    int screenW = GetScreenWidth();
    DrawText("Sensor Values:", screenW - 200, 20, 20, BLACK);
    DrawText(TextFormat("Front: %.1f", frontDist), screenW - 200, 50, 20, BLUE);
    DrawText(TextFormat("Left:  %.1f", leftDist), screenW - 200, 80, 20, BLUE);
    DrawText(TextFormat("Right: %.1f", rightDist), screenW - 200, 110, 20, BLUE);
}

void Simulation::ExecuteCode() {
    // Deprecated/Unused in threaded mode
}
