#include "Simulation.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>
#include <sstream>

Simulation::Simulation() {
    currentMaze = nullptr;
}

void Simulation::Init(const MazeGenerator& maze, const std::string& code) {
    currentMaze = &maze;
    currentCode = code;
    
    // Spawn Robot at Bottom Center (Grid Coordinates)
    // x: 0 to width, y: 0 to height
    // Center of cell (x, y) is (x + 0.5, y + 0.5)
    
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
}

void Simulation::Update() {
    if (!currentMaze) return;
    
    UpdatePhysics();
    ExecuteCode();
}

void Simulation::UpdatePhysics() {
    float dt = GetFrameTime();
    
    // Movement
    float speed = (robot.speedLeft + robot.speedRight) / 2.0f;
    float rotSpeed = (robot.speedLeft - robot.speedRight) / 2.0f; // Increased sensitivity
    
    robot.rotation += rotSpeed * dt * 180.0f; // Faster rotation (degrees per sec)
    
    Vector2 forward = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    // Speed: Cells per second (e.g., 2.0f)
    Vector2 move = Vector2Scale(forward, speed * dt * 2.0f);
    
    // Proposed new position
    Vector2 nextPos = Vector2Add(robot.position, move);
    
    // Collision Detection
    // Robot Radius in Grid Units (0.3f gives a bit of clearance, cell is 1.0f)
    float radius = 0.3f; 
    
    int cx = (int)nextPos.x;
    int cy = (int)nextPos.y;
    
    const Cell* cell = currentMaze->GetCell(cx, cy);
    bool collision = false;
    
    if (cell) {
        // Check walls of the current cell
        // Relative position in cell (0.0 to 1.0)
        float rx = nextPos.x - cx;
        float ry = nextPos.y - cy;
        
        // North Wall (y = 0)
        if (cell->wallNorth && ry < radius) collision = true;
        // South Wall (y = 1)
        if (cell->wallSouth && ry > (1.0f - radius)) collision = true;
        // West Wall (x = 0)
        if (cell->wallWest && rx < radius) collision = true;
        // East Wall (x = 1)
        if (cell->wallEast && rx > (1.0f - radius)) collision = true;
        
        // Corner checks (Simple approximation: if close to corner and both walls exist)
        // Ideally we check distance to corner points, but wall checks cover most cases
        // except the "inner corner" of a turn.
        // For now, this simple wall check prevents passing through.
    } else {
        // Out of bounds
        collision = true;
    }
    
    if (!collision) {
        robot.position = nextPos;
    } else {
        // Simple response: Stop or Slide?
        // For now, just stop movement in that direction (Slide along wall would be better but harder)
        // Let's try to allow rotation even if stuck
    }
    
    // Sensors (Raycast)
    // Front: Trig 2, Echo 3
    // Left: Trig 4, Echo 7
    // Right: Trig 8, Echo 11
    
    // TODO: Real Raycast
    // For now, mock values based on simple proximity to walls?
    // Let's just set them to "Clear" (100) or "Blocked" (10) based on collision check?
    // Better: Implement a simple Raycast in MazeGenerator or here.
    
    // Sensors (Raycast)
    // Units: Let's say 1 Cell = 40.0 units (cm)
    float unitsPerCell = 40.0f;
    
    Vector2 forwardDir = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    Vector2 leftDir = { cosf(DEG2RAD * (robot.rotation - 90)), sinf(DEG2RAD * (robot.rotation - 90)) };
    Vector2 rightDir = { cosf(DEG2RAD * (robot.rotation + 90)), sinf(DEG2RAD * (robot.rotation + 90)) };
    
    float frontDistVal = CastRay(robot.position, forwardDir) * unitsPerCell;
    float leftDistVal = CastRay(robot.position, leftDir) * unitsPerCell;
    float rightDistVal = CastRay(robot.position, rightDir) * unitsPerCell;
    
    // Store for UI
    frontDist = frontDistVal;
    leftDist = leftDistVal;
    rightDist = rightDistVal;
    
    // Inject variables
    interpreter.SetVariable("fdist", frontDist);
    interpreter.SetVariable("ldist", leftDist);
    interpreter.SetVariable("rdist", rightDist);
}

