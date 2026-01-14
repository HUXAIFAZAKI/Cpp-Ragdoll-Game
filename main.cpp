//----------------------------------
//
//  ASCII PHYSICS GAME
//  HUZAIFA ZAKI - FA25-BSAI-0051
//  UMAR SAEED - FA25-BSAI-0065
//
//---------------------------------

#include <windows.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define WIDTH 120
#define HEIGHT 40
#define MAX_POINTS 1000
#define MAX_STICKS 1000
#define MAX_BOXES 20
#define MAX_PARTICLES 150

// Game Constants
const float GRAVITY = 0.3f;
const float FRICTION = 0.98f;
const float BOUNCE = 0.6f;
const int CONSTRAINT_ITERATIONS = 8;
const float STICK_BREAK_FACTOR = 3.0f;
const float DRAG_SMOOTHNESS = 0.3f;

// More Constants
const int UI_BAR_HEIGHT = 1;
const int GAME_AREA_TOP = 2;
const int STATUS_BAR_Y = HEIGHT - 2;
const float CURSOR_SPEED_NORMAL = 1.0f;
const float CURSOR_SPEED_FAST = 2.0f;
const float CURSOR_SPEED_PRECISE = 0.5f;
const float PULSE_SPEED = 3.0f;
const float SHAKE_DECAY = 0.15f;
const float EXPLOSION_RADIUS = 15.0f;
const float EXPLOSION_POWER = 2.5f;
const float BLUR_SPEED_THRESHOLD = 2.0f;
const int MAX_UNDO_STATES = 5;
const int MAX_SHOP_ITEMS = 8;
const char SAVE_FILE[] = "game_save.txt";

// Colors
const int COLOR_BLACK = 0;
const int COLOR_WHITE = 15;
const int COLOR_RED = 12;
const int COLOR_GREEN = 10;
const int COLOR_YELLOW = 14;
const int COLOR_CYAN = 11;
const int COLOR_MAGENTA = 13;
const int COLOR_BLUE = 9;
const int COLOR_GRAY = 8;
const int COLOR_BRIGHT_RED = 12;
const int COLOR_BRIGHT_GREEN = 10;
const int COLOR_BRIGHT_YELLOW = 14;
const int COLOR_BRIGHT_CYAN = 11;
const int COLOR_BRIGHT_MAGENTA = 13;
const int COLOR_BRIGHT_BLUE = 9;
const int COLOR_DARK_GRAY = 7;
const int COLOR_ORANGE = 6;

// Game Statistics
struct GameStats {
    int objectsSpawned;
    int explosionsTriggered;
    int ragdollsCreated;
    int sticksBreached;
    float totalPlayTime;
    int missionsCompleted;
    int coins;
};

// Input Manager
struct InputManager {
    int keys[256];
    int keysPressed[256];
    int keysReleased[256];
    DWORD lastKeyTime[256];
};

// Sound Manager
struct SoundManager {
    int enabled;
};

// Shop Item
struct ShopItem {
    char name[20];
    char symbol;
    int price;
    int unlocked;
    int color;
    char description[50];
};

// Point Structure
struct Point {
    float x, y;
    float oldX, oldY;
    int isLocked;
    int isActive;
    char symbol;
    float radius;
    int isRagdollPart;
    int color;
    int isSpecialHead;  // For shop heads
};

// Stick Structure
struct Stick {
    int p1, p2;
    float length;
    int active;
    int isRagdollStick;
};

// Box Structure
struct Box {
    float x, y;
    float width, height;
    int isActive;
    int isSolid;
    int isWall;
};

// Target Structure
struct Target {
    float x, y;
    float radius;
    int isActive;
    int ragdollTouching;
};

// Undo System
struct GameState {
    int savedPointCount;
    int savedStickCount;
    int savedBoxCount;
    struct Point savedPoints[MAX_POINTS];
    struct Stick savedSticks[MAX_STICKS];
    struct Box savedBoxes[MAX_BOXES];
    int isValid;
};

// Particle Structure
struct Particle {
    float x, y;
    float vx, vy;
    int life;
    int maxLife;
    int color;
    char symbol;
    int active;
};


// Global variables
Point points[MAX_POINTS];
Stick sticks[MAX_STICKS];
Box boxes[MAX_BOXES];
Target targets[5];
Particle particles[MAX_PARTICLES];
char screenBuf[WIDTH * HEIGHT];
int colorBuf[WIDTH * HEIGHT];

int pointCount = 0;
int stickCount = 0;
int boxCount = 0;
int targetCount = 0;

int curX = 60;
int curY = 10;

int currentTool = 1;
int isSimulating = 0;
int dragMode = 0;
int dragPoint = -1;
int ropeStartX = -1;
int ropeStartY = -1;

int currentMode = 0;
int menuSelection = 0;

int currentMission = 1;
int maxMissions = 5;
int missionComplete = 0;
int missionFailed = 0;
int ragdollBroken = 0;
float missionTimer = 0;
float missionTimeLimit = 30.0f;
int targetsReached = 0;

float screenShake = 0.0f;
float particleTime = 0.0f;
float gameTime = 0.0f;

int hangmanModeActive = 0;
char hangmanWord[50];
char guessedWord[50];
char guessedLetters[27];
int wordLength = 0;
int wrongGuesses = 0;
int maxWrongGuesses = 6;
int hangmanGameOver = 0;
int hangmanWon = 0;
char currentLetter = 0;
int stickmanIntact = 1;
int currentHangmanStage = 0;

// variables
GameStats gameStats;
GameState undoStates[MAX_UNDO_STATES];
int currentUndoIndex = 0;
int undoCount = 0;
InputManager inputManager;
SoundManager soundManager;
int showHelp = 0;
int debugMode = 0;
float cursorSpeedMultiplier = CURSOR_SPEED_NORMAL;

// Shop variables
ShopItem shopItems[MAX_SHOP_ITEMS];
int currentHeadIndex = 0; 
int shopSelection = 0;
int shopPage = 0;
int itemsPerPage = 6;

// Function Prototypes
void ClearWorld();
void InitInputManager();
void UpdateInputManager();
int IsKeyPressed(int key);
int IsKeyDown(int key);
int IsKeyReleased(int key);
void UpdateCursor();
void DrawEnhancedCursor();
void DrawToolSelection();
void DrawStatusBar();
void DrawHelpOverlay();
void DrawDebugInfo();
void SaveState();
void Undo();
void PlaySoundPlace();
void PlaySoundBreak();
void PlaySoundExplosion();
void PlaySoundSuccess();
void PlaySoundFailure();
void PlaySoundClick();
void PlaySoundDrag();
void PlaySoundCoin();
void ToggleSound();
void SpawnExplosionParticles(float x, float y);
void SpawnBreakParticles(float x, float y);
void SpawnSuccessParticles(float x, float y);
void SpawnCoinParticles(float x, float y);
void DrawPointWithEffects(Point* p, int shakeX, int shakeY);
void DrawAnimatedTargets();
void UpdateMissionWithStats(float deltaTime);
void InitShop();
void DrawShop();
void HandleShopInput();
void BuyItem(int index);
void EquipHead(int index);
void SaveGame();
void LoadGame();
int AddPoint(float x, float y, char symbol, int locked, float radius, int isRagdoll, int color, int isSpecial);
void AddStick(int p1, int p2, int isRagdoll);
int AddBox(float x, float y, float w, float h, int solid, int isWall);
void AddTarget(float x, float y, float radius);
void SpawnRagdoll(int x, int y);
void SpawnBomb(int x, int y);
void SpawnRope(int x1, int y1, int x2, int y2);
void SpawnPlatform(int x, int y, int width);
void SpawnMovableBox(int x, int y);
void SpawnCoin(int x, int y);
void BreakRandomStick();
void InitHangmanMode();
void ProcessHangmanGuess(char letter);
void DrawHangmanUI();
void Explode(int x, int y);
void ClampPointToBox(int pointIndex, int boxIndex);
void ResolveBoxCollisions();
int CheckRagdollInTarget(int targetIndex);
int CheckRagdollIntegrity();
void InitMission(int missionNum);
void UpdateParticles();
void UpdatePhysics();
void DrawScreen(HANDLE hOut);
void ShowMainMenu(HANDLE hOut);
void ShowMissionComplete(HANDLE hOut);
void ShowMissionFailed(HANDLE hOut);
int FindNearestPoint(int x, int y, float maxDist);
void DrawMissionStartScreen(HANDLE hOut, int missionNum);
float GetDistance(float x1, float y1, float x2, float y2);
void PutChar(int x, int y, char c, int color);
void DrawLine(int x0, int y0, int x1, int y1, char c, int color);
void SpawnParticle(float x, float y, int color, char symbol, float speed);

//---------------------------------------------------------------------
// INITIALIZATION FUNCTIONS
//---------------------------------------------------------------------

void InitInputManager() {
    for (int i = 0; i < 256; i++) {
        inputManager.keys[i] = 0;
        inputManager.keysPressed[i] = 0;
        inputManager.keysReleased[i] = 0;
        inputManager.lastKeyTime[i] = 0;
    }
}

void UpdateInputManager() {
    DWORD currentTime = GetTickCount();

    for (int i = 0; i < 256; i++) {
        int currentState = (GetAsyncKeyState(i) & 0x8000) != 0;
        inputManager.keysPressed[i] = currentState && !inputManager.keys[i];
        inputManager.keysReleased[i] = !currentState && inputManager.keys[i];

        if (inputManager.keysPressed[i]) {
            inputManager.lastKeyTime[i] = currentTime;
        }

        inputManager.keys[i] = currentState;
    }
}

int IsKeyPressed(int key) { return inputManager.keysPressed[key]; }
int IsKeyDown(int key) { return inputManager.keys[key]; }
int IsKeyReleased(int key) { return inputManager.keysReleased[key]; }

void InitSoundManager() {
    soundManager.enabled = 1;
}

void InitShop() {
    // Default head (always unlocked)
    strcpy_s(shopItems[0].name, "Classic");
    shopItems[0].symbol = 'O';
    shopItems[0].price = 0;
    shopItems[0].unlocked = 1;
    shopItems[0].color = COLOR_BRIGHT_YELLOW;
    strcpy_s(shopItems[0].description, "Default stickman head");

    // Premium heads
    strcpy_s(shopItems[1].name, "Robot");
    shopItems[1].symbol = '@';
    shopItems[1].price = 100;
    shopItems[1].unlocked = 0;
    shopItems[1].color = COLOR_BRIGHT_CYAN;
    strcpy_s(shopItems[1].description, "Robotic head with blue glow");

    strcpy_s(shopItems[2].name, "Skull");
    shopItems[2].symbol = 'S';
    shopItems[2].price = 150;
    shopItems[2].unlocked = 0;
    shopItems[2].color = COLOR_WHITE;
    strcpy_s(shopItems[2].description, "Spooky skull head");

    strcpy_s(shopItems[3].name, "Alien");
    shopItems[3].symbol = 'A';
    shopItems[3].price = 200;
    shopItems[3].unlocked = 0;
    shopItems[3].color = COLOR_BRIGHT_GREEN;
    strcpy_s(shopItems[3].description, "Alien head with green aura");

    strcpy_s(shopItems[4].name, "King");
    shopItems[4].symbol = 'K';
    shopItems[4].price = 300;
    shopItems[4].unlocked = 0;
    shopItems[4].color = COLOR_BRIGHT_MAGENTA;
    strcpy_s(shopItems[4].description, "Royal crown head");

    strcpy_s(shopItems[5].name, "Ninja");
    shopItems[5].symbol = 'N';
    shopItems[5].price = 250;
    shopItems[5].unlocked = 0;
    shopItems[5].color = COLOR_DARK_GRAY;
    strcpy_s(shopItems[5].description, "Stealthy ninja head");

    strcpy_s(shopItems[6].name, "Wizard");
    shopItems[6].symbol = 'W';
    shopItems[6].price = 400;
    shopItems[6].unlocked = 0;
    shopItems[6].color = COLOR_BRIGHT_BLUE;
    strcpy_s(shopItems[6].description, "Magical wizard hat");

    strcpy_s(shopItems[7].name, "Demon");
    shopItems[7].symbol = 'D';
    shopItems[7].price = 500;
    shopItems[7].unlocked = 0;
    shopItems[7].color = COLOR_BRIGHT_RED;
    strcpy_s(shopItems[7].description, "Fiery demon head");
}

void SaveGame() {
    FILE* file;
    errno_t err = fopen_s(&file, SAVE_FILE, "w");
    if (err == 0 && file) {
        fprintf(file, "COINS:%d\n", gameStats.coins);
        fprintf(file, "CURRENT_HEAD:%d\n", currentHeadIndex);
        fprintf(file, "MISSIONS:%d\n", gameStats.missionsCompleted);

        for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
            fprintf(file, "ITEM%d:%d\n", i, shopItems[i].unlocked);
        }

        fclose(file);
    }
}

