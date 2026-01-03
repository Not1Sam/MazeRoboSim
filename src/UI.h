#pragma once
#include "MazeGenerator.h"

class UI {
public:
    UI(MazeGenerator& gen);
    void Draw();
    bool ShouldProceed() { return proceedToIDE; }
    void Reset() { proceedToIDE = false; }

private:
    MazeGenerator& generator;
    
    // Form State
    int width;
    int height;
    int innerWidth;
    int innerHeight;
    int shapeIndex;
    int styleIndex;
    int startIndex;
    int elaborateness;
    int removeDeadEnds;
    
    // Callbacks
    bool proceedToIDE;
};