float Simulation::CastRay(Vector2 start, Vector2 dir) {
    float dist = 0.0f;
    float step = 0.05f; // Small step for accuracy
    float maxDist = 5.0f; // Max 5 cells range
    
    Vector2 currentPos = start;
    
    while (dist < maxDist) {
        Vector2 nextPos = Vector2Add(currentPos, Vector2Scale(dir, step));
        
        // Check for wall crossing
        int cx = (int)currentPos.x;
        int cy = (int)currentPos.y;
        int nx = (int)nextPos.x;
        int ny = (int)nextPos.y;
        
        const Cell* cell = currentMaze->GetCell(cx, cy);
        if (!cell) return dist; // Out of bounds is a hit (or infinite? let's say hit)
        
        // If we changed cell, check the wall between them
        if (cx != nx) {
            if (nx > cx) { // Moving East
                if (cell->wallEast) return dist;
            } else { // Moving West
                if (cell->wallWest) return dist;
            }
        }
        
        if (cy != ny) {
            if (ny > cy) { // Moving South
                if (cell->wallSouth) return dist;
            } else { // Moving North
                if (cell->wallNorth) return dist;
            }
        }
        
        // Also check if we entered a null cell (out of bounds)
        if (!currentMaze->GetCell(nx, ny)) return dist;
        
        currentPos = nextPos;
        dist += step;
    }
    
    return maxDist;
}

void Simulation::ExecuteCode() {
    // Reset Command Pin
    interpreter.SetPinValue(100, 0);
    
    interpreter.Step();
    
    // Check Command Pin
    int cmd = interpreter.GetPinValue(100);
    if (cmd == 1 || cmd == 2) {
        // Snap to grid center first
        robot.position.x = floor(robot.position.x) + 0.5f;
        robot.position.y = floor(robot.position.y) + 0.5f;
        
        if (cmd == 1) robot.rotation -= 90.0f; // Left
        if (cmd == 2) robot.rotation += 90.0f; // Right
        
        // Move Forward 1 Cell (if clear)
        Vector2 fwd = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
        
        // Simple check: Is there a wall?
        // We can use CastRay. If distance > 0.6 (just more than half cell), it's safe.
        // Actually, let's just check the cell.
        float dist = CastRay(robot.position, fwd);
        if (dist >= 1.0f) {
            robot.position = Vector2Add(robot.position, fwd);
        }
    }
    
    // Map Pins to Motors (Only if no snap turn command?)
    // If we snapped, we probably want to stop motors for this frame or keep them?
    // Let's keep them, but the snap happens instantly.
    
    // Left: 5 (Fwd), 6 (Bwd)
    int lFwd = interpreter.GetPinValue(5);
    int lBwd = interpreter.GetPinValue(6);
    // Right: 9 (Fwd), 10 (Bwd)
    int rFwd = interpreter.GetPinValue(9);
    int rBwd = interpreter.GetPinValue(10);
    
    robot.speedLeft = (lFwd - lBwd) / 255.0f;
    robot.speedRight = (rFwd - rBwd) / 255.0f;
}

void Simulation::Draw() {
    if (!currentMaze) return;
    
    // Convert Grid Pos to Screen Pos
    Vector2 screenPos = currentMaze->GetScreenPos(robot.position.x, robot.position.y);
    float cellSize = currentMaze->GetRenderCellSize();
    float robotSize = cellSize * 0.3f; // Robot radius (0.3 matches physics radius of 0.3)
    
    DrawCircleV(screenPos, robotSize, RED);
    
    // Draw Direction Indicator
    Vector2 forward = { cosf(DEG2RAD * robot.rotation), sinf(DEG2RAD * robot.rotation) };
    DrawLineV(screenPos, Vector2Add(screenPos, Vector2Scale(forward, robotSize * 1.5f)), BLACK);
    
    // Draw Sensor Values (Top Right)
    int screenW = GetScreenWidth();
    DrawText("Sensor Values:", screenW - 200, 20, 20, BLACK);
    DrawText(TextFormat("Front: %.1f", frontDist), screenW - 200, 50, 20, BLUE);
    DrawText(TextFormat("Left:  %.1f", leftDist), screenW - 200, 80, 20, BLUE);
    DrawText(TextFormat("Right: %.1f", rightDist), screenW - 200, 110, 20, BLUE);
}