void LoadGame() {
    FILE* file;
    errno_t err = fopen_s(&file, SAVE_FILE, "r");
    if (err == 0 && file) {
        char line[100];
        while (fgets(line, 100, file)) {
            if (strstr(line, "COINS:")) {
                sscanf_s(line, "COINS:%d", &gameStats.coins);
            }
            else if (strstr(line, "CURRENT_HEAD:")) {
                sscanf_s(line, "CURRENT_HEAD:%d", &currentHeadIndex);
            }
            else if (strstr(line, "MISSIONS:")) {
                sscanf_s(line, "MISSIONS:%d", &gameStats.missionsCompleted);
            }
            else if (strstr(line, "ITEM")) {
                int index, unlocked;
                sscanf_s(line, "ITEM%d:%d", &index, &unlocked);
                if (index >= 0 && index < MAX_SHOP_ITEMS) {
                    shopItems[index].unlocked = unlocked;
                }
            }
        }
        fclose(file);
    }
}

//---------------------------------------------------------------------
// SOUND FUNCTIONS
//---------------------------------------------------------------------

void PlaySoundPlace() {
    if (!soundManager.enabled) return;
    Beep(800, 50);
}

void PlaySoundBreak() {
    if (!soundManager.enabled) return;
    Beep(400, 80);
    Beep(300, 60);
}

void PlaySoundExplosion() {
    if (!soundManager.enabled) return;
    Beep(250, 80); 
}

void PlaySoundSuccess() {
    if (!soundManager.enabled) return;
    Beep(523, 100);
    Beep(659, 100);
    Beep(784, 150);
}

void PlaySoundFailure() {
    if (!soundManager.enabled) return;
    Beep(400, 150);
    Beep(300, 150);
    Beep(200, 200);
}

void PlaySoundClick() {
    if (!soundManager.enabled) return;
    Beep(1000, 30);
}

void PlaySoundDrag() {
    if (!soundManager.enabled) return;
    Beep(600, 20);
}

void PlaySoundCoin() {
    if (!soundManager.enabled) return;
    Beep(800, 100);
    Beep(1000, 100);
}

void ToggleSound() {
    soundManager.enabled = !soundManager.enabled;
}

//---------------------------------------------------------------------
// UNDO SYSTEM
//---------------------------------------------------------------------

void SaveState() {
    if (currentMode != 1) return;

    currentUndoIndex = (currentUndoIndex + 1) % MAX_UNDO_STATES;
    undoStates[currentUndoIndex].savedPointCount = pointCount;
    undoStates[currentUndoIndex].savedStickCount = stickCount;
    undoStates[currentUndoIndex].savedBoxCount = boxCount;

    for (int i = 0; i < pointCount; i++) {
        undoStates[currentUndoIndex].savedPoints[i] = points[i];
    }

    for (int i = 0; i < stickCount; i++) {
        undoStates[currentUndoIndex].savedSticks[i] = sticks[i];
    }

    for (int i = 0; i < boxCount; i++) {
        undoStates[currentUndoIndex].savedBoxes[i] = boxes[i];
    }

    undoStates[currentUndoIndex].isValid = 1;
    if (undoCount < MAX_UNDO_STATES) undoCount++;
}

void Undo() {
    if (undoCount == 0 || !undoStates[currentUndoIndex].isValid) return;

    pointCount = undoStates[currentUndoIndex].savedPointCount;
    stickCount = undoStates[currentUndoIndex].savedStickCount;
    boxCount = undoStates[currentUndoIndex].savedBoxCount;

    for (int i = 0; i < pointCount; i++) {
        points[i] = undoStates[currentUndoIndex].savedPoints[i];
    }

    for (int i = 0; i < stickCount; i++) {
        sticks[i] = undoStates[currentUndoIndex].savedSticks[i];
    }

    for (int i = 0; i < boxCount; i++) {
        boxes[i] = undoStates[currentUndoIndex].savedBoxes[i];
    }

    currentUndoIndex = (currentUndoIndex - 1 + MAX_UNDO_STATES) % MAX_UNDO_STATES;
    undoCount--;
}

//---------------------------------------------------------------------
// CURSOR AND INPUT FUNCTIONS
//---------------------------------------------------------------------

void UpdateCursor() {
    // Speed modifiers
    if (IsKeyDown(VK_SHIFT)) {
        cursorSpeedMultiplier = CURSOR_SPEED_FAST;
    }
    else if (IsKeyDown(VK_CONTROL)) {
        cursorSpeedMultiplier = CURSOR_SPEED_PRECISE;
    }
    else {
        cursorSpeedMultiplier = CURSOR_SPEED_NORMAL;
    }

    // Movement
    int moveAmount = (int)cursorSpeedMultiplier;
    if (moveAmount < 1) moveAmount = 1;

    if (IsKeyDown(VK_LEFT)) curX -= moveAmount;
    if (IsKeyDown(VK_RIGHT)) curX += moveAmount;
    if (IsKeyDown(VK_UP)) curY -= moveAmount;
    if (IsKeyDown(VK_DOWN)) curY += moveAmount;

    // Clamp
    if (curX < 0) curX = 0;
    if (curX >= WIDTH) curX = WIDTH - 1;
    if (curY < GAME_AREA_TOP) curY = GAME_AREA_TOP;
    if (curY >= HEIGHT - 2) curY = HEIGHT - 3;
}

void DrawEnhancedCursor() {
    // Main cursor
    char cursorChar = dragMode ? 'X' : '+';
    int cursorColor = dragMode ? COLOR_BRIGHT_RED : COLOR_BRIGHT_MAGENTA;

    PutChar(curX, curY, cursorChar, cursorColor);

    // Crosshair for precise mode
    if (cursorSpeedMultiplier == CURSOR_SPEED_PRECISE) {
        PutChar(curX - 1, curY, '-', COLOR_GRAY);
        PutChar(curX + 1, curY, '-', COLOR_GRAY);
        PutChar(curX, curY - 1, '|', COLOR_GRAY);
        PutChar(curX, curY + 1, '|', COLOR_GRAY);
    }
}

//---------------------------------------------------------------------
// UI DRAWING FUNCTIONS
//---------------------------------------------------------------------

void DrawToolSelection() {
    if (currentMode != 1) return;

    const char* toolNames[] = { "Ragdoll", "Movable Box", "Bomb", "Rope", "Platform" };
    const char* toolIcons[] = { "[O]", "[#]", "[@]", "[~]", "[=]" };
    int toolColors[] = { COLOR_BRIGHT_YELLOW, COLOR_BRIGHT_MAGENTA,
                        COLOR_BRIGHT_RED, COLOR_BRIGHT_YELLOW, COLOR_BRIGHT_GREEN };

    // Draw tool bar
    char toolDisplay[100];
    sprintf_s(toolDisplay, 100, "TOOL: %s %s",
        toolIcons[currentTool - 1], toolNames[currentTool - 1]);

    int len = strlen(toolDisplay);
    int startX = 2;

    for (int i = 0; i < len; i++) {
        PutChar(startX + i, 1, toolDisplay[i], toolColors[currentTool - 1]);
    }

    // Draw all tools with current highlighted
    int toolX = WIDTH - 50;
    for (int t = 0; t < 5; t++) {
        int col = (t == currentTool - 1) ? toolColors[t] : COLOR_GRAY;
        char num[3];
        sprintf_s(num, 3, "%d", t + 1);
        PutChar(toolX + t * 10, 1, num[0], col);
        PutChar(toolX + t * 10 + 1, 1, ':', col);

        int iconLen = strlen(toolIcons[t]);
        for (int i = 0; i < iconLen; i++) {
            PutChar(toolX + t * 10 + 2 + i, 1, toolIcons[t][i], col);
        }
    }
}

void DrawStatusBar() {
    static DWORD lastFPSTime = GetTickCount();
    static int frameCount = 0;
    static int currentFPS = 0;

    frameCount++;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastFPSTime >= 1000) {
        currentFPS = frameCount;
        frameCount = 0;
        lastFPSTime = currentTime;
    }

    // Status bar content
    char status[WIDTH];

    if (currentMode == 1) {
        sprintf_s(status, WIDTH,
            "Objects: %d/%d | FPS: %d | Coins: %d | [SHIFT]=Fast [CTRL]=Precise | [U]=Undo(%d)",
            pointCount, MAX_POINTS, currentFPS, gameStats.coins, undoCount);
    }
    else if (currentMode == 2) {
        sprintf_s(status, WIDTH,
            "Mission: %d/%d | Time: %.1f/%.1f | Targets: %d/%d | FPS: %d | Coins: %d",
            currentMission, maxMissions, missionTimer, missionTimeLimit,
            targetsReached, targetCount, currentFPS, gameStats.coins);
    }
    else if (currentMode == 3) {
        sprintf_s(status, WIDTH,
            "Hangman | Wrong: %d/%d | Coins: %d",
            wrongGuesses, maxWrongGuesses, gameStats.coins);
    }
    else if (currentMode == 4) {
        sprintf_s(status, WIDTH,
            "SHOP | Coins: %d | [ENTER]=Buy/Equip [ARROWS]=Navigate [ESC]=Back",
            gameStats.coins);
    }

    int len = strlen(status);
    for (int i = 0; i < len && i < WIDTH; i++) {
        PutChar(i, HEIGHT - 1, status[i], COLOR_BRIGHT_CYAN);
    }
}

void DrawHelpOverlay() {
    if (!showHelp) return;

    int boxWidth = 70;
    int boxHeight = 28;
    int startX = (WIDTH - boxWidth) / 2;
    int startY = (HEIGHT - boxHeight) / 2;

    // Draw box
    for (int y = 0; y < boxHeight; y++) {
        for (int x = 0; x < boxWidth; x++) {
            if (x == 0 || x == boxWidth - 1 || y == 0 || y == boxHeight - 1) {
                PutChar(startX + x, startY + y, '#', COLOR_BRIGHT_YELLOW);
            }
            else {
                PutChar(startX + x, startY + y, ' ', COLOR_BLACK);
            }
        }
    }

    // Help text
    const char* helpLines[] = {
        "=== ASCII PHYSICS GAME CONTROLS ===",
        "",
        "GLOBAL CONTROLS:",
        "ESC - Return to menu",
        "H - Toggle this help",
        "S - Toggle sound",
        "F1 - Toggle debug mode",
        "",
        "SANDBOX MODE:",
        "1-5 - Select tool (Ragdoll/Box/Bomb/Rope/Platform)",
        "ENTER - Place object",
        "SPACE - Play/Pause simulation",
        "R - Reset world",
        "D - Toggle drag mode",
        "U - Undo last action",
        "SHIFT - Fast cursor movement",
        "CTRL - Precise cursor movement",
        "",
        "MISSION MODE:",
        "Same as sandbox but with mission objectives",
        "Guide ragdoll to targets before time runs out!",
        "",
        "Press H to close"
    };

    int numLines = sizeof(helpLines) / sizeof(helpLines[0]);
    for (int i = 0; i < numLines; i++) {
        int len = strlen(helpLines[i]);
        int textX = startX + (boxWidth - len) / 2;
        for (int j = 0; j < len; j++) {
            PutChar(textX + j, startY + 2 + i, helpLines[i][j], COLOR_WHITE);
        }
    }
}

void DrawDebugInfo() {
    if (!debugMode) return;

    int debugY = 3;
    int debugX = 2;

    char debug[100];

    sprintf_s(debug, 100, "[DEBUG] Points: %d/%d Active: ", pointCount, MAX_POINTS);
    int activePoints = 0;
    for (int i = 0; i < pointCount; i++) if (points[i].isActive) activePoints++;
    char temp[20];
    sprintf_s(temp, 20, "%d", activePoints);
    strcat_s(debug, 100, temp);

    for (int i = 0; i < strlen(debug); i++) {
        PutChar(debugX + i, debugY, debug[i], COLOR_YELLOW);
    }
    debugY++;

    sprintf_s(debug, 100, "[DEBUG] Sticks: %d/%d Active: ", stickCount, MAX_STICKS);
    int activeSticks = 0;
    for (int i = 0; i < stickCount; i++) if (sticks[i].active) activeSticks++;
    sprintf_s(temp, 20, "%d", activeSticks);
    strcat_s(debug, 100, temp);

    for (int i = 0; i < strlen(debug); i++) {
        PutChar(debugX + i, debugY, debug[i], COLOR_YELLOW);
    }
    debugY++;

    sprintf_s(debug, 100, "[DEBUG] Particles: ");
    int activeParticles = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) if (particles[i].active) activeParticles++;
    sprintf_s(temp, 20, "%d/%d", activeParticles, MAX_PARTICLES);
    strcat_s(debug, 100, temp);

    for (int i = 0; i < strlen(debug); i++) {
        PutChar(debugX + i, debugY, debug[i], COLOR_YELLOW);
    }
    debugY++;

    sprintf_s(debug, 100, "[DEBUG] Cursor: (%d, %d) Tool: %d", curX, curY, currentTool);
    for (int i = 0; i < strlen(debug); i++) {
        PutChar(debugX + i, debugY, debug[i], COLOR_YELLOW);
    }
}

