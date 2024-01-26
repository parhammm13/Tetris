#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>


using namespace std;


int BoardWidth = 5;
int BoardHeight = 5;
const char BoardChar = '#';
//const char BlockChar = '*';
// Game board represented as 2D array
//int board[BoardHeight][BoardWidth] = {};
bool isPaused = false;
bool Auto = true;
int blockType = 1;


struct PlayerScore {
    string name;
    int score;
    double playTime; // Time taken in seconds
    time_t timestamp;
};

const int colors[7] = {
        4, // Red
        2, // Green
        6, // Yellow
        1, // Blue
        5, // Magenta
        3, // Cyan
        7  // White
};

const char* resetColor = "\x1B[0m";


// Game state variablesaaa
int currentX = BoardWidth / 2, currentY = 0; // Initial position of the block
int currentBlock[4][4]; // Current block shape
int score = 0;
bool gameOver = false;
//Storing the next block
int nextBlock[4][4];
int currentBlockColor;
int nextBlockColor;
HANDLE pipe;
string pipeName = "\\\\.\\pipe\\tetrispipe";



bool CheckRotationCollision();
void RotateBlock();
void SetConsoleColor(int textColor, int bgColor);
void Input();
void InitializeCurrentBlock();
void MergeBlockIntoBoard();
bool CheckCollision(int dx, int dy, int tempBlock[4][4]);
void Update();
void Render();
void GenerateNextBlock();
bool scoreCompare(const PlayerScore &a, const PlayerScore &b);
void updateLeaderboard(const string &filename, const PlayerScore &player);
void showLeaderboard(const string &filename);
void showHowToPlay();
void Input2();
void InitializePipe();
void Updatemulti(int &lastDropTime);
void Input3();

struct Cell {
    bool isFilled;
    int color;
};

Cell **board; // Declare as a global variable



