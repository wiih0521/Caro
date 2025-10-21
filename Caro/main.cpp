#pragma warning(disable:4996) 
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>      // Dùng cho Save/Load
#include <string>
#include <vector>

//--- Hằng số (Đã cập nhật cho SFML) ---
const int BOARD_SIZE = 12;
const int CELL_SIZE = 50;           // Kích thước mỗi ô (pixel)
const int BOARD_OFFSET_X = 50;      // Lề trái
const int BOARD_OFFSET_Y = 100;     // Lề trên
const int WINDOW_WIDTH = BOARD_OFFSET_X * 2 + BOARD_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = BOARD_OFFSET_Y * 1.5 + BOARD_SIZE * CELL_SIZE;

//--- Kiểu dữ liệu (Đã đơn giản hóa) ---
struct _POINT {
    int c; // 0 = rỗng, -1 = X, 1 = O
};

//--- Biến toàn cục ---
_POINT _A[BOARD_SIZE][BOARD_SIZE];
bool _TURN = true;

// Trạng thái SFML
sf::RenderWindow window;
sf::Texture texX;
sf::Texture texO;
sf::Font font;

// Trạng thái Game
enum GameState { MENU, PLAYING, GAME_OVER };
GameState currentState = MENU;
int selectorRow = 0;    // Thay thế cho _X
int selectorCol = 0;    // Thay thế cho _Y
int menuSelection = 0;  // 0 = New Game, 1 = Load Game, 2 = Exit
std::string gameOverMessage = "";

//--- Khai báo các hàm ---

// Hàm Model (Logic game)
void ResetData();
int CheckBoard(int pRow, int pCol);
int TestBoard();
int CountHorizontal(int pRow, int pCol, int pMark);
int CountVertical(int pRow, int pCol, int pMark);
int CountDiagonal1(int pRow, int pCol, int pMark);
int CountDiagonal2(int pRow, int pCol, int pMark);
void SaveGame();
bool LoadGame();

// Hàm View & Control (SFML)
bool loadAssets();
sf::Texture createXTexture();
sf::Texture createOTexture();
void drawText(std::string str, int x, int y, int size, sf::Color color, bool center = false);
void renderMenu();
void renderGame();
void renderGameOver();
void handleMenuEvent(sf::Event event);
void handleGameEvent(sf::Event event);
void handleGameOverEvent(sf::Event event);


//--- Hàm Main (Vòng lặp SFML) ---
int main()
{
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Game Caro SFML (Tu Do Gia Huy)");
    window.setFramerateLimit(60);

    if (!loadAssets()) {
        std::cout << "Loi: Khong the tai assets (font hoac tao texture)!" << std::endl;
        return -1;
    }

    while (window.isOpen())
    {
        // 1. Xử lý sự kiện (Input)
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Chuyển sự kiện đến đúng hàm xử lý theo trạng thái
            if (currentState == MENU) handleMenuEvent(event);
            else if (currentState == PLAYING) handleGameEvent(event);
            else if (currentState == GAME_OVER) handleGameOverEvent(event);
        }

        // 2. Cập nhật (Update) - (Game này không cần update liên tục)

        // 3. Vẽ (Render)
        window.clear(sf::Color(245, 245, 220)); // Màu be (Beige)

        // Vẽ theo trạng thái
        if (currentState == MENU) renderMenu();
        else if (currentState == PLAYING) renderGame();
        else if (currentState == GAME_OVER) renderGameOver();

        window.display();
    }

    return 0;
}

//--- ĐỊNH NGHĨA CÁC HÀM SFML (VIEW & CONTROL) ---

/**
 * Tải font và tạo texture X, O
 */
bool loadAssets() {
    if (!font.loadFromFile("assets/Fonts/arial.ttf")) {
        std::cout << "Loi: Khong tim thay file 'arial.ttf'!" << std::endl;
        return false;
    }
    // Tự tạo texture X và O
    texX = createXTexture();
    texO = createOTexture();
    return true;
}

/**
 * Tự vẽ texture X (Dấu X màu xanh)
 */