void DrawShop() {
    // Clear screen
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    // Draw title
    char title[] = "=== STICKMAN HEAD SHOP ===";
    int titleLen = strlen(title);
    int titleX = (WIDTH - titleLen) / 2;
    for (int i = 0; i < titleLen; i++) {
        PutChar(titleX + i, 3, title[i], COLOR_BRIGHT_MAGENTA);
    }

    char coinText[50];
    sprintf_s(coinText, 50, "Coins: %d", gameStats.coins);
    int coinLen = strlen(coinText);
    int coinX = (WIDTH - coinLen) / 2;
    for (int i = 0; i < coinLen; i++) {
        PutChar(coinX + i, 5, coinText[i], COLOR_BRIGHT_YELLOW);
    }

    // Draw shop items
    int startY = 8;
    int itemsToShow = itemsPerPage;
    if (shopPage * itemsPerPage + itemsPerPage > MAX_SHOP_ITEMS) {
        itemsToShow = MAX_SHOP_ITEMS - shopPage * itemsPerPage;
    }

    for (int i = 0; i < itemsToShow; i++) {
        int index = shopPage * itemsPerPage + i;
        ShopItem* item = &shopItems[index];

        int itemY = startY + i * 4;
        int isSelected = (shopSelection == i);
        int isEquipped = (currentHeadIndex == index);

        // Draw selection indicator
        if (isSelected) {
            char selector[] = ">> ";
            for (int j = 0; j < 3; j++) {
                PutChar(20 + j, itemY, selector[j], COLOR_BRIGHT_GREEN);
            }
        }

        // Draw head preview
        PutChar(25, itemY, item->symbol, item->color);

        // Draw item info
        char itemText[100];
        if (item->unlocked) {
            if (isEquipped) {
                sprintf_s(itemText, 100, "%s [EQUIPPED] - %s", item->name, item->description);
                for (int j = 0; j < strlen(itemText); j++) {
                    PutChar(28 + j, itemY, itemText[j], COLOR_BRIGHT_GREEN);
                }
            }
            else {
                sprintf_s(itemText, 100, "%s [UNLOCKED] - %s", item->name, item->description);
                for (int j = 0; j < strlen(itemText); j++) {
                    PutChar(28 + j, itemY, itemText[j], COLOR_BRIGHT_CYAN);
                }
            }
        }
        else {
            sprintf_s(itemText, 100, "%s - %d coins - %s", item->name, item->price, item->description);
            for (int j = 0; j < strlen(itemText); j++) {
                PutChar(28 + j, itemY, itemText[j], (gameStats.coins >= item->price) ? COLOR_WHITE : COLOR_GRAY);
            }
        }

        // Draw price or status
        char status[50];
        if (item->unlocked) {
            if (isEquipped) {
                sprintf_s(status, 50, "[EQUIPPED]");
                for (int j = 0; j < strlen(status); j++) {
                    PutChar(WIDTH - 50 + j, itemY, status[j], COLOR_BRIGHT_GREEN);
                }
            }
            else {
                sprintf_s(status, 50, "[UNLOCKED]");
                for (int j = 0; j < strlen(status); j++) {
                    PutChar(WIDTH - 50 + j, itemY, status[j], COLOR_BRIGHT_CYAN);
                }
            }
        }
        else {
            sprintf_s(status, 50, "%d coins", item->price);
            for (int j = 0; j < strlen(status); j++) {
                PutChar(WIDTH - 50 + j, itemY, status[j],
                    (gameStats.coins >= item->price) ? COLOR_BRIGHT_YELLOW : COLOR_GRAY);
            }
        }
    }

    // Draw instructions
    char instr1[] = "Use UP/DOWN to select, ENTER to buy/equip, ESC to exit";
    int instr1Len = strlen(instr1);
    int instr1X = (WIDTH - instr1Len) / 2;
    for (int i = 0; i < instr1Len; i++) {
        PutChar(instr1X + i, HEIGHT - 5, instr1[i], COLOR_GRAY);
    }

    char pageText[30];
    sprintf_s(pageText, 30, "Page %d/%d", shopPage + 1, (MAX_SHOP_ITEMS + itemsPerPage - 1) / itemsPerPage);
    int pageLen = strlen(pageText);
    int pageX = (WIDTH - pageLen) / 2;
    for (int i = 0; i < pageLen; i++) {
        PutChar(pageX + i, HEIGHT - 3, pageText[i], COLOR_BRIGHT_YELLOW);
    }

    // Draw current equipped head preview
    char preview[] = "Current Head:";
    int previewLen = strlen(preview);
    int previewX = 2;
    for (int i = 0; i < previewLen; i++) {
        PutChar(previewX + i, HEIGHT - 8, preview[i], COLOR_BRIGHT_CYAN);
    }

    PutChar(previewX + 15, HEIGHT - 8, shopItems[currentHeadIndex].symbol, shopItems[currentHeadIndex].color);

    char headName[30];
    sprintf_s(headName, 30, "- %s", shopItems[currentHeadIndex].name);
    for (int i = 0; i < strlen(headName); i++) {
        PutChar(previewX + 17 + i, HEIGHT - 8, headName[i], shopItems[currentHeadIndex].color);
    }
}

void HandleShopInput() {
    int itemsToShow = itemsPerPage;
    if (shopPage * itemsPerPage + itemsPerPage > MAX_SHOP_ITEMS) {
        itemsToShow = MAX_SHOP_ITEMS - shopPage * itemsPerPage;
    }

    if (IsKeyPressed(VK_UP)) {
        shopSelection--;
        if (shopSelection < 0) shopSelection = itemsToShow - 1;
        PlaySoundClick();
        Sleep(100);
    }

    if (IsKeyPressed(VK_DOWN)) {
        shopSelection++;
        if (shopSelection >= itemsToShow) shopSelection = 0;
        PlaySoundClick();
        Sleep(100);
    }

    if (IsKeyPressed(VK_LEFT)) {
        shopPage--;
        if (shopPage < 0) shopPage = (MAX_SHOP_ITEMS + itemsPerPage - 1) / itemsPerPage - 1;
        shopSelection = 0;
        PlaySoundClick();
        Sleep(100);
    }

    if (IsKeyPressed(VK_RIGHT)) {
        shopPage++;
        if (shopPage >= (MAX_SHOP_ITEMS + itemsPerPage - 1) / itemsPerPage) shopPage = 0;
        shopSelection = 0;
        PlaySoundClick();
        Sleep(100);
    }

    if (IsKeyPressed(VK_RETURN)) {
        int index = shopPage * itemsPerPage + shopSelection;
        if (index >= 0 && index < MAX_SHOP_ITEMS) {
            if (shopItems[index].unlocked) {
                EquipHead(index);
            }
            else {
                BuyItem(index);
            }
        }
        Sleep(100);
    }
}

void BuyItem(int index) {
    if (index < 0 || index >= MAX_SHOP_ITEMS) return;

    if (shopItems[index].unlocked) {
        EquipHead(index);
        return;
    }

    if (gameStats.coins >= shopItems[index].price) {
        gameStats.coins -= shopItems[index].price;
        shopItems[index].unlocked = 1;
        EquipHead(index);
        PlaySoundCoin();
        SaveGame();
    }
    else {
        PlaySoundFailure();
    }
}

void EquipHead(int index) {
    if (index < 0 || index >= MAX_SHOP_ITEMS) return;

    if (shopItems[index].unlocked) {
        currentHeadIndex = index;
        PlaySoundSuccess();
        SaveGame();
    }
}

//---------------------------------------------------------------------
// PARTICLE AND EFFECT FUNCTIONS
//---------------------------------------------------------------------

void SpawnParticle(float x, float y, int color, char symbol, float speed) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].active = 1;
            particles[i].x = x;
            particles[i].y = y;
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float force = (rand() % 100 / 100.0f) * speed;
            particles[i].vx = cosf(angle) * force;
            particles[i].vy = sinf(angle) * force;
            particles[i].life = 20 + rand() % 20;
            particles[i].maxLife = particles[i].life;
            particles[i].color = color;
            particles[i].symbol = symbol;
            return;
        }
    }
}

void SpawnExplosionParticles(float x, float y) {
    // Core explosion
    for (int i = 0; i < 30; i++) {
        SpawnParticle(x, y, COLOR_BRIGHT_RED, '*', 2.5f);
    }

    // Secondary fire
    for (int i = 0; i < 20; i++) {
        SpawnParticle(x + (rand() % 5 - 2), y + (rand() % 5 - 2),
            COLOR_BRIGHT_YELLOW, '+', 2.0f);
    }

    // Smoke
    for (int i = 0; i < 15; i++) {
        SpawnParticle(x + (rand() % 7 - 3), y + (rand() % 7 - 3),
            COLOR_DARK_GRAY, 'o', 1.0f);
    }

    // Sparks
    for (int i = 0; i < 10; i++) {
        SpawnParticle(x, y, COLOR_WHITE, '.', 3.0f);
    }
}

void SpawnBreakParticles(float x, float y) {
    for (int i = 0; i < 5; i++) {
        SpawnParticle(x, y, COLOR_WHITE, '-', 1.0f);
        SpawnParticle(x, y, COLOR_GRAY, '|', 0.8f);
    }
}

void SpawnSuccessParticles(float x, float y) {
    for (int i = 0; i < 20; i++) {
        int colors[] = { COLOR_BRIGHT_GREEN, COLOR_BRIGHT_YELLOW, COLOR_BRIGHT_CYAN };
        char symbols[] = { '*', '+', 'o' };
        SpawnParticle(x, y, colors[rand() % 3], symbols[rand() % 3], 1.5f);
    }
}

void SpawnCoinParticles(float x, float y) {
    for (int i = 0; i < 10; i++) {
        SpawnParticle(x, y, COLOR_BRIGHT_YELLOW, '$', 1.0f);
    }
}

void UpdateParticles() {
    particleTime += 0.1f;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.1f;

        particles[i].life--;
        if (particles[i].life <= 0) particles[i].active = 0;

        if (particles[i].y >= HEIGHT - 1) {
            particles[i].y = HEIGHT - 2;
            particles[i].vy = -particles[i].vy * 0.5f;
            particles[i].vx *= 0.7f;
        }
    }
}

void DrawPointWithEffects(Point* p, int shakeX, int shakeY) {
    // Calculate velocity
    float dx = p->x - p->oldX;
    float dy = p->y - p->oldY;
    float speed = sqrtf(dx * dx + dy * dy);

    // Draw motion blur for fast objects
    if (speed > BLUR_SPEED_THRESHOLD) {
        int trailSteps = (int)(speed * 2);
        if (trailSteps > 5) trailSteps = 5;

        for (int t = 1; t <= trailSteps; t++) {
            float ratio = (float)t / (trailSteps + 1);
            int tx = (int)(p->oldX + dx * ratio);
            int ty = (int)(p->oldY + dy * ratio);

            int trailColor = (p->color == COLOR_WHITE) ? COLOR_GRAY : COLOR_DARK_GRAY;
            PutChar(tx + shakeX, ty + shakeY, '.', trailColor);
        }
    }

    // Draw the point itself
    PutChar((int)(p->x + 0.5f) + shakeX, (int)(p->y + 0.5f) + shakeY,
        p->symbol, p->color);
}

void DrawAnimatedTargets() {
    for (int i = 0; i < targetCount; i++) {
        if (targets[i].isActive == 0) continue;

        int tx = (int)(targets[i].x + 0.5f);
        int ty = (int)(targets[i].y + 0.5f);

        // Pulse effect
        float pulse = 0.5f + 0.5f * sinf(gameTime * PULSE_SPEED);
        int baseRadius = (int)(targets[i].radius);
        int animRadius = baseRadius + (int)(pulse * 2);

        int targetColor = (targets[i].ragdollTouching == 1) ?
            COLOR_BRIGHT_GREEN : COLOR_BRIGHT_YELLOW;
        char outerChar = (targets[i].ragdollTouching == 1) ? 'O' : 'o';

        // Inner ring (static)
        for (int angle = 0; angle < 360; angle += 30) {
            float rad = angle * 3.14159f / 180.0f;
            int cx = tx + (int)(cosf(rad) * (baseRadius - 2));
            int cy = ty + (int)(sinf(rad) * (baseRadius - 2));
            PutChar(cx, cy, '+', targetColor);
        }

        // Center
        char centerChar = (targets[i].ragdollTouching == 1) ? 'X' : '*';
        PutChar(tx, ty, centerChar, targetColor);

        // Number
        char numStr[3];
        sprintf_s(numStr, 3, "%d", i + 1);
        PutChar(tx + 2, ty, numStr[0], targetColor);

        // Success indicator
        if (targets[i].ragdollTouching == 1) {
            PutChar(tx - 3, ty, '>', COLOR_BRIGHT_GREEN);
            PutChar(tx + 3, ty, '<', COLOR_BRIGHT_GREEN);
        }
    }
}

//---------------------------------------------------------------------
// GAME OBJECT FUNCTIONS
//---------------------------------------------------------------------

float GetDistance(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx * dx + dy * dy);
}

void PutChar(int x, int y, char c, int color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    screenBuf[y * WIDTH + x] = c;
    colorBuf[y * WIDTH + x] = color;
}