const int Blocks[7][4][4] = {
        // Square block
        {
                {0, 0, 0, 0},
                {0, 1, 1, 0},
                {0, 1, 1, 0},
                {0, 0, 0, 0}
        },
        // Line block
        {
                {0, 0, 0, 0},
                {1, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },


        {
                {0, 1, 0, 0},
                {0, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },

        {
                {0, 0, 1, 0},
                {1, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },

        {
                {0, 0, 1, 0},
                {0, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },
        {
                {0, 0, 1, 1},
                {0, 1, 1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },
        {
                {1, 1, 0, 0},
                {0, 1,1, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
        },


        // Add more shapes here...
};
void showHowToPlay() {
    SetConsoleColor(3, 0); // Cyan text color
    cout << "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n";
    cout << "▓      HOW TO PLAY TETRIS    ▓\n";
    cout << "▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n";
    SetConsoleColor(7, 0); // Reset to default color

    SetConsoleColor(2, 0); // Green text color
    cout << "Controls:\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout << "  Move Left: 'a'\n";
    cout << "  Move Right: 'd'\n";
    cout << "  Move Down: 's'\n";
    cout << "  Rotate: 'w'\n";
    cout << "  Pause: 'p'\n\n";

    SetConsoleColor(6, 0); // Yellow text color
    cout << "Objective:\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout << "  Form complete rows with blocks.\n";
    cout << "  Rows disappear, granting points.\n";
    cout << "  Game ends when blocks reach the top.\n\n";

    SetConsoleColor(5, 0); // Magenta text color
    cout << "Tips:\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout << "  Plan moves to avoid running out of space.\n";
    cout << "  Clear multiple lines at once for more points.\n";
    cout << "  Adapt to increasing speed.\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout <<    "\n";
    SetConsoleColor(4, 0); // Magenta text color
    cout <<  "Difficulty Level 1 (Easy Mode):\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout <<     "\n";
    cout <<         "    -When the player selects difficulty level 1, the game is set to a slower pace.\n";
    cout <<    "-This is implemented by setting the delay variable to 500 milliseconds.\n";
    cout <<    "-A higher delay value means that the blocks fall slower, giving the player more time to think and react.\n";
    cout <<   "-This mode is more suitable for beginners or players who prefer a more relaxed gameplay experience.\n";
    SetConsoleColor(4, 0); // Magenta text color
    cout <<    "\n";
    cout <<    "Difficulty Level 2 (Hard Mode):\n";
    SetConsoleColor(7, 0); // Reset to default color
    cout <<    "\n";
    cout <<   "    -Choosing difficulty level 2 sets the game to a faster pace.\n";
    cout <<   "-The delay is set to 250 milliseconds, half of what it is in level 1.\n";
    cout <<   "-A lower delay value results in blocks falling faster, requiring quicker decision-making and reflexes.\n";
    cout <<   "-This mode offers a more challenging experience and is better suited for players who are more familiar with Tetris or seeking a more intense game.";
}




bool scoreCompare(const PlayerScore &a, const PlayerScore &b) {
    if (a.score != b.score) {
        return a.score > b.score;
    }
    // In case of tie in score, compare by play time
    return a.playTime < b.playTime;
}

void updateLeaderboard(const string &filename, const PlayerScore &player) {
    vector<PlayerScore> scores;
    ifstream infile(filename);
    PlayerScore temp;

    // Read existing scores
    while (infile >> temp.name >> temp.score >> temp.playTime >> temp.timestamp) {
        scores.push_back(temp);
    }
    infile.close();

    // Update or add new score
    bool scoreUpdated = false;
    for (vector<PlayerScore>::iterator s = scores.begin(); s != scores.end(); ++s) {
        if (s->name == player.name) {
            if (player.score > s->score || (player.score == s->score && player.playTime < s->playTime)) {
                s->score = player.score;
                s->playTime = player.playTime;
                s->timestamp = player.timestamp;
            }
            scoreUpdated = true;
            break;
        }
    }
    if (!scoreUpdated) {
        scores.push_back(player);
    }

    // Sort and write back to file
    sort(scores.begin(), scores.end(), scoreCompare);
    ofstream outfile(filename);
    for (vector<PlayerScore>::const_iterator s = scores.begin(); s != scores.end(); ++s) {
        outfile << s->name << " " << s->score << " " << s->playTime << " " << s->timestamp << endl;
    }
    outfile.close();
}


void showLeaderboard(const string &filename) {
    ifstream infile(filename);
    PlayerScore temp;
    cout << "Leaderboard:\n";
    cout << left << setw(20) << "Name" << setw(10) << "Score" << setw(15) << "Time(s)" << "Date/Time" << endl;

    while (infile >> temp.name >> temp.score >> temp.playTime >> temp.timestamp) {
        cout << left << setw(20) << temp.name << setw(10) << temp.score << setw(15) << temp.playTime;
        cout << ctime(&temp.timestamp);
    }
    infile.close();
}



void InitializeGame() {
    srand(time(NULL)); // Seed the random number generator for block selection

    score = 0;
    gameOver = false;

    // Dynamically allocate the game board
    board = new Cell*[BoardHeight];
    for (int i = 0; i < BoardHeight; i++) {
        board[i] = new Cell[BoardWidth];
    }

    // Clear the game board
    for (int y = 0; y < BoardHeight; y++) {
        for (int x = 0; x < BoardWidth; x++) {
            board[y][x].isFilled = false;
            board[y][x].color = 7; // Default color
        }
    }

    // Generate the first block and set it as the current block
    GenerateNextBlock();
    memcpy(currentBlock, nextBlock, sizeof(currentBlock));
    currentBlockColor = nextBlockColor;

    // Set the initial position of the block
    currentX = BoardWidth / 2 - 2; // Centered, assuming block width of 4
    currentY = 0;

    // Generate the next block for the upcoming preview
    GenerateNextBlock();
}


bool CheckCollision(int dx, int dy, int tempBlock[4][4]) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (tempBlock[y][x]) {
                int newX = currentX + x + dx;
                int newY = currentY + y + dy;
                if (newX < 0 || newX >= BoardWidth || newY >= BoardHeight || board[newY][newX].isFilled) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Define the Tetris blocks. Each block is a 4x4 matrix.
void Input() {
    if (_kbhit()) {
        char ch = _getch();
        switch (ch) {
            case 'a': // Move left
                if (!isPaused && !CheckCollision(-1, 0, currentBlock)) {
                    currentX--;
                }
                break;
            case 'd': // Move right
                if (!isPaused && !CheckCollision(1, 0, currentBlock)) {
                    currentX++;
                }
                break;
            case 's': // Move down
                if (!isPaused && !CheckCollision(0, 1, currentBlock)) {
                    currentY++;
                }
                break;
            case 'w': // Rotate
                if (!isPaused) {
                    RotateBlock();
                }
                break;
            case 'p': // Pause or resume
                isPaused = !isPaused;
                break;
        }
    }
}


void InitializeCurrentBlock() {
    // Choose a random block shape
    int blockType = rand() % 7; // Use the number of defined block shapes

    // Copy the chosen block shape into currentBlock
    memcpy(currentBlock, Blocks[blockType], sizeof(currentBlock));
}

void MergeBlockIntoBoard() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentBlock[y][x] == 1) {
                board[currentY + y][currentX + x].isFilled = true;
                board[currentY + y][currentX + x].color = currentBlockColor; // Assign color
            }
        }
    }
}



void ClearLines() {
    for (int y = 0; y < BoardHeight; y++) {
        bool lineComplete = true;
        for (int x = 0; x < BoardWidth; x++) {
            if (!board[y][x].isFilled) {
                lineComplete = false;
                break;
            }
        }

        if (lineComplete) {
            for (int ty = y; ty > 0; ty--) {
                for (int x = 0; x < BoardWidth; x++) {
                    board[ty][x] = board[ty - 1][x];
                }
            }
            for (int x = 0; x < BoardWidth; x++) {
                board[0][x].isFilled = false;
                board[0][x].color = 7; // Default color
            }
            score += BoardWidth;
        }
    }
}


void GenerateNextBlock() {
    if (Auto == 1){
        blockType = rand() % 7; // Adjust based on the number of block shapes
    }
    nextBlockColor = colors[blockType]; // Assign the color corresponding to the block type
    memcpy(nextBlock, Blocks[blockType], sizeof(nextBlock));
}



void Update() {
    if (isPaused) {
        return; // Skip updating the game logic if the game is paused
    }

    if (!CheckCollision(0, 1, currentBlock)) {
        currentY++;
    } else {
        // The block has collided and should be merged into the board.
        MergeBlockIntoBoard();

        // Check and clear any completed lines.
        ClearLines();

        // Update the current block with the next block
        memcpy(currentBlock, nextBlock, sizeof(currentBlock));
        currentBlockColor = nextBlockColor;

        // Generate the next block for the upcoming preview
        GenerateNextBlock();

        currentX = BoardWidth / 2 - 2; // Center the new block.
        currentY = 0; // Place the new block at the top of the board.

        // If the new block has an immediate collision, the game is over.
        if (CheckCollision(0, 0, currentBlock)) {
            gameOver = true;
        }
    }
}


// Function to set console color
void SetConsoleColor(int textColor, int bgColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
}

const int BlockCharUnicode = 0x25A0;


// Render function
void Render() {
    system("cls"); // Clear the screen

    // Define the colors as console color codes
    const int colors[7] = {
            4, // Red
            2, // Green
            6, // Yellow
            1, // Blue
            5, // Magenta
            3, // Cyan
            7  // White
    };

    SetConsoleColor(7, 0); // Set default color before drawing the top border

    // Draw the top horizontal wall
    for (int x = 0; x < BoardWidth + 2; x++) {
        cout << "▓";
    }
    cout << endl;

    // Draw the board with the current block
    for (int y = 0; y < BoardHeight; y++) {
        SetConsoleColor(7, 0); // Set default color before drawing the left border
        cout << "▓";
        for (int x = 0; x < BoardWidth; x++) {
            bool isPartOfCurrentBlock = x >= currentX && x < currentX + 4 &&
                                        y >= currentY && y < currentY + 4 &&
                                        currentBlock[y - currentY][x - currentX] == 1;

            if (board[y][x].isFilled || isPartOfCurrentBlock) {
                SetConsoleColor(colors[isPartOfCurrentBlock ? currentBlockColor : board[y][x].color], 0);
                cout << static_cast<char>(BlockCharUnicode);
                SetConsoleColor(7, 0); // Reset to default color (white text, black background)
            } else {
                cout << ' '; // Spaces are empty cells and should not have a background color
            }
        }
        SetConsoleColor(7, 0); // Set default color before drawing the right border
        cout << "▓";

        // Draw the next block and score beside the board
        if (y < 4) {
            cout << "  ";
            SetConsoleColor(colors[nextBlockColor], 0); // Set color for next block
            for (int x = 0; x < 4; x++) {
                cout << (nextBlock[y][x] ? static_cast<char>(BlockCharUnicode) : ' ');
            }
            SetConsoleColor(7, 0); // Reset to default color
        } else if (y == 5) {
            cout << "  Score: " << score;
        }

        cout << endl;
    }

    SetConsoleColor(7, 0); // Set default color before drawing the bottom border

    // Draw the bottom horizontal wall
    for (int x = 0; x < BoardWidth + 2; x++) {
        cout << "▓";
    }
    cout << endl;


    SetConsoleColor(7, 0); // Reset to default text color, black background after drawing the bottom border
    if (isPaused) {
        cout << "Game Paused. Press 'p' to resume..." << endl;
    }
}


void RotateBlock() {
    int tempBlock[4][4];
    // Rotate the block into tempBlock
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            tempBlock[y][x] = currentBlock[3-x][y];
        }
    }
    // Check if rotation is possible
    if (!CheckCollision(0, 0, tempBlock)) {
        // Apply rotation
        memcpy(currentBlock, tempBlock, sizeof(tempBlock));
    }
}

bool CheckRotationCollision() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentBlock[y][x] == 1) {
                int newX = currentX + x;
                int newY = currentY + y;
                // Corrected boundary checks and board collision checks.
                if (newX < 0 || newX >= BoardWidth || newY < 0 || newY >= BoardHeight || board[newY][newX].isFilled) {
                    return true;
                }
            }
        }
    }
    return false; // No collision detected.
}



