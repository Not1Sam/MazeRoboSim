#pragma once
#include "raylib.h"
#include <vector>

struct Cell {
    int x, y;
    bool visited;
    bool wallNorth;
    bool wallSouth;
    bool wallEast;
    bool wallWest;
};

class MazeGenerator {
public:
    int width;
    int height;
    int innerWidth;
    int innerHeight;
    
    std::vector<Cell> grid;

    MazeGenerator();
    
    void Generate(int w, int h);
    void Draw();
    
    // Coordinate Conversion
    Vector2 GetScreenPos(float gridX, float gridY) const;
    float GetRenderCellSize() const { return renderCellSize; }
    
    // Data Access
    const Cell* GetCell(int x, int y) const;

private:
    int GetIndex(int x, int y);
    std::vector<int> GetUnvisitedNeighbors(int index);
    void RemoveWalls(int current, int next);
    
    // Render State (Cached in Draw)
    float renderCellSize;
    float renderOffsetX;
    float renderOffsetY;
};