void DrawLine(int x0, int y0, int x1, int y1, char c, int color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        PutChar(x0, y0, c, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int AddPoint(float x, float y, char symbol, int locked, float radius, int isRagdoll, int color, int isSpecial) {
    if (pointCount >= MAX_POINTS) return -1;

    points[pointCount].x = x;
    points[pointCount].y = y;
    points[pointCount].oldX = x;
    points[pointCount].oldY = y;
    points[pointCount].isLocked = locked;
    points[pointCount].isActive = 1;
    points[pointCount].symbol = symbol;
    points[pointCount].radius = radius;
    points[pointCount].isRagdollPart = isRagdoll;
    points[pointCount].color = color;
    points[pointCount].isSpecialHead = isSpecial;

    pointCount++;
    gameStats.objectsSpawned++;
    return pointCount - 1;
}

void AddStick(int p1, int p2, int isRagdoll) {
    if (stickCount >= MAX_STICKS) return;

    sticks[stickCount].p1 = p1;
    sticks[stickCount].p2 = p2;
    sticks[stickCount].length = GetDistance(
        points[p1].x, points[p1].y,
        points[p2].x, points[p2].y
    );
    sticks[stickCount].active = 1;
    sticks[stickCount].isRagdollStick = isRagdoll;

    stickCount++;
}

int AddBox(float x, float y, float w, float h, int solid, int isWall) {
    if (boxCount >= MAX_BOXES) return -1;

    boxes[boxCount].x = x;
    boxes[boxCount].y = y;
    boxes[boxCount].width = w;
    boxes[boxCount].height = h;
    boxes[boxCount].isActive = 1;
    boxes[boxCount].isSolid = solid;
    boxes[boxCount].isWall = isWall;

    boxCount++;
    return boxCount - 1;
}

void AddTarget(float x, float y, float radius) {
    if (targetCount >= 5) return;

    targets[targetCount].x = x;
    targets[targetCount].y = y;
    targets[targetCount].radius = radius;
    targets[targetCount].isActive = 1;
    targets[targetCount].ragdollTouching = 0;

    targetCount++;
}

void SpawnRagdoll(int x, int y) {
    SaveState();
    gameStats.ragdollsCreated++;

    // Use current head from shop
    int head = AddPoint(x, y, shopItems[currentHeadIndex].symbol, 0, 1.2f, 1, shopItems[currentHeadIndex].color, 1);
    int neck = AddPoint(x, y + 3, '|', 0, 0.8f, 1, COLOR_WHITE, 0);
    int chest = AddPoint(x, y + 6, '#', 0, 1.2f, 1, COLOR_BRIGHT_CYAN, 0);
    int hips = AddPoint(x, y + 10, 'V', 0, 1.0f, 1, COLOR_BRIGHT_CYAN, 0);

    int leftHand = AddPoint(x - 5, y + 8, '[', 0, 0.8f, 1, COLOR_WHITE, 0);
    int rightHand = AddPoint(x + 5, y + 8, ']', 0, 0.8f, 1, COLOR_WHITE, 0);

    int leftKnee = AddPoint(x - 3, y + 14, '/', 0, 0.8f, 1, COLOR_WHITE, 0);
    int rightKnee = AddPoint(x + 3, y + 14, '\\', 0, 0.8f, 1, COLOR_WHITE, 0);

    int leftFoot = AddPoint(x - 3, y + 18, '/', 0, 0.9f, 1, COLOR_WHITE, 0);
    int rightFoot = AddPoint(x + 3, y + 18, '\\', 0, 0.9f, 1, COLOR_WHITE, 0);

    if (head < 0) return;

    AddStick(head, neck, 1);
    AddStick(neck, chest, 1);
    AddStick(chest, hips, 1);

    AddStick(neck, leftHand, 1);
    AddStick(neck, rightHand, 1);

    AddStick(hips, leftKnee, 1);
    AddStick(hips, rightKnee, 1);

    AddStick(leftKnee, leftFoot, 1);
    AddStick(rightKnee, rightFoot, 1);

    PlaySoundPlace();
}

void SpawnBomb(int x, int y) {
    SaveState();
    AddPoint(x, y, '@', 0, 1.5f, 0, COLOR_BRIGHT_RED, 0);
    PlaySoundPlace();
}

void SpawnRope(int x1, int y1, int x2, int y2) {
    SaveState();
    int segments = 10;
    float dx = (float)(x2 - x1) / segments;
    float dy = (float)(y2 - y1) / segments;

    int prevPoint = AddPoint(x1, y1, 'O', 1, 0.5f, 0, COLOR_BRIGHT_YELLOW, 0);

    for (int i = 1; i <= segments; i++) {
        int isLast = (i == segments);
        int currentPoint = AddPoint(
            x1 + dx * i,
            y1 + dy * i,
            isLast ? 'O' : '.',
            isLast,
            0.5f,
            0,
            COLOR_BRIGHT_YELLOW,
            0
        );

        if (currentPoint >= 0 && prevPoint >= 0) {
            AddStick(prevPoint, currentPoint, 0);
        }
        prevPoint = currentPoint;
    }
    PlaySoundPlace();
}

void SpawnPlatform(int x, int y, int width) {
    SaveState();
    int segments = width / 3;
    int startX = x - width / 2;

    for (int i = 0; i <= segments; i++) {
        float px = startX + (i * width / (float)segments);
        int p = AddPoint(px, y, '=', 1, 0.6f, 0, COLOR_BRIGHT_GREEN, 0);
        if (i > 0) AddStick(p - 1, p, 0);
    }

    AddBox(x, y + 1.5f, width, 3, 1, 1);
    PlaySoundPlace();
}

void SpawnMovableBox(int x, int y) {
    SaveState();
    int size = 5;
    int startIdx = pointCount;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            float px = x - size + (col * size);
            float py = y - size + (row * size);
            AddPoint(px, py, '#', 0, 0.8f, 0, COLOR_BRIGHT_MAGENTA, 0);
        }
    }

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int idx = startIdx + row * 3 + col;

            if (col < 2) AddStick(idx, idx + 1, 0);
            if (row < 2) AddStick(idx, idx + 3, 0);
            if (row < 2 && col < 2) AddStick(idx, idx + 4, 0);
            if (row < 2 && col > 0) AddStick(idx, idx + 2, 0);
        }
    }
    PlaySoundPlace();
}

void SpawnCoin(int x, int y) {
    AddPoint(x, y, '$', 0, 0.5f, 0, COLOR_BRIGHT_YELLOW, 0);
}

//---------------------------------------------------------------------
// HANGMAN FUNCTIONS
//---------------------------------------------------------------------

void BreakRandomStick() {
    int sticksToBreak[][2] = {
        {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}
    };

    for (int i = 0; i < stickCount; i++) {
        if (sticks[i].isRagdollStick == 1 && sticks[i].active == 1) {
            int p1 = sticks[i].p1;
            int p2 = sticks[i].p2;
            int p1Symbol = points[p1].symbol;
            int p2Symbol = points[p2].symbol;

            if ((p1Symbol == '/' && p2Symbol == '/') || (p1Symbol == '\\' && p2Symbol == '\\')) {
                if (sticksToBreak[0][0] == -1) sticksToBreak[0][0] = i;
                else if (sticksToBreak[0][1] == -1) sticksToBreak[0][1] = i;
            }
            else if ((p1Symbol == '/' && p2Symbol == 'V') || (p1Symbol == 'V' && p2Symbol == '/') ||
                (p1Symbol == '\\' && p2Symbol == 'V') || (p1Symbol == 'V' && p2Symbol == '\\')) {
                if (sticksToBreak[1][0] == -1) sticksToBreak[1][0] = i;
                else if (sticksToBreak[1][1] == -1) sticksToBreak[1][1] = i;
            }

            else if ((p1Symbol == '[' && p2Symbol == '|') || (p1Symbol == '|' && p2Symbol == '[') ||
                (p1Symbol == ']' && p2Symbol == '|') || (p1Symbol == '|' && p2Symbol == ']')) {
                if (sticksToBreak[2][0] == -1) sticksToBreak[2][0] = i;
                else if (sticksToBreak[2][1] == -1) sticksToBreak[2][1] = i;
            }
            else if ((p1Symbol == 'V' && p2Symbol == '#') || (p1Symbol == '#' && p2Symbol == 'V')) {
                sticksToBreak[3][0] = i;
            }
            else if ((p1Symbol == '#' && p2Symbol == '|') || (p1Symbol == '|' && p2Symbol == '#')) {
                sticksToBreak[4][0] = i;
            }
            else if ((p1Symbol == 'O' && p2Symbol == '|') || (p1Symbol == '|' && p2Symbol == 'O')) {
                sticksToBreak[5][0] = i;
            }
        }
    }

    switch (wrongGuesses) {
    case 1:
        if (sticksToBreak[0][0] != -1) {
            sticks[sticksToBreak[0][0]].active = 0;
            int p1 = sticks[sticksToBreak[0][0]].p1;
            int p2 = sticks[sticksToBreak[0][0]].p2;
            if (points[p1].symbol == '/') points[p1].isLocked = 0;
            if (points[p2].symbol == '/') points[p2].isLocked = 0;
        }
        break;
    case 2:
        if (sticksToBreak[0][1] != -1) sticks[sticksToBreak[0][1]].active = 0;
        if (sticksToBreak[1][0] != -1) {
            sticks[sticksToBreak[1][0]].active = 0;
            int p1 = sticks[sticksToBreak[1][0]].p1;
            int p2 = sticks[sticksToBreak[1][0]].p2;
            if (points[p1].symbol == '/' || points[p1].symbol == '\\') points[p1].isLocked = 0;
            if (points[p2].symbol == '/' || points[p2].symbol == '\\') points[p2].isLocked = 0;
        }
        break;
    case 3:
        if (sticksToBreak[1][1] != -1) sticks[sticksToBreak[1][1]].active = 0;
        if (sticksToBreak[2][0] != -1) {
            sticks[sticksToBreak[2][0]].active = 0;
            int p1 = sticks[sticksToBreak[2][0]].p1;
            int p2 = sticks[sticksToBreak[2][0]].p2;
            if (points[p1].symbol == '[' || points[p1].symbol == ']') points[p1].isLocked = 0;
            if (points[p2].symbol == '[' || points[p2].symbol == ']') points[p2].isLocked = 0;
        }
        break;
    case 4:
        if (sticksToBreak[2][1] != -1) sticks[sticksToBreak[2][1]].active = 0;
        if (sticksToBreak[3][0] != -1) {
            sticks[sticksToBreak[3][0]].active = 0;
            int p1 = sticks[sticksToBreak[3][0]].p1;
            int p2 = sticks[sticksToBreak[3][0]].p2;
            if (points[p1].symbol == 'V') points[p1].isLocked = 0;
            if (points[p2].symbol == 'V') points[p2].isLocked = 0;
        }
        break;
    case 5:
        if (sticksToBreak[4][0] != -1) {
            sticks[sticksToBreak[4][0]].active = 0;
            int p1 = sticks[sticksToBreak[4][0]].p1;
            int p2 = sticks[sticksToBreak[4][0]].p2;
            if (points[p1].symbol == '#') points[p1].isLocked = 0;
            if (points[p2].symbol == '#') points[p2].isLocked = 0;
        }
        for (int i = 0; i < stickCount; i++) {
            if (sticks[i].isRagdollStick == 1 && sticks[i].active == 1) {
                int p1 = sticks[i].p1;
                int p2 = sticks[i].p2;
                if ((points[p1].symbol == '[' || points[p1].symbol == ']') ||
                    (points[p2].symbol == '[' || points[p2].symbol == ']')) {
                    sticks[i].active = 0;
                    break;
                }
            }
        }
        break;
    case 6:
        if (sticksToBreak[5][0] != -1) sticks[sticksToBreak[5][0]].active = 0;
        for (int i = 0; i < pointCount; i++) {
            if (points[i].isRagdollPart == 1) {
                points[i].isLocked = 0;
            }
        }
        for (int i = 0; i < stickCount; i++) {
            if (sticks[i].isRagdollStick == 1 && sticks[i].active == 1) {
                sticks[i].active = 0;
            }
        }
        break;
    }

    for (int i = 0; i < pointCount; i++) {
        if (points[i].isRagdollPart == 1 && points[i].isLocked == 0) {
            points[i].oldX = points[i].x + (rand() % 3 - 1) * 1.0f;
            points[i].oldY = points[i].y + (rand() % 2) * 1.0f;
        }
    }
}

void InitHangmanMode() {
    srand(time(NULL));
    ClearWorld();
    hangmanModeActive = 1;
    wrongGuesses = 0;
    hangmanGameOver = 0;
    hangmanWon = 0;
    stickmanIntact = 1;
    currentHangmanStage = 0;

    for (int i = 0; i < 26; i++) guessedLetters[i] = 0;
    guessedLetters[26] = '\0';

    const char* wordList[] = {
        "SIRFAISAL", "PHYSICS", "SIMULATION", "GRAVITY",
        "STICKMAN", "PROGRAM", "COMPUTER", "WINDOWS", "CONSOLE", "GAMING"
    };

    int wordCount = 10;
    strcpy_s(hangmanWord, wordList[rand() % wordCount]);

    wordLength = (int)strlen(hangmanWord);
    for (int i = 0; i < wordLength; i++) {
        guessedWord[i] = '_';
    }
    guessedWord[wordLength] = '\0';

    SpawnRagdoll(WIDTH / 2, HEIGHT / 2 - 5);

    for (int i = 0; i < pointCount; i++) {
        if (points[i].isRagdollPart == 1) {
            points[i].isLocked = 1;
        }
    }

    isSimulating = 1;
}