void TetrisGameM(){
    InitializePipe();
    cout << "Enter board width: ";
    cin >> BoardWidth;
    cout << "Enter board height: ";
    cin >> BoardHeight;

    if (BoardWidth < 5 || BoardHeight < 5) {
        cout << "Width and height must be at least 5." << endl;
    }

    int delay, difficultyLevel;
    cout << "Choose difficulty level (1 - Easy, 2 - Hard): ";
    cin >> difficultyLevel;
    delay = (difficultyLevel == 1) ? 500 : 250;

    string playerName;
    cout << "Enter your name: ";
    cin >> playerName;

    InitializeGame();
    time_t startTime = time(nullptr); // Start time

    int lastDropTime = GetTickCount(); // Initialize timer for block drop

    while (!gameOver) {
        Input2(); // Handle user input
        Updatemulti(lastDropTime); // Update game state
        Render();
        Sleep(50); // Lower sleep for more responsive input
    }

    time_t endTime = time(nullptr); // End time
    cout << "Game Over! Score: " << score << endl;

    double playTime = difftime(endTime, startTime); // Calculate play time

    PlayerScore currentPlayer;
    currentPlayer.name = playerName;
    currentPlayer.score = score;
    currentPlayer.playTime = playTime;
    currentPlayer.timestamp = time(nullptr);


    for (int i = 0; i < BoardHeight; ++i) {
        delete[] board[i];
    }
    delete[] board;

    // Clean up the dynamically allocated memory
    for (int i = 0; i < BoardHeight; ++i) {
        delete[] board[i];
    }
    delete[] board;
}


