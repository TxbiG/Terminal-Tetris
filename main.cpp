#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <conio.h> // For _kbhit() and _getch()
#endif

// Todo
// Fix Visual Rotation
// Some times Collision doesnt work

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;

// Tetromino shapes defined as 4x4 grids
vector<vector<vector<int>>> TETROMINOES = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 1}, {1, 1}},                                         // O
    {{0, 1, 0}, {1, 1, 1}, {0, 0, 0}},                       // T
    {{1, 1, 0}, {0, 1, 1}, {0, 0, 0}},                       // Z
    {{0, 1, 1}, {1, 1, 0}, {0, 0, 0}},                       // S
    {{1, 0, 0}, {1, 1, 1}, {0, 0, 0}},                       // L
    {{0, 0, 1}, {1, 1, 1}, {0, 0, 0}}                        // J
};

// Game board
vector<vector<int>> board(HEIGHT, vector<int>(WIDTH, 0));

int currentPiece = 0; // Index of the current piece
int pieceX = WIDTH / 2 - 1, pieceY = 0, rotation = 0; // Current piece position and rotation
bool gameOver = false;

int fallCounter = 0;       // Counter to slow down piece falling
const int fallSpeed = 10;  // How many iterations before the piece moves down

// Function to clear the terminal screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Function to rotate a shape (generic rotation logic)
vector<vector<int>> rotateShape(vector<vector<int>> shape) {
    vector<vector<int>> rotatedShape(shape[0].size(), vector<int>(shape.size()));

    // Transpose and reverse each row
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            rotatedShape[j][shape.size() - 1 - i] = shape[i][j]; // Transpose and reverse
        }
    }
    return rotatedShape;
}

// Check collision for the current piece with its current rotation
bool isCollision(int x, int y, int r) {
    const auto& shape = TETROMINOES[currentPiece];
    vector<vector<int>> rotatedShape = shape;

    // Apply the rotation to the shape based on the current rotation state
    if (r == 1) {
        rotatedShape = rotateShape(shape); // 90 degrees
    } else if (r == 2) {
        rotatedShape = rotateShape(rotateShape(shape)); // 180 degrees
    } else if (r == 3) {
        rotatedShape = rotateShape(rotateShape(rotateShape(shape))); // 270 degrees
    }

    // Check if the rotated shape collides with the board
    for (int i = 0; i < rotatedShape.size(); i++) {
        for (int j = 0; j < rotatedShape[i].size(); j++) {
            if (rotatedShape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT || (newY >= 0 && board[newY][newX])) {
                    return true; // Collision detected
                }
            }
        }
    }
    return false;
}

// Rotate the current piece 90 degrees clockwise
void rotatePiece() {
    // Save the original rotation state
    int originalRotation = rotation;

    // Increment the rotation value (0-3)
    rotation = (rotation + 1) % 4;

    // Get the current piece shape
    vector<vector<int>> newShape = TETROMINOES[currentPiece];

    // Create a new rotated shape matrix
    vector<vector<int>> rotatedShape(newShape[0].size(), vector<int>(newShape.size()));

    // Transpose and reverse each row (rotate)
    for (int i = 0; i < newShape.size(); i++) {
        for (int j = 0; j < newShape[i].size(); j++) {
            rotatedShape[j][newShape.size() - 1 - i] = newShape[i][j]; // Transpose and reverse
        }
    }

    // Check if the rotated piece collides with the board
    if (!isCollision(pieceX, pieceY, rotation)) {
        TETROMINOES[currentPiece] = rotatedShape; // Apply the rotation if it's valid
    } else {
        // Revert to the original rotation if there's a collision
        rotation = originalRotation;
    }
}