void ProcessHangmanGuess(char letter) {
    if (hangmanGameOver) return;

    letter = toupper(letter);

    // Check if already guessed
    for (int i = 0; i < 26; i++) {
        if (guessedLetters[i] == letter) return;
    }

    // Add to guessed letters
    for (int i = 0; i < 26; i++) {
        if (guessedLetters[i] == 0) {
            guessedLetters[i] = letter;
            break;
        }
    }

    int found = 0;
    for (int i = 0; i < wordLength; i++) {
        if (hangmanWord[i] == letter) {
            guessedWord[i] = letter;
            found = 1;
        }
    }

    if (!found) {
        wrongGuesses++;
        BreakRandomStick();
        currentHangmanStage = wrongGuesses;
    }

    int complete = 1;
    for (int i = 0; i < wordLength; i++) {
        if (guessedWord[i] == '_') {
            complete = 0;
            break;
        }
    }

    if (complete) {
        hangmanWon = 1;
        hangmanGameOver = 1;
        gameStats.coins += 50;  // Win bonus
        PlaySoundSuccess();
        SaveGame();
    }

    if (wrongGuesses >= maxWrongGuesses) {
        hangmanGameOver = 1;
        stickmanIntact = 0;
        PlaySoundFailure();
    }
}

void DrawHangmanUI() {
    char wordDisplay[100];
    sprintf_s(wordDisplay, 100, "WORD: %s", guessedWord);
    int len = (int)strlen(wordDisplay);
    int startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, 2, wordDisplay[i], COLOR_BRIGHT_CYAN);

    char lettersDisplay[100];
    sprintf_s(lettersDisplay, 100, "Guessed: %s", guessedLetters);
    len = (int)strlen(lettersDisplay);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, 4, lettersDisplay[i], COLOR_BRIGHT_YELLOW);

    char wrongDisplay[100];
    sprintf_s(wrongDisplay, 100, "Wrong guesses: %d/%d", wrongGuesses, maxWrongGuesses);
    len = (int)strlen(wrongDisplay);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, 5, wrongDisplay[i], (wrongGuesses >= maxWrongGuesses) ? COLOR_BRIGHT_RED : COLOR_WHITE);

    char title[] = "STICKMAN HANGMAN - Guess the word or stickman dies!";
    len = (int)strlen(title);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, 0, title[i], COLOR_BRIGHT_MAGENTA);

    char instr[] = "Type letters A-Z to guess. ESC to return to menu.";
    len = (int)strlen(instr);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, HEIGHT - 2, instr[i], COLOR_GRAY);

    char stage[100];
    const char* stageNames[] = { "Intact", "Hands", "Arms", "Body", "Legs", "Almost Dead", "DEAD!" };
    if (wrongGuesses < 7) {
        sprintf_s(stage, 100, "Stickman Status: %s", stageNames[wrongGuesses]);
        len = (int)strlen(stage);
        startX = (WIDTH - len) / 2;
        for (int i = 0; i < len; i++) PutChar(startX + i, 7, stage[i], (wrongGuesses >= 5) ? COLOR_BRIGHT_RED : (wrongGuesses >= 3) ? COLOR_BRIGHT_YELLOW : COLOR_BRIGHT_GREEN);
    }

    if (hangmanGameOver) {
        if (hangmanWon) {
            char winMsg[] = "CONGRATULATIONS! You saved the stickman! +50 Coins";
            len = (int)strlen(winMsg);
            startX = (WIDTH - len) / 2;
            for (int i = 0; i < len; i++) PutChar(startX + i, 9, winMsg[i], COLOR_BRIGHT_GREEN);

            SpawnSuccessParticles(WIDTH / 2, HEIGHT / 2 - 5);

            char restart[] = "Press R to play again, ESC for menu";
            len = (int)strlen(restart);
            startX = (WIDTH - len) / 2;
            for (int i = 0; i < len; i++) PutChar(startX + i, 10, restart[i], COLOR_BRIGHT_CYAN);
        }
        else {
            char loseMsg[100];
            sprintf_s(loseMsg, 100, "GAME OVER! The word was: %s", hangmanWord);
            len = (int)strlen(loseMsg);
            startX = (WIDTH - len) / 2;
            for (int i = 0; i < len; i++) PutChar(startX + i, 9, loseMsg[i], COLOR_BRIGHT_RED);

            char deathMsg[] = "The stickman has died! You lose.";
            len = (int)strlen(deathMsg);
            startX = (WIDTH - len) / 2;
            for (int i = 0; i < len; i++) PutChar(startX + i, 10, deathMsg[i], COLOR_BRIGHT_MAGENTA);

            char restart[] = "Press R to try again, ESC for menu";
            len = (int)strlen(restart);
            startX = (WIDTH - len) / 2;
            for (int i = 0; i < len; i++) PutChar(startX + i, 11, restart[i], COLOR_BRIGHT_CYAN);
        }
    }
}

//---------------------------------------------------------------------
// PHYSICS FUNCTIONS
//---------------------------------------------------------------------

void Explode(int x, int y) {
    float explosionRadius = EXPLOSION_RADIUS;
    float explosionPower = EXPLOSION_POWER;

    screenShake = 2.0f;

    SpawnExplosionParticles((float)x, (float)y);

    // Play sound only once
    PlaySoundExplosion();
    gameStats.explosionsTriggered++;

    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive == 0) continue;

        float dx = points[i].x - x;
        float dy = points[i].y - y;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < explosionRadius && distance > 0.1f) {
            float force = (explosionRadius - distance) / distance * explosionPower;
            points[i].oldX = points[i].oldX - dx * force;
            points[i].oldY = points[i].oldY - dy * force;

            if (points[i].isRagdollPart && distance < 5.0f) points[i].isLocked = 0;
        }
    }

    for (int b = 0; b < boxCount; b++) {
        if (boxes[b].isActive == 0) continue;
        if (boxes[b].isWall == 0) continue;

        float dist = GetDistance(boxes[b].x, boxes[b].y, (float)x, (float)y);

        if (dist < explosionRadius * 0.6f && boxes[b].height > 5) {
            boxes[b].isActive = 0;
        }
    }

    for (int s = 0; s < stickCount; s++) {
        if (sticks[s].active == 0) continue;
        if (sticks[s].isRagdollStick == 1) continue;

        float midX = (points[sticks[s].p1].x + points[sticks[s].p2].x) / 2.0f;
        float midY = (points[sticks[s].p1].y + points[sticks[s].p2].y) / 2.0f;

        float dist = GetDistance(midX, midY, (float)x, (float)y);

        if (dist < explosionRadius * 0.5f) {
            sticks[s].active = 0;
            SpawnBreakParticles(midX, midY);
        }
    }
}

void ClampPointToBox(int pointIndex, int boxIndex) {
    if (boxes[boxIndex].isSolid == 0) return;

    float halfW = boxes[boxIndex].width / 2.0f;
    float halfH = boxes[boxIndex].height / 2.0f;

    float left = boxes[boxIndex].x - halfW;
    float right = boxes[boxIndex].x + halfW;
    float top = boxes[boxIndex].y - halfH;
    float bottom = boxes[boxIndex].y + halfH;

    float px = points[pointIndex].x;
    float py = points[pointIndex].y;

    if (px > left && px < right && py > top && py < bottom) {
        float distLeft = px - left;
        float distRight = right - px;
        float distTop = py - top;
        float distBottom = bottom - py;

        float minDist = distLeft;
        int side = 0;

        if (distRight < minDist) { minDist = distRight; side = 1; }
        if (distTop < minDist) { minDist = distTop; side = 2; }
        if (distBottom < minDist) { minDist = distBottom; side = 3; }

        float velX = (points[pointIndex].x - points[pointIndex].oldX);
        float velY = (points[pointIndex].y - points[pointIndex].oldY);

        if (side == 0) {
            points[pointIndex].x = left - points[pointIndex].radius;
            points[pointIndex].oldX = points[pointIndex].x + velX * BOUNCE;
        }
        else if (side == 1) {
            points[pointIndex].x = right + points[pointIndex].radius;
            points[pointIndex].oldX = points[pointIndex].x + velX * BOUNCE;
        }
        else if (side == 2) {
            points[pointIndex].y = top - points[pointIndex].radius;
            points[pointIndex].oldY = points[pointIndex].y + velY * BOUNCE;
        }
        else {
            points[pointIndex].y = bottom + points[pointIndex].radius;
            points[pointIndex].oldY = points[pointIndex].y + velY * BOUNCE;
        }
    }
}

void ResolveBoxCollisions() {
    for (int b = 0; b < boxCount; b++) {
        if (boxes[b].isActive == 0) continue;

        for (int i = 0; i < pointCount; i++) {
            if (points[i].isActive == 0) continue;
            if (points[i].isLocked == 1) continue;

            ClampPointToBox(i, b);
        }
    }
}

int CheckRagdollInTarget(int targetIndex) {
    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive == 0) continue;
        if (points[i].isRagdollPart == 0) continue;

        float dist = GetDistance(points[i].x, points[i].y,
            targets[targetIndex].x, targets[targetIndex].y);
        if (dist < targets[targetIndex].radius) {
            return 1;
        }
    }
    return 0;
}

int CheckRagdollIntegrity() {
    for (int i = 0; i < stickCount; i++) {
        if (sticks[i].isRagdollStick == 1 && sticks[i].active == 0) {
            return 0;
        }
    }

    for (int i = 0; i < pointCount; i++) {
        if (points[i].isRagdollPart == 1) {
            if (points[i].isActive == 0) {
                return 0;
            }
            return 1;
        }
    }

    return 0;
}

void ClearWorld() {
    pointCount = 0;
    stickCount = 0;
    boxCount = 0;
    targetCount = 0;
    dragPoint = -1;
    ropeStartX = -1;
    ropeStartY = -1;
    ragdollBroken = 0;
    hangmanModeActive = 0;
    undoCount = 0;
    currentUndoIndex = 0;

    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = 0;
}

//---------------------------------------------------------------------
// MISSION FUNCTIONS - REDESIGNED LEVELS
//---------------------------------------------------------------------

void DrawMissionStartScreen(HANDLE hOut, int missionNum) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    char missionTitle[100];
    char missionDesc[200];
    char missionHint[200];

    switch (missionNum) {
    case 1:
        strcpy_s(missionTitle, "MISSION 1: TRAINING GROUNDS");
        strcpy_s(missionDesc, "Navigate the simple course. Use SPACE to play/pause, D to drag.");
        strcpy_s(missionHint, "HINT: Drag the stickman's head to move it around.");
        break;
    case 2:
        strcpy_s(missionTitle, "MISSION 2: THE MAZE");
        strcpy_s(missionDesc, "Find your way through the maze to reach the target.");
        strcpy_s(missionHint, "HINT: Use bombs strategically to clear paths.");
        break;
    case 3:
        strcpy_s(missionTitle, "MISSION 3: OBSTACLE COURSE");
        strcpy_s(missionDesc, "Navigate through moving platforms and obstacles.");
        strcpy_s(missionHint, "HINT: Time your movements with platform swings.");
        break;
    case 4:
        strcpy_s(missionTitle, "MISSION 4: PINBALL CHALLENGE");
        strcpy_s(missionDesc, "Use bumpers and flippers to reach the top target.");
        strcpy_s(missionHint, "HINT: Create momentum with bombs and ropes.");
        break;
    case 5:
        strcpy_s(missionTitle, "MISSION 5: FINAL BOSS");
        strcpy_s(missionDesc, "The ultimate challenge! Multiple stages to complete.");
        strcpy_s(missionHint, "HINT: You'll need all your skills for this one!");
        break;
    }

    int y = HEIGHT / 2 - 5;
    int len = strlen(missionTitle);
    int startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, y, missionTitle[i], COLOR_BRIGHT_YELLOW);

    len = strlen(missionDesc);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, y + 2, missionDesc[i], COLOR_BRIGHT_CYAN);

    len = strlen(missionHint);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, y + 4, missionHint[i], COLOR_GREEN);

    char reward[100];
    sprintf_s(reward, "REWARD: %d COINS", missionNum * 50);
    len = strlen(reward);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, y + 6, reward[i], COLOR_BRIGHT_MAGENTA);

    char pressKey[] = "Press ENTER to start mission";
    len = strlen(pressKey);
    startX = (WIDTH - len) / 2;
    for (int i = 0; i < len; i++) PutChar(startX + i, y + 8, pressKey[i], COLOR_WHITE);

    CHAR_INFO buffer[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i].Char.UnicodeChar = (WCHAR)screenBuf[i];
        buffer[i].Attributes = colorBuf[i];
    }
    COORD bufferSize = { (short)WIDTH, (short)HEIGHT };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };
    WriteConsoleOutput(hOut, buffer, bufferSize, bufferCoord, &writeRegion);
}

