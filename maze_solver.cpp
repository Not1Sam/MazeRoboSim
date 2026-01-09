// Maze Solver using DFS, Piles, and Backtracking

// Pins
const int trigF = 2; const int echoF = 3;
const int trigL = 4; const int echoL = 5;
const int trigR = 6; const int echoR = 7;

// Constants
const int WALL_DIST = 20;
const int MOVE_TIME = 600; // Time to move one cell
const int TURN_TIME = 400; // Time to turn 90 deg

// Globals
int distF; int distL; int distR;
pile pathStack; // Tracks the path: 1=F, 2=R, 3=L

// Sensor Function
int readDistance(int trig, int echo) {
    return pulseIn(echo, 1, 30000) * 0.034 / 2;
}

void readSensors() {
    distF = readDistance(trigF, echoF);
    distL = readDistance(trigL, echoL);
    distR = readDistance(trigR, echoR);
}

// Movement Wrappers
void moveOneCell() {
    forward();
    delay(MOVE_TIME);
    stop();
    delay(200); // Stabilize
}

void moveBackOneCell() {
    backward();
    delay(MOVE_TIME);
    stop();
    delay(200);
}

void turnLeft90() {
    left(); // Built-in 90 deg turn
    delay(200);
}

void turnRight90() {
    right(); // Built-in 90 deg turn
    delay(200);
}

// Recursive DFS Solver
bool solveMaze() {
    readSensors();
    
    // Check for Goal (e.g., very large open space or specific condition)
    // For this maze, let's assume if we find a long corridor > 100cm it might be an exit
    if (distF > 100) return true;

    // Try Right
    if (distR > WALL_DIST) {
        turnRight90();
        moveOneCell();
        push(pathStack, 2); // 2 = Right
        
        if (solveMaze()) return true;
        
        // Backtrack
        pop(pathStack);
        moveBackOneCell();
        turnLeft90(); // Undo Right turn
        readSensors(); // Update sensors after move
    }

    // Try Forward
    if (distF > WALL_DIST) {
        moveOneCell();
        push(pathStack, 1); // 1 = Forward
        
        if (solveMaze()) return true;
        
        // Backtrack
        pop(pathStack);
        moveBackOneCell();
        readSensors();
    }

    // Try Left
    if (distL > WALL_DIST) {
        turnLeft90();
        moveOneCell();
        push(pathStack, 3); // 3 = Left
        
        if (solveMaze()) return true;
        
        // Backtrack
        pop(pathStack);
        moveBackOneCell();
        turnRight90(); // Undo Left turn
        readSensors();
    }

    // Dead End
    return false;
}

void setup() {
    // Optional setup
}

void loop() {
    // Start solving
    if (solveMaze()) {
        // Goal Found!
        stop();
        while(true); // Stop forever
    } else {
        // Maze explored, no exit found (returned to start)
        stop();
        while(true);
    }
}