void InitializePipe() {
    pipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            1024 * 16,
            1024 * 16,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);

    if (pipe == INVALID_HANDLE_VALUE) {
        cerr << "Failed to create pipe." << endl;
        exit(1);
    }

    if (!ConnectNamedPipe(pipe, NULL)) {
        if (GetLastError() != ERROR_PIPE_CONNECTED) {
            cerr << "Failed to connect pipe." << endl;
            CloseHandle(pipe);
            exit(1);
        }
    }
}

void Updatemulti(int &lastDropTime) {
    if (isPaused) return; // Skip if the game is paused

    int currentTime = GetTickCount();
    if (currentTime - lastDropTime > 500) { // 500 ms for block to move down
        if (!CheckCollision(0, 1, currentBlock)) {
            currentY++;
        } else {
            MergeBlockIntoBoard();
            ClearLines();
            memcpy(currentBlock, nextBlock, sizeof(currentBlock));
            currentBlockColor = nextBlockColor;
            GenerateNextBlock();
            currentX = BoardWidth / 2 - 2;
            currentY = 0;
            if (CheckCollision(0, 0, currentBlock)) gameOver = true;
        }
        lastDropTime = currentTime;
    }
}

void Input2() {
    char ch;
    DWORD bytesRead;
    if (ReadFile(pipe, &ch, sizeof(ch), &bytesRead, NULL) && bytesRead > 0) {
        switch (ch) {
            case 'a': // Move left
                if (!isPaused && !CheckCollision(-1, 0, currentBlock)) {
                    currentX--;
                }
                break;
            case 'd': // Move right
                if (!isPaused && !CheckCollision(1, 0, currentBlock)) {
                    currentX++;
                }
                break;
            case 's': // Move down
                if (!isPaused && !CheckCollision(0, 1, currentBlock)) {
                    currentY++;
                }
                break;
            case 'w': // Rotate
                if (!isPaused) {
                    RotateBlock();
                }
                break;
            case 'p': // Pause or resume
                isPaused = !isPaused;
                break;
        }
    }
}