void InitMission(int missionNum) {
    ClearWorld();
    targetsReached = 0;
    missionComplete = 0;
    missionFailed = 0;
    missionTimer = 0;
    isSimulating = 1;

    // Reset all targets
    for (int i = 0; i < targetCount; i++) {
        targets[i].ragdollTouching = 0;
    }

    // NEW AND IMPROVED MISSIONS
    if (missionNum == 1) {
        // Mission 1: Basic Training - Simple ramp to target
        SpawnRagdoll(20, 35);

        // Create a gentle ramp
        for (int i = 0; i < 8; i++) {
            AddBox(30 + i * 8, 38 - i * 2, 8, 4, 1, 1);
        }

        AddTarget(100, 25, 8);
        missionTimeLimit = 30.0f;

        // Add coins for collection
        SpawnCoin(50, 32);
        SpawnCoin(70, 28);
        SpawnCoin(90, 24);
    }
    else if (missionNum == 2) {
        // Mission 2: The Maze - Navigate through walls
        SpawnRagdoll(20, 10);

        // Create maze walls
        AddBox(40, 30, 60, 5, 1, 1);
        AddBox(40, 40, 5, 20, 1, 1);
        AddBox(80, 50, 5, 20, 1, 1);
        AddBox(60, 75, 40, 5, 1, 1);

        AddTarget(100, 25, 8);
        missionTimeLimit = 45.0f;

        // Add bombs to help clear path
        SpawnBomb(75, 25);

        // Coins
        SpawnCoin(35, 15);
        SpawnCoin(85, 20);
        SpawnCoin(95, 30);
    }
    else if (missionNum == 3) {
        // Mission 3: Obstacle Course - Moving platforms and challenges
        SpawnRagdoll(15, 35);

        // Starting platform
        SpawnPlatform(30, 32, 15);

        // Swinging platform
        int swingAnchor = AddPoint(50, 10, 'O', 1, 1.0f, 0, COLOR_BRIGHT_CYAN, 0);
        int swingSeat = AddPoint(50, 25, '[', 0, 1.0f, 0, COLOR_BRIGHT_YELLOW, 0);
        AddStick(swingAnchor, swingSeat, 0);

        // Moving platform (pendulum)
        int moveAnchor = AddPoint(70, 10, 'O', 1, 1.0f, 0, COLOR_BRIGHT_CYAN, 0);
        int movePlatform = AddPoint(70, 25, '=', 0, 1.0f, 0, COLOR_BRIGHT_GREEN, 0);
        AddStick(moveAnchor, movePlatform, 0);

        // Final platform
        SpawnPlatform(90, 20, 15);

        AddTarget(111, 15, 8);
        missionTimeLimit = 60.0f;

        // Coins in tricky spots
        SpawnCoin(50, 20);
        SpawnCoin(70, 20);
        SpawnCoin(90, 15);
    }
    else if (missionNum == 4) {
        // Mission 4: Pinball Challenge - Bumpers and flippers
        SpawnRagdoll(30, 35);

        // Create pinball-like bumpers
        for (int i = 0; i < 4; i++) {
            int bumper = AddPoint(40 + i * 15, 25, 'O', 1, 3.0f, 0, COLOR_BRIGHT_MAGENTA, 0);
            // Make bumpers bouncy
            if (bumper >= 0) {
                points[bumper].radius = 3.0f;
            }
        }

        // Flippers
        int leftFlipper = AddPoint(30, 38, '/', 1, 1.0f, 0, COLOR_BRIGHT_YELLOW, 0);
        int rightFlipper = AddPoint(90, 38, '\\', 1, 1.0f, 0, COLOR_BRIGHT_YELLOW, 0);

        // Walls
        AddBox(20, 40, 100, 5, 1, 1);  // Bottom wall
        AddBox(10, 10, 5, 60, 1, 1);   // Left wall
        AddBox(105, 10, 5, 60, 1, 1);  // Right wall

        AddTarget(60, 10, 8);  // Top target
        missionTimeLimit = 50.0f;

        // Coins
        SpawnCoin(45, 20);
        SpawnCoin(60, 15);
        SpawnCoin(75, 20);
    }
    else if (missionNum == 5) {
        // Mission 5: Final Boss - Multi-stage challenge
        SpawnRagdoll(20, 10);

        // Stage 1: Climbing section
        for (int i = 0; i < 4; i++) {
            SpawnPlatform(30 + i * 15, 15 + i * 5, 12);
        }

        // Stage 2: Swinging section
        int bigSwing = AddPoint(85, 10, 'O', 1, 1.5f, 0, COLOR_BRIGHT_CYAN, 0);
        int swingSeat = AddPoint(85, 30, '[', 0, 1.5f, 0, COLOR_BRIGHT_YELLOW, 0);
        AddStick(bigSwing, swingSeat, 0);

        // Stage 3: Final platform with moving obstacle
        SpawnPlatform(100, 20, 15);

        // Moving obstacle
        int obstacle = AddPoint(100, 15, 'X', 0, 2.0f, 0, COLOR_BRIGHT_RED, 0);
        if (obstacle >= 0) {
            points[obstacle].oldX = points[obstacle].x - 5;
        }

        // Multiple targets for multi-stage completion
        AddTarget(40, 10, 6);   // First target
        AddTarget(70, 25, 6);   // Second target
        AddTarget(100, 15, 6);  // Final target

        missionTimeLimit = 75.0f;

        // Lots of coins
        for (int i = 0; i < 8; i++) {
            SpawnCoin(25 + i * 12, 12 + (i % 3) * 3);
        }
    }
}

void UpdateMissionWithStats(float deltaTime) {
    if (missionComplete == 1 || missionFailed == 1) return;

    missionTimer = missionTimer + deltaTime;

    // Check for coin collection
    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive && points[i].symbol == '$') {
            // Check if ragdoll touches coin
            for (int j = 0; j < pointCount; j++) {
                if (points[j].isRagdollPart && points[j].isActive) {
                    float dist = GetDistance(points[i].x, points[i].y,
                        points[j].x, points[j].y);
                    if (dist < 3.0f) {
                        points[i].isActive = 0;
                        gameStats.coins += 10;
                        SpawnCoinParticles(points[i].x, points[i].y);
                        PlaySoundCoin();
                        SaveGame();
                    }
                }
            }
        }
    }

    if (CheckRagdollIntegrity() == 0) {
        ragdollBroken = 1;
        missionFailed = 1;
        isSimulating = 0;
        return;
    }

    if (missionTimer >= missionTimeLimit) {
        missionFailed = 1;
        isSimulating = 0;
        return;
    }

    int totalTouched = 0;
    for (int i = 0; i < targetCount; i++) {
        if (targets[i].isActive == 0) continue;
        if (CheckRagdollInTarget(i) == 1) {
            if (targets[i].ragdollTouching == 0) {
                targets[i].ragdollTouching = 1;
                targetsReached++;
                SpawnSuccessParticles(targets[i].x, targets[i].y);
            }
            totalTouched++;
        }
    }

    if (totalTouched >= targetCount && targetCount > 0) {
        missionComplete = 1;
        isSimulating = 0;
        gameStats.missionsCompleted++;

        // Award coins based on mission number and time
        int baseReward = currentMission * 50;
        int timeBonus = (int)((missionTimeLimit - missionTimer) * 2);
        int totalReward = baseReward + timeBonus;

        gameStats.coins += totalReward;
        PlaySoundSuccess();
        SaveGame();
    }
}

//---------------------------------------------------------------------
// PHYSICS SIMULATION
//---------------------------------------------------------------------

void UpdatePhysics() {
    UpdateParticles();
    if (screenShake > 0) {
        screenShake -= SHAKE_DECAY;
        if (screenShake < 0) screenShake = 0;
    }

    // Update ragdoll head symbol based on velocity
    for (int i = 0; i < pointCount; i++) {
        if (points[i].isRagdollPart && points[i].isSpecialHead) {
            float velSq = (points[i].x - points[i].oldX) * (points[i].x - points[i].oldX) +
                (points[i].y - points[i].oldY) * (points[i].y - points[i].oldY);

            if (ragdollBroken || (hangmanModeActive && hangmanGameOver && !hangmanWon)) {
                points[i].symbol = 'X';
            }
            else if (velSq > 0.5f) {
                // Keep the shop head symbol
                points[i].symbol = shopItems[currentHeadIndex].symbol;
            }
            else {
                points[i].symbol = shopItems[currentHeadIndex].symbol;
            }
        }
    }

    // Verlet integration
    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive == 0) continue;
        if (points[i].isLocked == 1) continue;

        float velX = (points[i].x - points[i].oldX) * FRICTION;
        float velY = (points[i].y - points[i].oldY) * FRICTION;

        points[i].oldX = points[i].x;
        points[i].oldY = points[i].y;

        points[i].x = points[i].x + velX;
        points[i].y = points[i].y + velY + GRAVITY;

        // Boundary collision
        if (points[i].y > HEIGHT - 1 - points[i].radius) {
            points[i].y = HEIGHT - 1 - points[i].radius;
            points[i].oldY = points[i].y + velY * BOUNCE;
            points[i].oldX = points[i].x - velX * 0.8f;

            if (points[i].symbol == '@') {
                Explode((int)points[i].x, (int)points[i].y);
                points[i].isActive = 0;
            }
        }

        if (points[i].x < points[i].radius) {
            points[i].x = points[i].radius;
            points[i].oldX = points[i].x + velX * BOUNCE;
        }

        if (points[i].x > WIDTH - 1 - points[i].radius) {
            points[i].x = WIDTH - 1 - points[i].radius;
            points[i].oldX = points[i].x + velX * BOUNCE;
        }

        if (points[i].y < points[i].radius) {
            points[i].y = points[i].radius;
            points[i].oldY = points[i].y + velY * BOUNCE;
        }
    }

    ResolveBoxCollisions();

    // Track stick breaks for feedback
    int sticksBeforeUpdate = 0;
    for (int i = 0; i < stickCount; i++) {
        if (sticks[i].active) sticksBeforeUpdate++;
    }

    // Constraint solving
    for (int iteration = 0; iteration < CONSTRAINT_ITERATIONS; iteration++) {
        for (int s = 0; s < stickCount; s++) {
            if (sticks[s].active == 0) continue;

            int p1 = sticks[s].p1;
            int p2 = sticks[s].p2;

            if (points[p1].isActive == 0) continue;
            if (points[p2].isActive == 0) continue;

            float dx = points[p2].x - points[p1].x;
            float dy = points[p2].y - points[p1].y;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance < 0.001f) continue;

            if (distance > sticks[s].length * STICK_BREAK_FACTOR) {
                sticks[s].active = 0;
                SpawnBreakParticles((points[p1].x + points[p2].x) / 2, (points[p1].y + points[p2].y) / 2);
                continue;
            }

            float difference = (sticks[s].length - distance) / distance;
            float offsetX = dx * difference * 0.5f;
            float offsetY = dy * difference * 0.5f;

            if (points[p1].isLocked == 0) {
                points[p1].x = points[p1].x - offsetX;
                points[p1].y = points[p1].y - offsetY;
            }
            if (points[p2].isLocked == 0) {
                points[p2].x = points[p2].x + offsetX;
                points[p2].y = points[p2].y + offsetY;
            }
        }

        ResolveBoxCollisions();
    }

    // After constraint solving, check for new breaks
    int sticksAfterUpdate = 0;
    for (int i = 0; i < stickCount; i++) {
        if (sticks[i].active) sticksAfterUpdate++;
    }

    if (sticksAfterUpdate < sticksBeforeUpdate) {
        PlaySoundBreak();
        gameStats.sticksBreached++;
        if (screenShake < 2.0f) screenShake += 0.5f;
    }

    // Point-to-point collision
    for (int i = 0; i < pointCount; i++) {
        for (int j = i + 1; j < pointCount; j++) {
            if (points[i].isActive == 0) continue;
            if (points[j].isActive == 0) continue;

            float dx = points[i].x - points[j].x;
            float dy = points[i].y - points[j].y;
            float distance = sqrtf(dx * dx + dy * dy);
            float minDistance = points[i].radius + points[j].radius;

            if (distance < minDistance && distance > 0.001f) {
                float overlap = minDistance - distance;
                float pushX = (dx / distance) * overlap * 0.5f;
                float pushY = (dy / distance) * overlap * 0.5f;

                if (points[i].isLocked == 0) {
                    points[i].x = points[i].x + pushX;
                    points[i].y = points[i].y + pushY;
                }
                if (points[j].isLocked == 0) {
                    points[j].x = points[j].x - pushX;
                    points[j].y = points[j].y - pushY;
                }
            }
        }
    }
}

//---------------------------------------------------------------------
// SCREEN DRAWING
//---------------------------------------------------------------------

