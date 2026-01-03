#include "MazeGenerator.h"
#include <stack>
#include <cstdlib>
#include <ctime>

MazeGenerator::MazeGenerator() {
    width = 20;
    height = 20;
    innerWidth = 0;
    innerHeight = 0;
    renderCellSize = 20.0f;
    renderOffsetX = 400.0f;
    renderOffsetY = 50.0f;
    Generate(width, height);
}

int MazeGenerator::GetIndex(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return -1;
    return y * width + x;
}

void MazeGenerator::Generate(int w, int h) {
    width = w;
    height = h;
    grid.clear();
    grid.resize(width * height);
    
    // Initialize grid
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Cell& cell = grid[GetIndex(x, y)];
            cell.x = x;
            cell.y = y;
            cell.visited = false;
            cell.wallNorth = true;
            cell.wallSouth = true;
            cell.wallEast = true;
            cell.wallWest = true;
        }
    }
    
    // Recursive Backtracker
    std::stack<int> stack;
    int startIdx = GetIndex(0, 0); // Start top-left for generation logic
    grid[startIdx].visited = true;
    stack.push(startIdx);
    
    srand(time(NULL));
    
    while (!stack.empty()) {
        int current = stack.top();
        std::vector<int> neighbors = GetUnvisitedNeighbors(current);
        
        if (!neighbors.empty()) {
            int next = neighbors[rand() % neighbors.size()];
            RemoveWalls(current, next);
            grid[next].visited = true;
            stack.push(next);
        } else {
            stack.pop();
        }
    }
    
    // Create Entrance (Bottom Center)
    int entranceX = width / 2;
    int entranceIdx = GetIndex(entranceX, height - 1);
    if (entranceIdx != -1) {
        grid[entranceIdx].wallSouth = false;
    }
    
    // Create Exit (Top Center)
    int exitX = width / 2;
    int exitIdx = GetIndex(exitX, 0);
    if (exitIdx != -1) {
        grid[exitIdx].wallNorth = false;
    }
}

std::vector<int> MazeGenerator::GetUnvisitedNeighbors(int index) {
    std::vector<int> neighbors;
    int x = grid[index].x;
    int y = grid[index].y;
    
    int n = GetIndex(x, y - 1);
    int s = GetIndex(x, y + 1);
    int e = GetIndex(x + 1, y);
    int w = GetIndex(x - 1, y);
    
    if (n != -1 && !grid[n].visited) neighbors.push_back(n);
    if (s != -1 && !grid[s].visited) neighbors.push_back(s);
    if (e != -1 && !grid[e].visited) neighbors.push_back(e);
    if (w != -1 && !grid[w].visited) neighbors.push_back(w);
    
    return neighbors;
}

void MazeGenerator::RemoveWalls(int current, int next) {
    int x1 = grid[current].x;
    int y1 = grid[current].y;
    int x2 = grid[next].x;
    int y2 = grid[next].y;
    
    if (x1 == x2) {
        if (y1 > y2) { // Top
            grid[current].wallNorth = false;
            grid[next].wallSouth = false;
        } else { // Bottom
            grid[current].wallSouth = false;
            grid[next].wallNorth = false;
        }
    } else {
        if (x1 > x2) { // Left
            grid[current].wallWest = false;
            grid[next].wallEast = false;
        } else { // Right
            grid[current].wallEast = false;
            grid[next].wallWest = false;
        }
    }
}

void MazeGenerator::Draw() {
    float cellSize = 20.0f;
    float offsetX = 400.0f; // Offset for UI
    float offsetY = 50.0f;
    
    // Auto-scale
    float availWidth = GetScreenWidth() - offsetX - 50;
    float availHeight = GetScreenHeight() - 100;
    float scaleX = availWidth / (width * cellSize);
    float scaleY = availHeight / (height * cellSize);
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    if (scale < 1.0f) cellSize *= scale;

    // Cache for Simulation
    renderCellSize = cellSize;
    renderOffsetX = offsetX;
    renderOffsetY = offsetY;

    for (const auto& cell : grid) {
        float x = offsetX + cell.x * cellSize;
        float y = offsetY + cell.y * cellSize;
        
        if (cell.wallNorth) DrawLineEx({x, y}, {x + cellSize, y}, 2.0f, BLACK);
        if (cell.wallSouth) DrawLineEx({x, y + cellSize}, {x + cellSize, y + cellSize}, 2.0f, BLACK);
        if (cell.wallEast) DrawLineEx({x + cellSize, y}, {x + cellSize, y + cellSize}, 2.0f, BLACK);
        if (cell.wallWest) DrawLineEx({x, y}, {x, y + cellSize}, 2.0f, BLACK);
    }
}

Vector2 MazeGenerator::GetScreenPos(float gridX, float gridY) const {
    return {
        renderOffsetX + gridX * renderCellSize,
        renderOffsetY + gridY * renderCellSize
    };
}

const Cell* MazeGenerator::GetCell(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;
    return &grid[y * width + x];
}