// Render the game board to the terminal
void render() {
    clearScreen();

    // Temporary copy of the board to overlay the active piece
    vector<vector<int>> tempBoard = board;

    // Get the current piece shape
    const auto& shape = TETROMINOES[currentPiece]; // Use current piece

    // Apply the rotation on the fly based on the rotation state (0-3)
    vector<vector<int>> rotatedShape = shape;

    // Perform the 90-degree rotation (transpose and reverse rows) based on rotation state
    if (rotation == 1) {  // 90 degrees rotation
        rotatedShape = rotateShape(shape);
    } else if (rotation == 2) {  // 180 degrees rotation
        rotatedShape = rotateShape(rotateShape(shape)); // Rotate 90 twice
    } else if (rotation == 3) {  // 270 degrees rotation
        rotatedShape = rotateShape(rotateShape(rotateShape(shape))); // Rotate 90 three times
    }

    // Overlay the rotated piece on the temporary board
    for (int i = 0; i < rotatedShape.size(); i++) {
        for (int j = 0; j < rotatedShape[i].size(); j++) {
            if (rotatedShape[i][j]) {
                int x = pieceX + j;
                int y = pieceY + i;
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                    tempBoard[y][x] = currentPiece + 1; // Temporarily add the active piece
                }
            }
        }
    }

    // Draw the temporary board with the active piece
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (tempBoard[y][x] != 0) {
                cout << "[]"; // Filled block
            } else {
                cout << " ."; // Empty space
            }
        }
        cout << endl;
    }

    cout << "Controls: [A] Left  [D] Right  [S] Drop  [W] Rotate  [Q] Quit" << endl;
}

// Lock the piece into the board and preserve its rotation state
void lockPiece() {
    const auto& shape = TETROMINOES[currentPiece];
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            if (shape[i][j]) {
                board[pieceY + i][pieceX + j] = currentPiece + 1;  // Place the block on the board
            }
        }
    }
}

// Clear full rows
void clearRows() {
    for (int y = HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < WIDTH; x++) {
            if (board[y][x] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            for (int ny = y; ny > 0; ny--) {
                board[ny] = board[ny - 1]; // Shift rows down
            }
            board[0] = vector<int>(WIDTH, 0); // Clear the top row
            y++; // Check the same row again
        }
    }
}

// Spawn a new piece
void spawnPiece() {
    currentPiece = rand() % TETROMINOES.size();
    pieceX = WIDTH / 2 - 1;
    pieceY = 0;
    rotation = 0;

    if (isCollision(pieceX, pieceY, rotation)) {
        gameOver = true; // Game over if new piece cannot spawn
    }
}

// Update the game state
void update() {
    fallCounter++;

    // Only move the piece down every `fallSpeed` updates
    if (fallCounter >= fallSpeed) {
        pieceY++;       // Move the piece down
        fallCounter = 0; // Reset the counter
    }

    // Check for collision after moving the piece down
    if (isCollision(pieceX, pieceY, rotation)) {
        pieceY--; // Undo the move if there's a collision
        lockPiece(); // Lock the piece into the board
        clearRows(); // Clear completed rows
        spawnPiece(); // Spawn a new piece
    }
}

// Handle player input
void handleInput() {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == 'a' && !isCollision(pieceX - 1, pieceY, rotation)) pieceX--; // Move left
        if (ch == 'd' && !isCollision(pieceX + 1, pieceY, rotation)) pieceX++; // Move right
        if (ch == 's') {
            while (!isCollision(pieceX, pieceY + 1, rotation)) {
                pieceY++; // Drop the piece
            }
            lockPiece();
            clearRows();
            spawnPiece();
        }
        if (ch == 'w') rotatePiece(); // Rotate the piece when 'W' is pressed
        if (ch == 'q') gameOver = true; // Quit
    }
}

// Main function
int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed random number generator

    spawnPiece();

    while (!gameOver) {
        render();      // Render the board and active piece
        handleInput(); // Process user input
        update();      // Update the game state
        this_thread::sleep_for(chrono::milliseconds(50)); // Add delay for the loop
    }

    cout << "Game Over! Thanks for playing!" << endl;
    return 0;
}