void DrawScreen(HANDLE hOut) {
    // Clear screen buffers
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    // Apply screen shake
    int shakeX = 0, shakeY = 0;
    if (screenShake > 0) {
        shakeX = (rand() % 3 - 1) * (int)screenShake;
        shakeY = (rand() % 3 - 1) * (int)screenShake;
    }

    // Draw particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            PutChar((int)particles[i].x + shakeX, (int)particles[i].y + shakeY,
                particles[i].symbol, particles[i].color);
        }
    }

    // Draw targets in mission mode
    if (currentMode == 2) {
        DrawAnimatedTargets();
    }

    // Draw boxes
    for (int i = 0; i < boxCount; i++) {
        if (boxes[i].isActive) {
            int halfW = (int)(boxes[i].width / 2.0f);
            int halfH = (int)(boxes[i].height / 2.0f);
            int left = (int)boxes[i].x - halfW + shakeX;
            int right = (int)boxes[i].x + halfW + shakeX;
            int top = (int)boxes[i].y - halfH + shakeY;
            int bottom = (int)boxes[i].y + halfH + shakeY;
            int col = (boxes[i].isWall) ? COLOR_GRAY : COLOR_BRIGHT_BLUE;

            for (int y = top; y <= bottom; y++) {
                for (int x = left; x <= right; x++) {
                    if (x == left || x == right || y == top || y == bottom) {
                        PutChar(x, y, '#', col);
                    }
                    else if (boxes[i].isSolid) {
                        PutChar(x, y, ':', col);
                    }
                }
            }
        }
    }

    // Draw sticks
    for (int i = 0; i < stickCount; i++) {
        if (sticks[i].active == 0) continue;
        if (points[sticks[i].p1].isActive == 0) continue;
        if (points[sticks[i].p2].isActive == 0) continue;

        DrawLine(
            (int)(points[sticks[i].p1].x + 0.5f) + shakeX,
            (int)(points[sticks[i].p1].y + 0.5f) + shakeY,
            (int)(points[sticks[i].p2].x + 0.5f) + shakeX,
            (int)(points[sticks[i].p2].y + 0.5f) + shakeY,
            '-',
            COLOR_WHITE
        );
    }

    // Draw points with effects
    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive == 0) continue;
        DrawPointWithEffects(&points[i], shakeX, shakeY);
    }

    // Draw UI elements
    DrawToolSelection();
    DrawStatusBar();
    DrawDebugInfo();
    DrawHelpOverlay();

    // Mode-specific UI
    if (currentMode == 1) {
        // Sandbox mode
        DrawEnhancedCursor();
        char uiText[] = "SANDBOX | 1:Ragdoll 2:Box 3:Bomb 4:Rope 5:Platform | SPACE:Play R:Reset D:Drag";
        int uiTextLen = (int)strlen(uiText);
        for (int i = 0; i < uiTextLen && i < WIDTH; i++) {
            PutChar(i, 0, uiText[i], COLOR_BRIGHT_CYAN);
        }

        if (ropeStartX >= 0) {
            DrawLine(ropeStartX, ropeStartY, curX, curY, ':', COLOR_BRIGHT_YELLOW);
        }
    }
    else if (currentMode == 2) {
        // Mission mode
        DrawEnhancedCursor();
        char uiText[256];
        float timeLeft = missionTimeLimit - missionTimer;
        if (timeLeft < 0) timeLeft = 0;
        sprintf_s(uiText, 256, "MISSION %d/5 | Time: %.1fs | Targets: %d/%d | D:Drag R:Restart ESC:Menu",
            currentMission, timeLeft, targetsReached, targetCount);
        int uiTextLen = (int)strlen(uiText);
        for (int i = 0; i < uiTextLen && i < WIDTH; i++) {
            PutChar(i, 0, uiText[i], COLOR_BRIGHT_YELLOW);
        }
    }
    else if (currentMode == 3) {
        // Hangman mode
        DrawHangmanUI();
    }
    else if (currentMode == 4) {
        // Shop mode
        DrawShop();
    }

    // Output to console
    CHAR_INFO buffer[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i].Char.UnicodeChar = (WCHAR)screenBuf[i];
        buffer[i].Attributes = colorBuf[i];
    }

    COORD bufferSize = { (short)WIDTH, (short)HEIGHT };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };
    WriteConsoleOutput(hOut, buffer, bufferSize, bufferCoord, &writeRegion);
}

//---------------------------------------------------------------------
// MENU FUNCTIONS
//---------------------------------------------------------------------

void ShowMainMenu(HANDLE hOut) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    // Draw title
    char title1[] = "  ____    _    ____ ____   ___  _     _       ____  _   ___   _______ ___ ____ ____  ";
    char title2[] = " |  _ \\  / \\  / ___|  _ \\ / _ \\| |   | |     |  _ \\| | | \\ \\ / / ____|_ _/ ___/ ___| ";
    char title3[] = " | |_) |/ _ \\| |  _| | | | | | | |   | |     | |_) | |_| |\\ V /|  _|  | | |   \\___ \\ ";
    char title4[] = " |  _ </ ___ \\ |_| | |_| | |_| | |___| |___  |  __/|  _  | | | | |___ | | |___ ___) |";
    char title5[] = " |_| \\_\\_/   \\_\\____|____/ \\___/|_____|_____| |_|   |_| |_| |_| |_____|___\\____|____/ ";

    int startY = 8;
    int len = (int)strlen(title1);
    int startX = (WIDTH - len) / 2;
    for (int j = 0; j < len && j < WIDTH; j++) PutChar(startX + j, startY, title1[j], COLOR_BRIGHT_CYAN);

    len = (int)strlen(title2);
    startX = (WIDTH - len) / 2;
    for (int j = 0; j < len && j < WIDTH; j++) PutChar(startX + j, startY + 1, title2[j], COLOR_BRIGHT_CYAN);

    len = (int)strlen(title3);
    startX = (WIDTH - len) / 2;
    for (int j = 0; j < len && j < WIDTH; j++) PutChar(startX + j, startY + 2, title3[j], COLOR_BRIGHT_CYAN);

    len = (int)strlen(title4);
    startX = (WIDTH - len) / 2;
    for (int j = 0; j < len && j < WIDTH; j++) PutChar(startX + j, startY + 3, title4[j], COLOR_BRIGHT_CYAN);

    len = (int)strlen(title5);
    startX = (WIDTH - len) / 2;
    for (int j = 0; j < len && j < WIDTH; j++) PutChar(startX + j, startY + 4, title5[j], COLOR_BRIGHT_CYAN);

    char subtitle[] = "ASCII Physics Game | by Huzaifa & Umar";
    int subLen = (int)strlen(subtitle);
    int subX = (WIDTH - subLen) / 2;
    for (int i = 0; i < subLen; i++) PutChar(subX + i, startY + 7, subtitle[i], COLOR_BRIGHT_YELLOW);

    // Menu options
    char opt1[] = "1. SANDBOX MODE - Free play with all tools";
    char opt2[] = "2. MISSION MODE - Complete 5 challenging levels";
    char opt3[] = "3. STICKMAN HANGMAN - Guess words or stickman dies!";
    char opt4[] = "4. HEAD SHOP - Buy new stickman heads";
    char opt5[] = "5. EXIT GAME";

    int optionY = 22;

    len = (int)strlen(opt1);
    int optX = (WIDTH - len) / 2;
    int color1 = (menuSelection == 0) ? COLOR_BRIGHT_GREEN : COLOR_WHITE;
    for (int j = 0; j < len; j++) PutChar(optX + j, optionY, opt1[j], color1);
    if (menuSelection == 0) {
        PutChar(optX - 3, optionY, '>', COLOR_BRIGHT_GREEN);
        PutChar(optX + len + 2, optionY, '<', COLOR_BRIGHT_GREEN);
    }

    len = (int)strlen(opt2);
    optX = (WIDTH - len) / 2;
    int color2 = (menuSelection == 1) ? COLOR_BRIGHT_GREEN : COLOR_WHITE;
    for (int j = 0; j < len; j++) PutChar(optX + j, optionY + 2, opt2[j], color2);
    if (menuSelection == 1) {
        PutChar(optX - 3, optionY + 2, '>', COLOR_BRIGHT_GREEN);
        PutChar(optX + len + 2, optionY + 2, '<', COLOR_BRIGHT_GREEN);
    }

    len = (int)strlen(opt3);
    optX = (WIDTH - len) / 2;
    int color3 = (menuSelection == 2) ? COLOR_BRIGHT_GREEN : COLOR_WHITE;
    for (int j = 0; j < len; j++) PutChar(optX + j, optionY + 4, opt3[j], color3);
    if (menuSelection == 2) {
        PutChar(optX - 3, optionY + 4, '>', COLOR_BRIGHT_GREEN);
        PutChar(optX + len + 2, optionY + 4, '<', COLOR_BRIGHT_GREEN);
    }

    len = (int)strlen(opt4);
    optX = (WIDTH - len) / 2;
    int color4 = (menuSelection == 3) ? COLOR_BRIGHT_GREEN : COLOR_WHITE;
    for (int j = 0; j < len; j++) PutChar(optX + j, optionY + 6, opt4[j], color4);
    if (menuSelection == 3) {
        PutChar(optX - 3, optionY + 6, '>', COLOR_BRIGHT_GREEN);
        PutChar(optX + len + 2, optionY + 6, '<', COLOR_BRIGHT_GREEN);
    }

    len = (int)strlen(opt5);
    optX = (WIDTH - len) / 2;
    int color5 = (menuSelection == 4) ? COLOR_BRIGHT_GREEN : COLOR_WHITE;
    for (int j = 0; j < len; j++) PutChar(optX + j, optionY + 8, opt5[j], color5);
    if (menuSelection == 4) {
        PutChar(optX - 3, optionY + 8, '>', COLOR_BRIGHT_GREEN);
        PutChar(optX + len + 2, optionY + 8, '<', COLOR_BRIGHT_GREEN);
    }

    // Stats and controls
    char stats[100];
    sprintf_s(stats, 100, "Coins: %d | Missions Completed: %d/%d",
        gameStats.coins, gameStats.missionsCompleted, maxMissions);
    int statsLen = strlen(stats);
    int statsX = (WIDTH - statsLen) / 2;
    for (int i = 0; i < statsLen; i++) {
        PutChar(statsX + i, optionY + 11, stats[i], COLOR_BRIGHT_YELLOW);
    }

    char controls[] = "Use UP/DOWN arrows, ENTER to select | H=Help S=Sound F1=Debug";
    int ctrlLen = (int)strlen(controls);
    int ctrlX = (WIDTH - ctrlLen) / 2;
    for (int i = 0; i < ctrlLen; i++) PutChar(ctrlX + i, HEIGHT - 3, controls[i], COLOR_GRAY);

    // Output to console
    CHAR_INFO buffer[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i].Char.UnicodeChar = (WCHAR)screenBuf[i];
        buffer[i].Attributes = colorBuf[i];
    }
    COORD bufferSize = { (short)WIDTH, (short)HEIGHT };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };
    WriteConsoleOutput(hOut, buffer, bufferSize, bufferCoord, &writeRegion);
}

void ShowMissionComplete(HANDLE hOut) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    char msg1[] = "=== MISSION COMPLETE! ===";
    char msg2[100];
    sprintf_s(msg2, 100, "Mission %d cleared in %.1f seconds!", currentMission, missionTimer);

    // Calculate reward
    int baseReward = currentMission * 50;
    int timeBonus = (int)((missionTimeLimit - missionTimer) * 2);
    int totalReward = baseReward + timeBonus;

    char msg3[100];
    sprintf_s(msg3, 100, "Reward: %d + %d (time bonus) = %d coins!", baseReward, timeBonus, totalReward);

    char msg4[100];
    if (currentMission < maxMissions) {
        sprintf_s(msg4, 100, "Press ENTER for Mission %d", currentMission + 1);
    }
    else {
        sprintf_s(msg4, 100, "ALL MISSIONS COMPLETE! YOU ARE A CHAMPION!");
    }

    char msg5[] = "Press ESC for menu";

    int y = HEIGHT / 2 - 3;
    int len1 = (int)strlen(msg1);
    for (int i = 0; i < len1; i++) PutChar((WIDTH - len1) / 2 + i, y, msg1[i], COLOR_BRIGHT_GREEN);

    int len2 = (int)strlen(msg2);
    for (int i = 0; i < len2; i++) PutChar((WIDTH - len2) / 2 + i, y + 2, msg2[i], COLOR_BRIGHT_YELLOW);

    int len3 = (int)strlen(msg3);
    for (int i = 0; i < len3; i++) PutChar((WIDTH - len3) / 2 + i, y + 3, msg3[i], COLOR_BRIGHT_MAGENTA);

    int len4 = (int)strlen(msg4);
    for (int i = 0; i < len4; i++) PutChar((WIDTH - len4) / 2 + i, y + 5, msg4[i], COLOR_BRIGHT_CYAN);

    int len5 = (int)strlen(msg5);
    for (int i = 0; i < len5; i++) PutChar((WIDTH - len5) / 2 + i, y + 6, msg5[i], COLOR_WHITE);

    CHAR_INFO buffer[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i].Char.UnicodeChar = (WCHAR)screenBuf[i];
        buffer[i].Attributes = colorBuf[i];
    }
    COORD bufferSize = { (short)WIDTH, (short)HEIGHT };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };
    WriteConsoleOutput(hOut, buffer, bufferSize, bufferCoord, &writeRegion);
}