sf::Texture createXTexture() {
    sf::RenderTexture rt;
    rt.create(CELL_SIZE, CELL_SIZE);
    rt.clear(sf::Color::Transparent); // Nền trong suốt

    sf::RectangleShape line1(sf::Vector2f(CELL_SIZE * 0.8f, 10));
    line1.setFillColor(sf::Color(0, 0, 200)); // Xanh đậm
    line1.setOrigin(CELL_SIZE * 0.4f, 5);
    line1.setPosition(CELL_SIZE / 2.0f, CELL_SIZE / 2.0f);
    line1.setRotation(45);

    sf::RectangleShape line2(sf::Vector2f(CELL_SIZE * 0.8f, 10));
    line2.setFillColor(sf::Color(0, 0, 200));
    line2.setOrigin(CELL_SIZE * 0.4f, 5);
    line2.setPosition(CELL_SIZE / 2.0f, CELL_SIZE / 2.0f);
    line2.setRotation(135);

    rt.draw(line1);
    rt.draw(line2);
    rt.display();
    return rt.getTexture();
}

/**
 * Tự vẽ texture O (Dấu O màu đỏ)
 */
sf::Texture createOTexture() {
    sf::RenderTexture rt;
    rt.create(CELL_SIZE, CELL_SIZE);
    rt.clear(sf::Color::Transparent);

    sf::CircleShape o(CELL_SIZE / 2.0f * 0.8f); // Bán kính 80%
    o.setFillColor(sf::Color::Transparent);
    o.setOutlineColor(sf::Color(200, 0, 0)); // Đỏ đậm
    o.setOutlineThickness(10);
    o.setOrigin(o.getRadius(), o.getRadius());
    o.setPosition(CELL_SIZE / 2.0f, CELL_SIZE / 2.0f);

    rt.draw(o);
    rt.display();
    return rt.getTexture();
}

/**
 * Hàm tiện ích để vẽ chữ
 */
void drawText(std::string str, int x, int y, int size, sf::Color color, bool center) {
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);

    if (center) {
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    }
    text.setPosition((float)x, (float)y);
    window.draw(text);
}

/**
 * Vẽ màn hình Menu
 */
void renderMenu() {
    drawText("GAME CARO", WINDOW_WIDTH / 2, 150, 50, sf::Color::Black, true);

    sf::Color colNormal = sf::Color(100, 100, 100); // Xám
    sf::Color colSelect = sf::Color::Blue;

    drawText("New Game", WINDOW_WIDTH / 2, 250, 30, (menuSelection == 0 ? colSelect : colNormal), true);
    drawText("Load Game", WINDOW_WIDTH / 2, 300, 30, (menuSelection == 1 ? colSelect : colNormal), true);
    drawText("Exit", WINDOW_WIDTH / 2, 350, 30, (menuSelection == 2 ? colSelect : colNormal), true);
}

/**
 * Vẽ màn hình chơi game
 */
void renderGame() {
    // 1. Vẽ lưới
    sf::RectangleShape line;
    line.setFillColor(sf::Color(128, 128, 128)); // Màu xám
    for (int i = 0; i <= BOARD_SIZE; i++) {
        // Vẽ hàng ngang
        line.setSize(sf::Vector2f((float)BOARD_SIZE * CELL_SIZE, 2));
        line.setPosition((float)BOARD_OFFSET_X, (float)(BOARD_OFFSET_Y + i * CELL_SIZE));
        window.draw(line);
        // Vẽ hàng dọc
        line.setSize(sf::Vector2f(2, (float)BOARD_SIZE * CELL_SIZE));
        line.setPosition((float)(BOARD_OFFSET_X + i * CELL_SIZE), (float)BOARD_OFFSET_Y);
        window.draw(line);
    }

    // 2. Vẽ ô chọn (selector)
    sf::RectangleShape selector;
    selector.setSize(sf::Vector2f((float)CELL_SIZE, (float)CELL_SIZE));
    selector.setFillColor(sf::Color(0, 255, 0, 100)); // Màu xanh lá mờ
    selector.setOutlineColor(sf::Color::Green);
    selector.setOutlineThickness(2);
    selector.setPosition(
        (float)(BOARD_OFFSET_X + selectorCol * CELL_SIZE),
        (float)(BOARD_OFFSET_Y + selectorRow * CELL_SIZE)
    );
    window.draw(selector);

    // 3. Vẽ các quân X, O
    sf::Sprite sprite;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == -1) { // X
                sprite.setTexture(texX);
                sprite.setPosition(
                    (float)(BOARD_OFFSET_X + j * CELL_SIZE),
                    (float)(BOARD_OFFSET_Y + i * CELL_SIZE)
                );
                window.draw(sprite);
            }
            else if (_A[i][j].c == 1) { // O
                sprite.setTexture(texO);
                sprite.setPosition(
                    (float)(BOARD_OFFSET_X + j * CELL_SIZE),
                    (float)(BOARD_OFFSET_Y + i * CELL_SIZE)
                );
                window.draw(sprite);
            }
        }
    }

    // 4. Vẽ thông tin lượt chơi
    std::string turnText = (_TURN ? "Luot Nguoi Choi: X" : "Luot Nguoi Choi: O");
    drawText(turnText, WINDOW_WIDTH / 2, 50, 30, (_TURN ? sf::Color::Blue : sf::Color::Red), true);
    drawText("L: Save | T: Load | ESC: Menu", 10, WINDOW_HEIGHT - 30, 18, sf::Color::Black);
}