void Input3() {
    if (_kbhit()) {
        char ch = _getch();
        switch (ch) {
            case 'a': // Move left
                if (!isPaused && !CheckCollision(-1, 0, currentBlock)) {
                    currentX--;
                }
                break;
            case 'd': // Move right
                if (!isPaused && !CheckCollision(1, 0, currentBlock)) {
                    currentX++;
                }
                break;
            case 's': // Move down
                if (!isPaused && !CheckCollision(0, 1, currentBlock)) {
                    currentY++;
                }
                break;
            case 'w': // Rotate
                if (!isPaused) {
                    RotateBlock();
                }
                break;

            case '0': // Pause or resume
                blockType = 0;
                break;

            case '1': // Pause or resume
                blockType = 1;
                break;

            case '2': // Pause or resume
                blockType = 2 ;
                break;

            case '3': // Pause or resume
                blockType = 3;
                break;

            case '4': // Pause or resume
                blockType = 4;
                break;

            case '5': // Pause or resume
                blockType = 5;
                break;

            case '6': // Pause or resume
                blockType = 6;
                break;


        }
    }
}

void TetrisGame(){
    // Get user input for board dimensions
    cout << "Enter board width: ";
    cin >> BoardWidth;
    cout << "Enter board height: ";
    cin >> BoardHeight;

    // Validate input (optional)
    if (BoardWidth < 5 || BoardHeight < 5) {
        cout << "Width and height must be at least 4." << endl;
    }
    int delay;
    int i;
    cout << "difficulty level 1 2";
    cin >> i;
    if (i == 1) {
        delay = 500;
    } else if (i == 2) {
        delay = 250;
    }

    string playerName;
    cout << "Enter your name: ";
    cin >> playerName;

    // Dynamically allocate the game board
    board = new Cell *[BoardHeight];
    for (int i = 0; i < BoardHeight; i++) {
        board[i] = new Cell[BoardWidth];
    }


    // Rest of the initialization...
    InitializeGame();

    time_t startTime, endTime;
    startTime = time(nullptr); // Start time


    // Main game loop
    while (!gameOver) {
        Input();
        Update();
        Render();
        Sleep(delay); // Adjust speed
    }
    endTime = time(nullptr); // End time

    cout << "Game Over! Score: " << score << endl;
    // Calculate play time
    double playTime = difftime(endTime, startTime);

    // Prepare score data
    PlayerScore currentPlayer;
    currentPlayer.name = playerName;
    currentPlayer.score = score;
    currentPlayer.playTime = playTime;
    currentPlayer.timestamp = time(nullptr);

    // Choose leaderboard file based on difficulty
    string leaderboardFile = (i == 1) ? "leaderboard_easy.txt" : "leaderboard_hard.txt";

    // Update leaderboard
    updateLeaderboard(leaderboardFile, currentPlayer);

    // Clean up the dynamically allocated memory
    for (int i = 0; i < BoardHeight; ++i) {
        delete[] board[i];
    }
    delete[] board;
}