void ShowMissionFailed(HANDLE hOut) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        screenBuf[i] = ' ';
        colorBuf[i] = COLOR_WHITE;
    }

    char msg1[] = "=== MISSION FAILED! ===";
    char msg2[100];
    if (ragdollBroken == 1) {
        sprintf_s(msg2, 100, "Ragdoll Destroyed! Keep it intact!");
    }
    else {
        sprintf_s(msg2, 100, "Time's Up! Mission %d Failed!", currentMission);
    }

    char msg3[] = "Press R to retry";
    char msg4[] = "Press ESC for menu";

    int y = HEIGHT / 2 - 2;
    int len1 = (int)strlen(msg1);
    for (int i = 0; i < len1; i++) PutChar((WIDTH - len1) / 2 + i, y, msg1[i], COLOR_BRIGHT_RED);
    int len2 = (int)strlen(msg2);
    for (int i = 0; i < len2; i++) PutChar((WIDTH - len2) / 2 + i, y + 2, msg2[i], COLOR_BRIGHT_YELLOW);
    int len3 = (int)strlen(msg3);
    for (int i = 0; i < len3; i++) PutChar((WIDTH - len3) / 2 + i, y + 4, msg3[i], COLOR_BRIGHT_CYAN);
    int len4 = (int)strlen(msg4);
    for (int i = 0; i < len4; i++) PutChar((WIDTH - len4) / 2 + i, y + 5, msg4[i], COLOR_WHITE);

    CHAR_INFO buffer[WIDTH * HEIGHT];
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i].Char.UnicodeChar = (WCHAR)screenBuf[i];
        buffer[i].Attributes = colorBuf[i];
    }
    COORD bufferSize = { (short)WIDTH, (short)HEIGHT };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (short)(WIDTH - 1), (short)(HEIGHT - 1) };
    WriteConsoleOutput(hOut, buffer, bufferSize, bufferCoord, &writeRegion);
}

//---------------------------------------------------------------------
// UTILITY FUNCTIONS
//---------------------------------------------------------------------

int FindNearestPoint(int x, int y, float maxDist) {
    int bestPoint = -1;
    float bestDistance = maxDist;

    for (int i = 0; i < pointCount; i++) {
        if (points[i].isActive == 0) continue;

        float distance = GetDistance(points[i].x, points[i].y, (float)x, (float)y);

        if (distance < bestDistance) {
            bestDistance = distance;
            bestPoint = i;
        }
    }

    return bestPoint;
}

//---------------------------------------------------------------------
// MAIN FUNCTION
//---------------------------------------------------------------------

int main() {
    // Console setup
    SetConsoleCP(437);
    SetConsoleOutputCP(437);

    HANDLE hOut = CreateConsoleScreenBuffer(GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hOut);

    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(hOut, &cursorInfo);

    system("title ASCII Physics Game");

    // Initialize systems
    InitInputManager();
    InitSoundManager();
    InitShop();

    // Initialize game stats
    gameStats.objectsSpawned = 0;
    gameStats.explosionsTriggered = 0;
    gameStats.ragdollsCreated = 0;
    gameStats.sticksBreached = 0;
    gameStats.totalPlayTime = 0.0f;
    gameStats.missionsCompleted = 0;
    gameStats.coins = 100;  // Starting coins

    // Load saved game
    LoadGame();

    // Game loop variables
    int frameTime = 33;
    DWORD lastTime = GetTickCount();
    srand(time(NULL));

    int showMissionStart = 0;

    // Main game loop
    while (1) {
        DWORD startTime = GetTickCount();
        float deltaTime = (startTime - lastTime) / 1000.0f;
        lastTime = startTime;

        // Update game time and stats
        gameTime += deltaTime;
        gameStats.totalPlayTime += deltaTime;

        // Update input manager
        UpdateInputManager();

        // Global hotkeys - disabled in active Hangman gameplay
        if (currentMode != 3 || hangmanGameOver) {
            if (IsKeyPressed('H')) {
                showHelp = !showHelp;
                PlaySoundClick();
            }

            if (IsKeyPressed('S')) {
                ToggleSound();
                PlaySoundClick();
            }
        }

        // Debug mode - always available except during active Hangman
        if (currentMode != 3 || hangmanGameOver) {
            if (IsKeyPressed(VK_F1)) {
                debugMode = !debugMode;
                PlaySoundClick();
            }
        }

        // Handle ESC key (always works)
        if (IsKeyPressed(VK_ESCAPE)) {
            if (showHelp) {
                showHelp = 0;
            }
            else if (currentMode == 0) {
                break;
            }
            else {
                currentMode = 0;
                Sleep(200);
            }
        }

        // Mode-specific handling
        if (currentMode == 0) {
            // Main Menu
            ShowMainMenu(hOut);

            if (IsKeyPressed(VK_UP)) {
                menuSelection--;
                if (menuSelection < 0) menuSelection = 4;
                PlaySoundClick();
                Sleep(100);
            }
            if (IsKeyPressed(VK_DOWN)) {
                menuSelection++;
                if (menuSelection > 4) menuSelection = 0;
                PlaySoundClick();
                Sleep(100);
            }

            if (IsKeyPressed(VK_RETURN)) {
                if (menuSelection == 0) {
                    // Sandbox Mode
                    currentMode = 1;
                    ClearWorld();
                    SpawnRagdoll(WIDTH / 2, 10);
                    isSimulating = 0;
                    PlaySoundClick();
                }
                else if (menuSelection == 1) {
                    // Mission Mode - show mission start screen
                    currentMode = 2;
                    currentMission = 1;
                    showMissionStart = 1;
                    PlaySoundClick();
                }
                else if (menuSelection == 2) {
                    // Hangman Mode
                    currentMode = 3;
                    InitHangmanMode();
                    PlaySoundClick();
                }
                else if (menuSelection == 3) {
                    // Shop Mode
                    currentMode = 4;
                    shopSelection = 0;
                    shopPage = 0;
                    PlaySoundClick();
                }
                else if (menuSelection == 4) {
                    // Exit Game
                    SaveGame();  // Save before exiting
                    break;
                }
                Sleep(200);
            }
        }
        else if (currentMode == 2) {
            // Mission Mode
            if (showMissionStart) {
                DrawMissionStartScreen(hOut, currentMission);
                if (IsKeyPressed(VK_RETURN)) {
                    showMissionStart = 0;
                    InitMission(currentMission);
                    PlaySoundClick();
                }
            }
            else if (missionComplete == 1) {
                ShowMissionComplete(hOut);

                if (IsKeyPressed(VK_RETURN)) {
                    currentMission++;
                    if (currentMission > maxMissions) {
                        currentMode = 0;
                        currentMission = 1;
                    }
                    else {
                        showMissionStart = 1;
                    }
                    PlaySoundClick();
                    Sleep(200);
                }
            }
            else if (missionFailed == 1) {
                ShowMissionFailed(hOut);

                if (IsKeyPressed('R')) {
                    showMissionStart = 1;
                    PlaySoundClick();
                    Sleep(200);
                }
            }
            else {
                // Active mission gameplay
                if (IsKeyPressed(VK_SPACE)) {
                    isSimulating = (isSimulating == 1) ? 0 : 1;
                    PlaySoundClick();
                    Sleep(100);
                }

                if (IsKeyPressed('R')) {
                    showMissionStart = 1;
                    PlaySoundClick();
                    Sleep(100);
                }

                if (IsKeyPressed('D')) {
                    dragMode = (dragMode == 1) ? 0 : 1;
                    dragPoint = -1;
                    PlaySoundDrag();
                    Sleep(100);
                }

                UpdateCursor();

                if (IsKeyPressed(VK_RETURN) && dragMode == 1) {
                    int nearPoint = FindNearestPoint(curX, curY, 5.0f);
                    if (nearPoint >= 0) {
                        if (points[nearPoint].isLocked == 1) {
                            points[nearPoint].isLocked = 0;
                            dragPoint = -1;
                        }
                        else {
                            points[nearPoint].isLocked = 1;
                            dragPoint = nearPoint;
                        }
                        PlaySoundClick();
                    }
                    Sleep(100);
                }

                if (dragPoint >= 0 && points[dragPoint].isLocked == 1) {
                    float targetX = (float)curX;
                    float targetY = (float)curY;
                    points[dragPoint].x = points[dragPoint].x + (targetX - points[dragPoint].x) * DRAG_SMOOTHNESS;
                    points[dragPoint].y = points[dragPoint].y + (targetY - points[dragPoint].y) * DRAG_SMOOTHNESS;
                    points[dragPoint].oldX = points[dragPoint].x;
                    points[dragPoint].oldY = points[dragPoint].y;
                }

                if (isSimulating == 1) {
                    UpdatePhysics();
                    UpdateMissionWithStats(deltaTime);
                }

                DrawScreen(hOut);
            }
        }
        else if (currentMode == 3) {
            // Hangman Mode
            if (hangmanGameOver == 0) {
                // Process letter guesses
                for (int key = 'A'; key <= 'Z'; key++) {
                    if (IsKeyPressed(key)) {
                        ProcessHangmanGuess((char)key);
                        Sleep(100);
                    }
                }
                for (int key = 'a'; key <= 'z'; key++) {
                    if (IsKeyPressed(key)) {
                        ProcessHangmanGuess((char)key);
                        Sleep(100);
                    }
                }

                // Space toggles simulation
                if (IsKeyPressed(VK_SPACE)) {
                    isSimulating = (isSimulating == 1) ? 0 : 1;
                    PlaySoundClick();
                    Sleep(100);
                }
            }
            else {
                // Game over state
                if (IsKeyPressed('R')) {
                    InitHangmanMode();
                    PlaySoundClick();
                    Sleep(200);
                }
            }

            if (isSimulating == 1) {
                UpdatePhysics();
            }

            DrawScreen(hOut);
        }
        else if (currentMode == 4) {
            // Shop Mode
            HandleShopInput();
            DrawScreen(hOut);
        }
        else {
            // Sandbox Mode (currentMode == 1)
            if (IsKeyPressed(VK_SPACE)) {
                isSimulating = (isSimulating == 1) ? 0 : 1;
                PlaySoundClick();
                Sleep(100);
            }

            if (IsKeyPressed('R')) {
                ClearWorld();
                PlaySoundClick();
                Sleep(100);
            }

            if (IsKeyPressed('D')) {
                dragMode = (dragMode == 1) ? 0 : 1;
                dragPoint = -1;
                PlaySoundDrag();
                Sleep(100);
            }

            if (IsKeyPressed('U')) {
                Undo();
                PlaySoundClick();
                Sleep(100);
            }

            // Tool selection
            if (IsKeyPressed('1')) { currentTool = 1; PlaySoundClick(); Sleep(100); }
            if (IsKeyPressed('2')) { currentTool = 2; PlaySoundClick(); Sleep(100); }
            if (IsKeyPressed('3')) { currentTool = 3; PlaySoundClick(); Sleep(100); }
            if (IsKeyPressed('4')) { currentTool = 4; PlaySoundClick(); Sleep(100); }
            if (IsKeyPressed('5')) { currentTool = 5; PlaySoundClick(); Sleep(100); }

            UpdateCursor();

            if (IsKeyPressed(VK_RETURN)) {
                if (dragMode == 0) {
                    if (currentTool == 4) {
                        // Rope mode
                        if (ropeStartX < 0) {
                            ropeStartX = curX;
                            ropeStartY = curY;
                            PlaySoundClick();
                        }
                        else {
                            SpawnRope(ropeStartX, ropeStartY, curX, curY);
                            ropeStartX = -1;
                            ropeStartY = -1;
                        }
                    }
                    else {
                        // Other tools
                        if (currentTool == 1) SpawnRagdoll(curX, curY);
                        else if (currentTool == 2) SpawnMovableBox(curX, curY);
                        else if (currentTool == 3) SpawnBomb(curX, curY);
                        else if (currentTool == 5) SpawnPlatform(curX, curY, 15);
                    }
                }
                else if (dragMode == 1) {
                    int nearPoint = FindNearestPoint(curX, curY, 5.0f);
                    if (nearPoint >= 0) {
                        if (points[nearPoint].isLocked == 1) {
                            points[nearPoint].isLocked = 0;
                            dragPoint = -1;
                        }
                        else {
                            points[nearPoint].isLocked = 1;
                            dragPoint = nearPoint;
                        }
                        PlaySoundClick();
                    }
                }
                Sleep(100);
            }

            if (dragPoint >= 0 && points[dragPoint].isLocked == 1) {
                float targetX = (float)curX;
                float targetY = (float)curY;
                points[dragPoint].x = points[dragPoint].x + (targetX - points[dragPoint].x) * DRAG_SMOOTHNESS;
                points[dragPoint].y = points[dragPoint].y + (targetY - points[dragPoint].y) * DRAG_SMOOTHNESS;
                points[dragPoint].oldX = points[dragPoint].x;
                points[dragPoint].oldY = points[dragPoint].y;
            }

            if (isSimulating == 1) {
                UpdatePhysics();
            }

            DrawScreen(hOut);
        }

        // Frame rate control
        DWORD elapsed = GetTickCount() - startTime;
        if (elapsed < frameTime) {
            Sleep(frameTime - elapsed);
        }
    }

    SaveGame();
    SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
    return 0;
}