/**
 * Vẽ màn hình Game Over
 */
void renderGameOver() {
    renderGame(); // Vẽ lại bàn cờ bên dưới

    // Vẽ lớp phủ mờ
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 150)); // Màu đen mờ
    window.draw(overlay);

    // Vẽ thông báo
    drawText(gameOverMessage, WINDOW_WIDTH / 2, 300, 50, sf::Color::Yellow, true);
    drawText("Nhan 'Y' de quay ve Menu", WINDOW_WIDTH / 2, 400, 30, sf::Color::White, true);
}

/**
 * Xử lý sự kiện Menu
 */
void handleMenuEvent(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            menuSelection = (menuSelection - 1 + 3) % 3;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            menuSelection = (menuSelection + 1) % 3;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (menuSelection == 0) { // New Game
                ResetData();
                currentState = PLAYING;
            }
            else if (menuSelection == 1) { // Load Game
                if (LoadGame()) {
                    currentState = PLAYING;
                }
                else {
                    // (Có thể thêm thông báo lỗi tải game ở đây)
                }
            }
            else if (menuSelection == 2) { // Exit
                window.close();
            }
        }
    }
}

/**
 * Xử lý sự kiện Chơi Game
 */
void handleGameEvent(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
        // Di chuyển
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
            selectorRow = (selectorRow - 1 + BOARD_SIZE) % BOARD_SIZE;
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
            selectorRow = (selectorRow + 1) % BOARD_SIZE;
        else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A)
            selectorCol = (selectorCol - 1 + BOARD_SIZE) % BOARD_SIZE;
        else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D)
            selectorCol = (selectorCol + 1) % BOARD_SIZE;

        // Quay về Menu
        else if (event.key.code == sf::Keyboard::Escape)
            currentState = MENU;

        // Save/Load (Đơn giản hóa)
        else if (event.key.code == sf::Keyboard::L)
            SaveGame(); // (Có thể thêm thông báo "Đã lưu!")
        else if (event.key.code == sf::Keyboard::T)
            LoadGame(); // (Có thể thêm thông báo "Đã tải!")

        // Đánh cờ
        else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
            // Kiểm tra xem ô có trống không
            if (CheckBoard(selectorRow, selectorCol) != 0) {
                // Đánh dấu thành công
                int result = TestBoard(); // TestBoard sẽ dùng (selectorRow, selectorCol)

                if (result == -1) {
                    gameOverMessage = "Nguoi Choi X THANG!";
                    currentState = GAME_OVER;
                }
                else if (result == 1) {
                    gameOverMessage = "Nguoi Choi O THANG!";
                    currentState = GAME_OVER;
                }
                else if (result == 0) {
                    gameOverMessage = "HOA CO!";
                    currentState = GAME_OVER;
                }
                else {
                    // Nếu chưa ai thắng, đổi lượt
                    _TURN = !_TURN;
                }
            }
        }
    }
}

/**
 * Xử lý sự kiện Game Over
 */
void handleGameOverEvent(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Y) {
            currentState = MENU;
        }
    }
}


//--- ĐỊNH NGHĨA CÁC HÀM LOGIC (MODEL) ---

/**
 * Bước 4: Khởi tạo dữ liệu
 */
void ResetData() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _A[i][j].c = 0;
        }
    }
    _TURN = true;
    selectorRow = 0;
    selectorCol = 0;
    gameOverMessage = "";
}

/**
 * Bước 9: Kiểm tra bàn cờ (Phiên bản hoàn chỉnh)
 */
int TestBoard()
{
    // Lấy vị trí vừa đánh từ biến toàn cục
    int row = selectorRow;
    int col = selectorCol;
    int playerMark = _A[row][col].c;

    if (playerMark == 0) return 2; // Lỗi (không nên xảy ra)

    // 2. Kiểm tra thắng
    if (CountHorizontal(row, col, playerMark) >= 5 ||
        CountVertical(row, col, playerMark) >= 5 ||
        CountDiagonal1(row, col, playerMark) >= 5 ||
        CountDiagonal2(row, col, playerMark) >= 5)
    {
        return playerMark; // Trả về -1 (X thắng) hoặc 1 (O thắng)
    }

    // 3. Nếu không ai thắng, kiểm tra hòa
    bool boardFull = true;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == 0) {
                boardFull = false;
                break;
            }
        }
        if (!boardFull) break;
    }

    if (boardFull) {
        return 0; // Hòa
    }

    // 4. Nếu không thắng và không hòa, tiếp tục chơi
    return 2;
}