void TetrisM(){
    Auto = false;
    // Get user input for board dimensions
    cout << "Enter board width: ";
    cin >> BoardWidth;
    cout << "Enter board height: ";
    cin >> BoardHeight;

    // Validate input (optional)
    if (BoardWidth < 5 || BoardHeight < 5) {
        cout << "Width and height must be at least 4." << endl;
    }
    int delay;
    int i;
    cout << "difficulty level 1 2";
    cin >> i;
    if (i == 1) {
        delay = 500;
    } else if (i == 2) {
        delay = 250;
    }

    string playerName;
    cout << "Enter your name: ";
    cin >> playerName;

    // Dynamically allocate the game board
    board = new Cell *[BoardHeight];
    for (int i = 0; i < BoardHeight; i++) {
        board[i] = new Cell[BoardWidth];
    }


    // Rest of the initialization...
    InitializeGame();

    time_t startTime, endTime;
    startTime = time(nullptr); // Start time


    // Main game loop
    while (!gameOver) {
        Input3();
        Update();
        Render();
        Sleep(delay); // Adjust speed
    }
    endTime = time(nullptr); // End time

    cout << "Game Over! Score: " << score << endl;
    // Calculate play time
    double playTime = difftime(endTime, startTime);


    // Prepare score data
    PlayerScore currentPlayer;
    currentPlayer.name = playerName;
    currentPlayer.score = score;
    currentPlayer.playTime = playTime;
    currentPlayer.timestamp = time(nullptr);

    // Clean up the dynamically allocated memory
    for (int i = 0; i < BoardHeight; ++i) {
        delete[] board[i];
    }
    delete[] board;
}

int main(int argc, char* argv[]) {

    if (argc > 1 && string(argv[1]) == "run_game") {
        TetrisGameM();  // Run the game directly if the special argument is present
        return 0;
    }

    int menuin;
    cout << "1 - New Game" << '\n';
    cout << "2 - How to Play" << '\n';
    cout << "3 - Leaderboard" << '\n';
    cout << "4 - Player vs Player" << '\n';
    cout << "5 - manual Block" << '\n';
    cout << "6 - Exit" << '\n';
    cin >> menuin;


    switch (menuin) {

        case 1: {
            TetrisGame();
            main(argc , argv) ;
            break;
        }
        case 2:{
            showHowToPlay();

            main(argc , argv) ;
            break;
        }
        case 3: {
            // Optionally, show leaderboard
            cout << "difficulty level 1 2";
            int showLB;
            cin >> showLB;
            string showleaderboardFile = (showLB == 1) ? "leaderboard_easy.txt" : "leaderboard_hard.txt";
            showLeaderboard(showleaderboardFile);
            main(argc , argv);
            break;
        }
        case 6:{
            cout << "thanks for playing";
            return 0;
        }
        case 4:{
            system("start cmd.exe /k .\\a.exe run_game");
            system("start cmd.exe /k .\\a.exe run_game");
            return 0;
        }
        case 5:{
            TetrisM();
            main(argc , argv);
            break;
        }
    }
    return 0;

}