/**
 * Bước 10: Kiểm tra ô và đánh dấu
 */
int CheckBoard(int pRow, int pCol) {
    // Kiểm tra xem ô có rỗng không
    if (_A[pRow][pCol].c == 0) {
        if (_TURN == true) _A[pRow][pCol].c = -1; // Lượt true (X)
        else _A[pRow][pCol].c = 1; // Lượt false (O)
        return _A[pRow][pCol].c;
    }
    return 0; // Ô đã được đánh
}

/**
 * Hàm Lưu Game (Đơn giản hóa)
 */
void SaveGame() {
    std::ofstream f("gamesave.txt", std::ios::binary);
    if (!f.is_open()) {
        std::cout << "Loi: Khong the mo file 'caro.dat' de luu!" << std::endl;
        return;
    }

    // Ghi dữ liệu game vào file
    f.write((char*)_A, sizeof(_A));
    f.write((char*)&_TURN, sizeof(_TURN));
    f.write((char*)&selectorRow, sizeof(selectorRow));
    f.write((char*)&selectorCol, sizeof(selectorCol));

    f.close();
    std::cout << "Da luu game vao 'gamesave.txt'!" << std::endl;
}

/**
 * Hàm Tải Game
 */
bool LoadGame() {
    std::ifstream f("gamesave.txt", std::ios::binary);
    if (!f.is_open()) {
        std::cout << "Loi: Khong tim thay file 'gamesave.txt' de tai!" << std::endl;
        return false;
    }

    // Đọc dữ liệu (THEO ĐÚNG THỨ TỰ LÚC LƯU)
    f.read((char*)_A, sizeof(_A));
    f.read((char*)&_TURN, sizeof(_TURN));
    f.read((char*)&selectorRow, sizeof(selectorRow));
    f.read((char*)&selectorCol, sizeof(selectorCol));

    f.close();
    gameOverMessage = ""; // Reset thông báo (nếu có)
    std::cout << "Da tai game tu 'gamesave.txt'!" << std::endl;
    return true;
}


//--- CÁC HÀM PHỤ TRỢ ĐẾM (KHÔNG THAY ĐỔI) ---

int CountHorizontal(int pRow, int pCol, int pMark) {
    int count = 1;
    for (int i = 1; i < 5; i++) {
        if (pCol - i >= 0 && _A[pRow][pCol - i].c == pMark) count++;
        else break;
    }
    for (int i = 1; i < 5; i++) {
        if (pCol + i < BOARD_SIZE && _A[pRow][pCol + i].c == pMark) count++;
        else break;
    }
    return count;
}

int CountVertical(int pRow, int pCol, int pMark) {
    int count = 1;
    for (int i = 1; i < 5; i++) {
        if (pRow - i >= 0 && _A[pRow - i][pCol].c == pMark) count++;
        else break;
    }
    for (int i = 1; i < 5; i++) {
        if (pRow + i < BOARD_SIZE && _A[pRow + i][pCol].c == pMark) count++;
        else break;
    }
    return count;
}

int CountDiagonal1(int pRow, int pCol, int pMark) {
    int count = 1;
    for (int i = 1; i < 5; i++) {
        if (pRow - i >= 0 && pCol - i >= 0 && _A[pRow - i][pCol - i].c == pMark) count++;
        else break;
    }
    for (int i = 1; i < 5; i++) {
        if (pRow + i < BOARD_SIZE && pCol + i < BOARD_SIZE && _A[pRow + i][pCol + i].c == pMark) count++;
        else break;
    }
    return count;
}

int CountDiagonal2(int pRow, int pCol, int pMark) {
    int count = 1;
    for (int i = 1; i < 5; i++) {
        if (pRow - i >= 0 && pCol + i < BOARD_SIZE && _A[pRow - i][pCol + i].c == pMark) count++;
        else break;
    }
    for (int i = 1; i < 5; i++) {
        if (pRow + i < BOARD_SIZE && pCol - i >= 0 && _A[pRow + i][pCol - i].c == pMark) count++;
        else break;
    }
    return count;
}