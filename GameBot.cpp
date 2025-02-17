#include "GameBot.h"

struct Move
{
    int row, col;
    char symbol; // 'X' cho người chơi, 'O' cho bot
    bool isPlayerMove;
};
stack<Move> moveHistory;

GameBot::GameBot(int boardSize, sf::Color boardColor, MenuCarogame &menu)
    : boardSize(boardSize),
      boardColor(boardColor),
      window(sf::VideoMode(1100, 1100), "Caro Game"),
      menu(menu),
      emptyCells(boardSize * boardSize),
      currentI(-1),
      currentJ(-1),
      boardValue(0),
      lastPlayed(0),
      turn(-1),
      rollingHash(0),
      winningLine()
{
    if (!font.loadFromFile("D:/DATA STRUCTURE & ALGORITHM_EXERCISES/Project/caro_nxn/assets/fonts/Roboto/Roboto-Medium.ttf"))
    {
        throw runtime_error("Failed to load font.");
    }

    cellSize = window.getSize().x / boardSize;
    board.resize(boardSize, vector<char>(boardSize, ' '));
    IntBoard.resize(boardSize, vector<int>(boardSize, 0));

    for (int i = 1; i < boardSize; ++i)
    {
        sf::RectangleShape line(sf::Vector2f(window.getSize().x, 2));
        line.setFillColor(sf::Color::Black);
        line.setPosition(0, i * cellSize);
        gridLines.push_back(line);

        line.setSize(sf::Vector2f(2, window.getSize().y));
        line.setPosition(i * cellSize, 0);
        gridLines.push_back(line);
    }

    if (boardSize < 4)
    {
        winCondition = 3; // Với bàn cờ nhỏ
    }
    else
    {
        winCondition = 5; // Với bàn cờ lớn
    }
    patternDict = create_pattern_dict();
    initZobrist();
    if (boardSize > 15)
    {
        depth = 3;
    }
    else
    {
        depth = 2;
        if (boardSize == 1)
        {
            depth = 1;
        }
    }
    dem = 0;
}
void GameBot::run()
{
    while (window.isOpen())
    {
        if (!isDifficultySelected)
        {
            showDifficultySelection(window);
        }

        processEvents();
        if (!window.isOpen())
            break;
        render();
    }
}
void GameBot::processEvents()
{
    sf::Event event;

    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
            return;
        }

        // Kiểm tra sự kiện phím Ctrl + Z (Undo)
        if (event.type == sf::Event::KeyPressed && event.key.control && event.key.code == sf::Keyboard::Z)
        {
            if (!isUndoing) // Đảm bảo không đang thực hiện Undo
            {
                isUndoing = true; // Đặt cờ Undo là true
                undoMove();       // Thực hiện Undo
            }
        }

        if (gameOver)
        {
            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

                if (playAgainText.getGlobalBounds().contains(mousePosition.x, mousePosition.y))
                {
                    resetGame();
                    return;
                }
                else if (newGameText.getGlobalBounds().contains(mousePosition.x, mousePosition.y))
                {
                    window.close();
                    return;
                }
            }
        }
        else
        {
            if (event.type == sf::Event::MouseButtonPressed)
            {
                handleMouseClick(sf::Mouse::getPosition(window));
            }
        }
    }
}

void GameBot::undoMove()
{
    if (!moveHistory.empty())
    {
        // Lấy bước đi cuối cùng và xóa nó khỏi lịch sử
        Move lastMove = moveHistory.top();
        moveHistory.pop();

        // Xóa bước đi trên bàn cờ
        board[lastMove.row][lastMove.col] = ' ';
        IntBoard[lastMove.row][lastMove.col] = 0;

        // Xóa marker khỏi danh sách markers
        markers.pop_back();

        lastMove = moveHistory.top();
        moveHistory.pop();
        board[lastMove.row][lastMove.col] = ' ';
        IntBoard[lastMove.row][lastMove.col] = 0;
        markers.pop_back();

        // Cập nhật lại trạng thái trò chơi
        gameOver = false; // Nếu undo nước đi, trò chơi vẫn tiếp tục
        playerWinner = false;
        botCheck = false;

        // Vẽ lại bàn cờ và các markers sau khi undo
        render(); // Hàm này vẽ lại bàn cờ và các markers
        isUndoing = false;
    }
}
void GameBot::render()
{
    window.clear(boardColor);

    float boardAreaSize = min(window.getSize().x, window.getSize().y);
    cellSize = boardAreaSize / boardSize;

    for (int row = 0; row < boardSize; ++row)
    {
        for (int col = 0; col < boardSize; ++col)
        {
            sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
            cell.setPosition(col * cellSize, row * cellSize);

            bool isWinningCell = false;
            if (gameOver && !gameDraw)
            {
                for (const auto &pos : winningLine)
                {
                    if (pos.x == row && pos.y == col)
                    {
                        isWinningCell = true;
                        break;
                    }
                }
            }
            if (isWinningCell)
            {
                cell.setFillColor(sf::Color::Green);
            }
            else if (boardColor == sf::Color::Black)
            {
                cell.setFillColor(sf::Color::Black);
                cell.setOutlineColor(sf::Color::White);
            }
            else
            {
                cell.setFillColor(sf::Color::White);
                cell.setOutlineColor(sf::Color::Black);
            }
            cell.setOutlineThickness(1);
            window.draw(cell);
        }
    }

    for (const auto &marker : markers)
    {
        window.draw(marker);
    }

    if (gameOver)
    {

        if (gameDraw)
        {
            showWinScreen("Draw");
        }
        else if (playerWinner)
        {
            showWinScreen("PLAYER");
        }
        else
        {
            showWinScreen("BOT");
        }
    }

    window.display();
}

bool GameBot::checkDraw()
{
    for (int r = 0; r < boardSize; ++r)
    {
        for (int c = 0; c < boardSize; ++c)
        {
            if (board[r][c] == ' ')
            {
                return false;
            }
        }
    }
    return true;
}
bool GameBot::checkWin(int row, int col, char player, bool botCheck, vector<sf::Vector2i> &winningLine)
{
    int winCondition = (boardSize <= 4) ? 3 : 5;

    auto checkDirection = [&](int dr, int dc)
    {
        int count = 1;
        vector<sf::Vector2i> tempLine = {{row, col}};

        for (int step = 1; step < winCondition; ++step)
        {
            int nr = row + step * dr, nc = col + step * dc;
            if (nr >= 0 && nr < boardSize && nc >= 0 && nc < boardSize && board[nr][nc] == player)
            {
                ++count;
                tempLine.push_back({nr, nc});
            }
            else
                break;
        }

        for (int step = 1; step < winCondition; ++step)
        {
            int nr = row - step * dr, nc = col - step * dc;
            if (nr >= 0 && nr < boardSize && nc >= 0 && nc < boardSize && board[nr][nc] == player)
            {
                ++count;
                tempLine.push_back({nr, nc});
            }
            else
                break;
        }

        if (count >= winCondition)
        {
            winningLine = tempLine;
            return true;
        }
        return false;
    };

    if (checkDirection(1, 0) || checkDirection(0, 1) || checkDirection(1, 1) || checkDirection(1, -1))
    {
        if (!botCheck)
            gameOver = true;
        return true;
    }
    return false;
}
void GameBot::showWinScreen(const string &winner)
{
    float backgroundWidth = window.getSize().x * 0.6f;
    float backgroundHeight = window.getSize().y * 0.4f;

    sf::RectangleShape background(sf::Vector2f(backgroundWidth, backgroundHeight));
    background.setFillColor(sf::Color(0, 0, 0, 200));
    background.setPosition((window.getSize().x - backgroundWidth) / 2, (window.getSize().y - backgroundHeight) / 2);

    int fontSize = static_cast<int>(backgroundHeight * 0.15f);

    if (winner == "Draw")
    {
        winnerText.setString("It's a Draw!");
        winnerText.setFillColor(sf::Color::Yellow);
    }
    else if (winner == "BOT")
    {
        winnerText.setString("YOU LOSE!");
        winnerText.setFillColor(sf::Color::Red);
    }
    else if (winner == "PLAYER")
    {
        winnerText.setString("YOU WIN!");
        winnerText.setFillColor(sf::Color::Green);
    }

    winnerText.setFont(font);
    winnerText.setCharacterSize(fontSize);
    winnerText.setOutlineColor(sf::Color::Black);
    winnerText.setOutlineThickness(2);
    winnerText.setOrigin(winnerText.getLocalBounds().width / 2, winnerText.getLocalBounds().height / 2);
    winnerText.setPosition(background.getPosition().x + backgroundWidth / 2, background.getPosition().y + backgroundHeight * 0.25f);

    playAgainText.setFont(font);
    playAgainText.setString("Play Again");
    playAgainText.setCharacterSize(fontSize * 0.8f);
    playAgainText.setFillColor(sf::Color::White);
    playAgainText.setOrigin(playAgainText.getLocalBounds().width / 2, playAgainText.getLocalBounds().height / 2);
    playAgainText.setPosition(background.getPosition().x + backgroundWidth / 2, background.getPosition().y + backgroundHeight * 0.55f);

    newGameText.setFont(font);
    newGameText.setString("New Game");
    newGameText.setCharacterSize(fontSize * 0.8f);
    newGameText.setFillColor(sf::Color::White);
    newGameText.setOrigin(newGameText.getLocalBounds().width / 2, newGameText.getLocalBounds().height / 2);
    newGameText.setPosition(background.getPosition().x + backgroundWidth / 2, background.getPosition().y + backgroundHeight * 0.75f);

    window.draw(background);
    window.draw(winnerText);
    window.draw(playAgainText);
    window.draw(newGameText);
}
void GameBot::resetGame()
{

    board.assign(boardSize, vector<char>(boardSize, ' '));
    IntBoard.assign(boardSize, vector<int>(boardSize, 0));
    markers.clear();
    bound.clear();
    nextBound.clear();
    turn = -1;
    currentI = -1;
    currentJ = -1;
    boardValue = 0;
    lastPlayed = 0;
    TTable.clear();
    rollingHash = 0;
    gameOver = false;
    cout << "reset game" << endl;
    winningLine.clear();
}
void GameBot::showDifficultySelection(sf::RenderWindow &window)
{
    sf::Text instructionText;
    instructionText.setFont(font);
    instructionText.setString("Choose Bot Difficulty");
    instructionText.setCharacterSize(30);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setPosition(window.getSize().x / 2 - instructionText.getLocalBounds().width / 2, 50);

    easyButton.setSize(sf::Vector2f(150, 50));
    easyButton.setFillColor(sf::Color(100, 100, 255));
    easyButton.setOutlineThickness(0);
    easyButton.setPosition(window.getSize().x / 2 - 250, 150);

    easyText.setFont(font);
    easyText.setString("Easy");
    easyText.setCharacterSize(20);
    easyText.setFillColor(sf::Color::White);
    easyText.setPosition(easyButton.getPosition().x + 40, easyButton.getPosition().y + 10);

    mediumButton.setSize(sf::Vector2f(150, 50));
    mediumButton.setFillColor(sf::Color(255, 150, 0));
    mediumButton.setOutlineThickness(0);
    mediumButton.setPosition(window.getSize().x / 2 - 75, 150);

    mediumText.setFont(font);
    mediumText.setString("Medium");
    mediumText.setCharacterSize(20);
    mediumText.setFillColor(sf::Color::White);
    mediumText.setPosition(mediumButton.getPosition().x + 25, mediumButton.getPosition().y + 10);

    hardButton.setSize(sf::Vector2f(150, 50));
    hardButton.setFillColor(sf::Color(255, 100, 100));
    hardButton.setOutlineThickness(0);
    hardButton.setPosition(window.getSize().x / 2 + 100, 150);

    hardText.setFont(font);
    hardText.setString("Hard");
    hardText.setCharacterSize(20);
    hardText.setFillColor(sf::Color::White);
    hardText.setPosition(hardButton.getPosition().x + 40, hardButton.getPosition().y + 10);

    sf::Text symbolText;
    symbolText.setFont(font);
    symbolText.setString("Choose Your Symbol");
    symbolText.setCharacterSize(30);
    symbolText.setFillColor(sf::Color::White);
    symbolText.setPosition(window.getSize().x / 2 - symbolText.getLocalBounds().width / 2, 250);

    xButton.setSize(sf::Vector2f(150, 50));
    xButton.setFillColor(sf::Color(100, 255, 100));
    xButton.setOutlineThickness(0);
    xButton.setPosition(window.getSize().x / 2 - 200, 350);

    xText.setFont(font);
    xText.setString("X");
    xText.setCharacterSize(30);
    xText.setFillColor(sf::Color::Blue);
    xText.setPosition(xButton.getPosition().x + 60, xButton.getPosition().y + 5);

    oButton.setSize(sf::Vector2f(150, 50));
    oButton.setFillColor(sf::Color(255, 100, 100));
    oButton.setOutlineThickness(0);
    oButton.setPosition(window.getSize().x / 2 + 50, 350);

    oText.setFont(font);
    oText.setString("O");
    oText.setCharacterSize(30);
    oText.setFillColor(sf::Color::Red);
    oText.setPosition(oButton.getPosition().x + 60, oButton.getPosition().y + 5);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                return;
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (easyButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    botDifficulty = "EASY";
                    isDifficultySelected = true;
                    easyButton.setOutlineThickness(5);
                    easyButton.setOutlineColor(sf::Color::White);
                    mediumButton.setOutlineThickness(0);
                    hardButton.setOutlineThickness(0);
                }
                if (mediumButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    botDifficulty = "MEDIUM";
                    isDifficultySelected = true;
                    mediumButton.setOutlineThickness(5);
                    mediumButton.setOutlineColor(sf::Color::White);
                    easyButton.setOutlineThickness(0);
                    hardButton.setOutlineThickness(0);
                }
                if (hardButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    botDifficulty = "HARD";
                    isDifficultySelected = true;
                    hardButton.setOutlineThickness(5);
                    hardButton.setOutlineColor(sf::Color::White);
                    easyButton.setOutlineThickness(0);
                    mediumButton.setOutlineThickness(0);
                }
                if (xButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    playerSymbol = 'X';
                    botSymbol = 'O';
                    isSymbolSelected = true;
                    xButton.setOutlineThickness(5);
                    xButton.setOutlineColor(sf::Color::White);
                    oButton.setOutlineThickness(0);
                }
                else if (oButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    playerSymbol = 'O';
                    botSymbol = 'X';
                    isSymbolSelected = true;
                    oButton.setOutlineThickness(5);
                    oButton.setOutlineColor(sf::Color::White);
                    xButton.setOutlineThickness(0);
                }
                if (isDifficultySelected && isSymbolSelected)
                {
                    return;
                }
            }
        }

        // Vẽ các thành phần
        window.clear(sf::Color::Black);
        window.draw(instructionText);
        window.draw(easyButton);
        window.draw(easyText);
        window.draw(mediumButton);
        window.draw(mediumText);
        window.draw(hardButton);
        window.draw(hardText);
        window.draw(symbolText);
        window.draw(xButton);
        window.draw(xText);
        window.draw(oButton);
        window.draw(oText);
        window.display();
    }
}
sf::Vector2i GameBot::easyBotMove()
{
    // 1. Tìm nước đi cần chặn người chơi
    sf::Vector2i blockMove = findBlockMove();
    if (blockMove.x != -1 && blockMove.y != -1)
    {
        return blockMove; // Trả về nước đi chặn
    }

    // 2. Tìm nước đi để tiếp tục chuỗi của bot
    sf::Vector2i extendMove = findExtendMove();
    if (extendMove.x != -1 && extendMove.y != -1)
    {
        return extendMove; // Trả về nước đi mở rộng chuỗi
    }

    // 3. Đánh xung quanh nước đi của người chơi
    sf::Vector2i surroundMove = findSurroundMove();
    if (surroundMove.x != -1 && surroundMove.y != -1)
    {
        return surroundMove; // Trả về nước đi xung quanh
    }

    // 4. Nếu không có nước đi đặc biệt, đánh ô trống bất kỳ
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] == ' ')
            {
                return sf::Vector2i(col, row); // Nước đi đầu tiên có thể
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không còn ô trống
}
sf::Vector2i GameBot::findBlockMove()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue;

            for (const auto &dir : directions)
            {
                int count = 1;
                bool leftEmpty = false, rightEmpty = false;
                int leftX = col - dir[1], leftY = row - dir[0];
                int rightX = col + dir[1], rightY = row + dir[0];

                // Đếm chuỗi nước đi của người chơi
                count += countDirection(row, col, dir[0], dir[1], playerSymbol);
                count += countDirection(row, col, -dir[0], -dir[1], playerSymbol);

                // Kiểm tra hai đầu chuỗi
                if (isValid(leftY, leftX) && board[leftY][leftX] == ' ')
                    leftEmpty = true;
                if (isValid(rightY, rightX) && board[rightY][rightX] == ' ')
                    rightEmpty = true;

                // Chặn nếu phát hiện chuỗi cần ngăn
                if (count == winCondition - 2 && (leftEmpty || rightEmpty))
                {
                    return leftEmpty ? sf::Vector2i(leftX, leftY) : sf::Vector2i(rightX, rightY);
                }
                else if (count == winCondition - 1 && (leftEmpty || rightEmpty))
                {
                    return leftEmpty ? sf::Vector2i(leftX, leftY) : sf::Vector2i(rightX, rightY);
                }
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không tìm thấy nước đi cần chặn
}
sf::Vector2i GameBot::findExtendMove()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != botSymbol)
                continue;

            for (const auto &dir : directions)
            {
                int count = 1;
                int nextX = col + dir[1], nextY = row + dir[0];

                // Đếm chuỗi liên tiếp của bot
                count += countDirection(row, col, dir[0], dir[1], botSymbol);

                // Kiểm tra nước tiếp theo
                if (isValid(nextY, nextX) && board[nextY][nextX] == ' ')
                {
                    return sf::Vector2i(nextX, nextY);
                }
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không tìm thấy nước đi mở rộng
}
sf::Vector2i GameBot::findSurroundMove()
{
    const int offsets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] == playerSymbol)
            {
                for (const auto &offset : offsets)
                {
                    int x = col + offset[1], y = row + offset[0];
                    if (isValid(y, x) && board[y][x] == ' ')
                    {
                        return sf::Vector2i(x, y);
                    }
                }
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không tìm thấy nước đi xung quanh
}
int GameBot::countDirection(int row, int col, int dRow, int dCol, char symbol)
{
    int count = 0;
    int x = col + dCol, y = row + dRow;
    while (isValid(y, x) && board[y][x] == symbol)
    {
        count++;
        x += dCol;
        y += dRow;
    }
    return count;
}
bool GameBot::isValid(int row, int col)
{
    return row >= 0 && row < boardSize && col >= 0 && col < boardSize;
}
sf::Vector2i GameBot::mediumBotMove()
{
    // 2. Tìm nước đi để tiếp tục chuỗi của bot
    sf::Vector2i extendMove = getBestExtendMove();
    if (extendMove.x != -1 && extendMove.y != -1)
    {
        return extendMove; // Trả về nước đi mở rộng chuỗi
    }

    // 1. Tìm nước đi cần chặn người chơi
    sf::Vector2i blockMove = getBestBlockingMove();
    if (blockMove.x != -1 && blockMove.y != -1)
    {
        return blockMove; // Trả về nước đi chặn
    }

    // 3. Đánh xung quanh nước đi của người chơi
    sf::Vector2i surroundMove = findSurroundMove();
    if (surroundMove.x != -1 && surroundMove.y != -1)
    {
        return surroundMove; // Trả về nước đi xung quanh
    }

    // 4. Nếu không có nước đi đặc biệt, đánh ô trống bất kỳ
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] == ' ')
            {
                return sf::Vector2i(col, row); // Nước đi đầu tiên có thể
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không còn ô trống
}
sf::Vector2i GameBot::getBestExtendMove()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

    // Ưu tiên kiểm tra các thế của bot
    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != botSymbol)
                continue;

            for (const auto &dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], botSymbol) + 1;

                // Nếu bot có 4 nước liên tiếp, đánh thành 5 để thắng

                sf::Vector2i blockMove = detectFourBlocked();
                if (blockMove.x != -1 && blockMove.y != -1)
                {
                    return blockMove; // Ưu tiên chặn người chơi
                }

                if (count == 4 && checkOpenEnds(row, col, dir, count) >= 1)
                {
                    return findBlockingMove(row, col, dir, count);
                }

                // Nếu bot có 3 nước liên tiếp không bị chặn hai đầu, mở rộng thành 4
                if (count == 3 && checkOpenEnds(row, col, dir, count) == 2)
                {
                    return findBlockingMove(row, col, dir, count);
                }
            }
        }
    }

    // Trước khi mở rộng, đảm bảo không có thế nguy hiểm của người chơi

    return sf::Vector2i(-1, -1); // Không tìm thấy nước đi mở rộng hoặc chặn
}
sf::Vector2i GameBot::getBestBlockingMove()
{
    sf::Vector2i move;

    // Ưu tiên chặn các thế nguy hiểm nhất
    if (boardSize < 5)
    {

        move = detectTwoConsecutive();
        if (move.x != -1 && move.y != -1)
            return move;
    };
    move = detectAlmostFive();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectFourBlocked();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectThreeOpen();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectDoubleThree();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectFourThree();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectTwoFourParallel();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectGapPair();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectLOrTShape();
    if (move.x != -1 && move.y != -1)
        return move;

    move = detectOpenTwo();
    if (move.x != -1 && move.y != -1)
        return move;

    return sf::Vector2i(-1, -1); // Không có nước chặn
}
sf::Vector2i GameBot::findBlockingMove(int row, int col, const int dir[2], int count)
{
    // Kiểm tra đầu trước (phía -dir)
    int prevRow = row - dir[0];
    int prevCol = col - dir[1];
    if (isValid(prevRow, prevCol) && board[prevRow][prevCol] == ' ')
    {
        return sf::Vector2i(prevCol, prevRow); // (col, row) để đúng thứ tự sf::Vector2i
    }

    // Kiểm tra đầu sau (phía +dir)
    int nextRow = row + count * dir[0];
    int nextCol = col + count * dir[1];
    if (isValid(nextRow, nextCol) && board[nextRow][nextCol] == ' ')
    {
        return sf::Vector2i(nextCol, nextRow); // (col, row)
    }

    return sf::Vector2i(-1, -1); // Không tìm thấy nước chặn hợp lệ
}
int GameBot::checkOpenEnds(int row, int col, const int dir[2], int length)
{
    int openEnds = 0;

    // Kiểm tra đầu trước (phía -dir)
    int prevRow = row - dir[0];
    int prevCol = col - dir[1];
    if (isValid(prevRow, prevCol) && board[prevRow][prevCol] == ' ')
    {
        openEnds++;
    }

    // Kiểm tra đầu sau (phía +dir)
    int nextRow = row + length * dir[0];
    int nextCol = col + length * dir[1];
    if (isValid(nextRow, nextCol) && board[nextRow][nextCol] == ' ')
    {
        openEnds++;
    }

    return openEnds; // Trả về số đầu mở
}
sf::Vector2i GameBot::detectTwoConsecutive()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không phải của đối thủ

            // Kiểm tra 4 hướng
            for (const auto &dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;
                // Nếu phát hiện 2 liên tiếp không bị chặn cả hai đầu
                if (count == 2 && checkOpenEnds(row, col, dir, count) == 1)
                {
                    return findBlockingMove(row, col, dir, count); // Trả về nước chặn
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không tìm thấy chuỗi 2 liên tiếp
}

sf::Vector2i GameBot::detectAlmostFive()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không phải của đối thủ

            // Kiểm tra 4 hướng
            for (const auto &dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;
                if (count == 4 && checkOpenEnds(row, col, dir, count) == 2)
                {
                    return findBlockingMove(row, col, dir, count); // Trả về nước chặn
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không tìm thấy thế gần thắng
}

sf::Vector2i GameBot::detectFourBlocked()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            for (auto dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;
                // Nếu có chuỗi 4 quân liên tiếp bị chặn một đầu
                if (count == 4 && checkOpenEnds(row, col, dir, count) == 1)
                {

                    dangerous = true;
                    return findBlockingMove(row, col, dir, count); // Tìm và trả về nước đi chặn
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không tìm thấy thế 4 bị chặn
}

sf::Vector2i GameBot::detectThreeOpen()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            for (auto dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;

                // Nếu có chuỗi 3 quân liên tiếp và cả hai đầu đều mở
                if (count == 3 && checkOpenEnds(row, col, dir, count) == 2)
                {

                    return findBlockingMove(row, col, dir, count); // Tìm và trả về nước đi chặn
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không tìm thấy thế 3 mở
}

sf::Vector2i GameBot::detectDoubleThree()
{
    // Tìm hai chuỗi 3 không bị chặn
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            vector<sf::Vector2i> potentialMoves;

            for (auto dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;

                // Kiểm tra chuỗi 3 quân không bị chặn hai đầu
                if (count == 3 && checkOpenEnds(row, col, dir, count) == 2)
                {
                    sf::Vector2i blockMove = findBlockingMove(row, col, dir, count);
                    if (blockMove.x != -1 && blockMove.y != -1)
                    {
                        potentialMoves.push_back(blockMove);
                    }
                }

                // Nếu phát hiện được ít nhất hai chuỗi 3-3 kép
                if (potentialMoves.size() >= 2)
                {
                    return potentialMoves[0]; // Ưu tiên nước đi đầu tiên để chặn
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không có thế 3-3 kép
}

sf::Vector2i GameBot::detectFourThree()
{
    // Tìm chuỗi 4-3 kết hợp
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            for (auto dir : directions)
            {
                // Đếm chuỗi 4 trong hướng dir
                int countFour = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;
                // Đếm chuỗi 3 trong hướng ngược lại (-dir)
                int countThree = countDirection(row, col, -dir[0], -dir[1], playerSymbol) + 1;

                // Kiểm tra chuỗi 4 và chuỗi 3 đồng thời
                if (countFour == 4 && checkOpenEnds(row, col, dir, countFour) == 1 &&
                    countThree == 3 && checkOpenEnds(row, col, dir, countThree) == 2)
                {
                    // Ưu tiên chặn chuỗi 4 quân trước
                    sf::Vector2i blockMoveFour = findBlockingMove(row, col, dir, countFour);
                    if (blockMoveFour.x != -1 && blockMoveFour.y != -1)
                    {
                        return blockMoveFour;
                    }

                    // Nếu không thể chặn chuỗi 4, chặn chuỗi 3 quân
                    sf::Vector2i blockMoveThree = findBlockingMove(row, col, dir, countThree);
                    if (blockMoveThree.x != -1 && blockMoveThree.y != -1)
                    {
                        return blockMoveThree;
                    }
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không có thế 4-3
}

sf::Vector2i GameBot::detectTwoFourParallel()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Hướng ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            int fourCount = 0;                  // Đếm số chuỗi 4 bị chặn
            sf::Vector2i blockMove(-1, -1);     // Lưu nước đi chặn cuối cùng
            vector<sf::Vector2i> blockingMoves; // Lưu các nước đi có thể chặn

            for (auto dir : directions)
            {
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;

                if (count == 4 && checkOpenEnds(row, col, dir, count) == 1)
                {
                    sf::Vector2i potentialBlock = findBlockingMove(row, col, dir, count);
                    if (potentialBlock.x != -1 && potentialBlock.y != -1)
                    {
                        fourCount++;
                        blockingMoves.push_back(potentialBlock);
                    }
                }

                // Nếu tìm thấy ít nhất 2 chuỗi 4 song song
                if (fourCount >= 2)
                {
                    // Ưu tiên chặn tại vị trí có thể phá cả hai chuỗi
                    for (const auto &move : blockingMoves)
                    {
                        if (std::count(blockingMoves.begin(), blockingMoves.end(), move) > 1)
                        {
                            return move; // Trả về nước đi chặn chung
                        }
                    }
                    // Nếu không có nước chặn chung, trả về nước đầu tiên
                    return blockingMoves.front();
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không có thế 2 chuỗi 4 song song
}

sf::Vector2i GameBot::detectGapPair()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Các hướng kiểm tra

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không thuộc đối thủ

            for (auto dir : directions)
            {
                if (isGapPresent(row, col, dir))
                {
                    // Xác định vị trí ô trống giữa cặp quân
                    int gapRow = row + dir[0];
                    int gapCol = col + dir[1];
                    if (isValid(gapRow, gapCol) && board[gapRow][gapCol] == ' ')
                    {
                        return sf::Vector2i(gapRow, gapCol); // Trả về vị trí chặn
                    }
                }
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không có cặp song song với khoảng cách 1 ô
}
bool GameBot::isGapPresent(int row, int col, const int dir[2])
{
    // Tìm vị trí ô trống giữa hai quân
    int gapRow = row + dir[0];
    int gapCol = col + dir[1];

    if (!isValid(gapRow, gapCol) || board[gapRow][gapCol] != ' ')
        return false; // Không phải ô trống hoặc ngoài giới hạn

    // Kiểm tra hai phía của ô trống
    int leftRow = row - dir[0];
    int leftCol = col - dir[1];
    int rightRow = gapRow + dir[0];
    int rightCol = gapCol + dir[1];

    return isValid(leftRow, leftCol) && board[leftRow][leftCol] == playerSymbol &&
           isValid(rightRow, rightCol) && board[rightRow][rightCol] == playerSymbol;
}

sf::Vector2i GameBot::detectLOrTShape()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Các hướng kiểm tra: ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không phải của đối thủ

            // Kiểm tra các kết hợp hướng vuông góc nhau để phát hiện chữ L hoặc chữ T
            for (int i = 0; i < 4; i++)
            {
                auto dir1 = directions[i];           // Hướng đầu tiên
                auto dir2 = directions[(i + 1) % 4]; // Hướng vuông góc với hướng đầu tiên

                // Kiểm tra xem có hình L hoặc T không
                if (isLShape(row, col, dir1, dir2))
                {
                    // Tính toán số lượng quân trong mỗi hướng
                    int count1 = countDirection(row, col, dir1[0], dir1[1], playerSymbol) + 1;
                    int count2 = countDirection(row, col, dir2[0], dir2[1], playerSymbol) + 1;

                    // Gọi findBlockingMove với các tham số row, col, dir1, count1
                    sf::Vector2i blockMove = findBlockingMove(row, col, dir1, count1);
                    if (blockMove.x != -1 && blockMove.y != -1) // Nếu tìm thấy nước chặn hợp lệ
                    {
                        return blockMove;
                    }

                    // Gọi findBlockingMove với các tham số row, col, dir2, count2
                    blockMove = findBlockingMove(row, col, dir2, count2);
                    if (blockMove.x != -1 && blockMove.y != -1) // Nếu tìm thấy nước chặn hợp lệ
                    {
                        return blockMove;
                    }
                }
            }
        }
    }
    return sf::Vector2i(-1, -1); // Không tìm thấy hình dạng L hoặc T
}
bool GameBot::isLShape(int row, int col, const int dir1[2], const int dir2[2])
{
    // Kiểm tra hai chuỗi theo hai hướng vuông góc nhau
    int count1 = countDirection(row, col, dir1[0], dir1[1], playerSymbol); // Đếm theo hướng đầu tiên
    int count2 = countDirection(row, col, dir2[0], dir2[1], playerSymbol); // Đếm theo hướng vuông góc

    // Kiểm tra nếu có ít nhất 2 quân ở mỗi hướng
    if (count1 >= 2 && count2 >= 2)
    {
        // Kiểm tra đầu mở ở cả hai hướng
        int openEnds = 0;
        if (isValid(row - dir1[0], col - dir1[1]) && board[row - dir1[0]][col - dir1[1]] == ' ')
            openEnds++;
        if (isValid(row + count1 * dir1[0], col + count1 * dir1[1]) && board[row + count1 * dir1[0]][col + count1 * dir1[1]] == ' ')
            openEnds++;

        if (isValid(row - dir2[0], col - dir2[1]) && board[row - dir2[0]][col - dir2[1]] == ' ')
            openEnds++;
        if (isValid(row + count2 * dir2[0], col + count2 * dir2[1]) && board[row + count2 * dir2[0]][col + count2 * dir2[1]] == ' ')
            openEnds++;

        // Nếu có ít nhất 2 đầu mở, thì có thể là L hoặc T
        if (openEnds >= 2)
        {
            return true; // Phát hiện được L hoặc T
        }
    }

    return false; // Không phải L hoặc T
}
sf::Vector2i GameBot::detectOpenTwo()
{
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}}; // Các hướng kiểm tra: ngang, dọc, chéo xuôi, chéo ngược

    for (int row = 0; row < boardSize; row++)
    {
        for (int col = 0; col < boardSize; col++)
        {
            if (board[row][col] != playerSymbol)
                continue; // Bỏ qua ô không phải của đối thủ

            // Kiểm tra các hướng để tìm chuỗi 2 quân mở
            for (auto dir : directions)
            {
                // Đếm số quân liên tiếp theo hướng dir
                int count = countDirection(row, col, dir[0], dir[1], playerSymbol) + 1;

                // Nếu tìm thấy chuỗi 2 quân mở
                if (count == 2 && checkOpenEnds(row, col, dir, count) == 2)
                {
                    // Tính toán các nước chặn theo hướng
                    return findBlockingMove(row, col, dir, count);
                }
            }
        }
    }

    return sf::Vector2i(-1, -1); // Không có chuỗi 2 quân mở
}
//////////////////
void GameBot::handleMouseClick(const sf::Vector2i &mousePosition)
{
    cout << "currentI: " << currentI << "\n";
    cout << "currentJ: " << currentJ << "\n";
    cout << "boardValue: " << boardValue << "\n";
    cout << "lastPlayed: " << lastPlayed << "\n";
    cout << "turn: " << turn << "\n";
    cout << "dem: " << dem << "\n";

    // In map<pair<int, int>, int> bound
    cout << "bound:\n";
    for (const auto &[key, value] : bound)
    {
        cout << "(" << key.first << ", " << key.second << "): " << value << "\n";
    }

    // In vector<vector<vector<uint64_t>>> zobristTable
    // cout << "zobristTable:\n";
    // for (size_t i = 0; i < zobristTable.size(); i++)
    // {
    //     for (size_t j = 0; j < zobristTable[i].size(); j++)
    //     {
    //         for (size_t k = 0; k < zobristTable[i][j].size(); k++)
    //         {
    //             cout << "zobristTable[" << i << "][" << j << "][" << k << "]: "
    //                  << zobristTable[i][j][k] << "\n";
    //         }
    //     }
    // }

    // In map<uint64_t, pair<int, int>> TTable
    cout << "TTable:\n";
    for (const auto &[key, value] : TTable)
    {
        cout << "Hash: " << key << " -> (" << value.first << ", " << value.second << ")\n";
    }

    // In rollingHash
    cout << "rollingHash: " << rollingHash << "\n";

    // In map<pair<int, int>, int> nextBound
    cout << "nextBound:\n";
    for (const auto &[key, value] : nextBound)
    {
        cout << "(" << key.first << ", " << key.second << "): " << value << "\n";
    }

    // In vector<pair<int, int>> remember
    cout << "remember:\n";
    for (const auto &p : remember)
    {
        cout << "(" << p.first << ", " << p.second << ")\n";
    }
    cout << endl;

    dem++;
    int row = mousePosition.y / cellSize;
    int col = mousePosition.x / cellSize;

    // Kiểm tra xem vị trí nhấp chuột có hợp lệ không
    if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && board[row][col] == ' ' && !gameOver)
    {
        boardValue = evaluate(row, col, boardValue, -1, nextBound);
        updateBound(row, col, nextBound);
        currentI = row;
        currentJ = col;
        setState(row, col, turn);
        rollingHash ^= zobristTable[row][col][1];
        emptyCells -= 1;

        board[row][col] = playerSymbol;

        turn *= -1;

        moveHistory.push({row, col, playerSymbol, true});

        // Vẽ biểu tượng người chơi
        sf::Text marker;
        marker.setFont(font);
        marker.setString(playerSymbol);
        marker.setCharacterSize(cellSize / 2);
        marker.setFillColor(playerSymbol == 'X' ? sf::Color::Blue : sf::Color::Red);
        marker.setPosition(col * cellSize + cellSize / 4, row * cellSize + cellSize / 4);
        markers.push_back(marker);
        botCheck = false;

        // vector<sf::Vector2i> winningLine;
        //  Kiểm tra người chơi có thắng không
        if (checkWin(row, col, playerSymbol, botCheck, winningLine))
        {
            playerWinner = true;
            gameOver = true;
            gameDraw = false;
            return;
        }

        // Kiểm tra xem trò chơi có hòa không
        if (checkDraw())
        {
            gameOver = true;
            gameDraw = true;
            return;
        }

        // Bot thực hiện nước đi
        botMove();
    }
}
void GameBot::botMove()
{
    sf::Vector2i botMovePos;

    // Xử lý các mức độ khó của bot và chọn nước đi
    if (botDifficulty == "EASY")
    {
        botMovePos = easyBotMove();
    }
    else if (botDifficulty == "MEDIUM")
    {
        botMovePos = mediumBotMove();
    }
    else if (botDifficulty == "HARD")
    {
        botMovePos = hardBotMove();
    }

    int row = botMovePos.y;
    int col = botMovePos.x;

    // Kiểm tra tính hợp lệ của vị trí đã chọn
    if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && board[row][col] == ' ')
    {
        setState(row, col, turn);
        rollingHash ^= zobristTable[row][col][0];
        emptyCells -= 1;
        board[row][col] = botSymbol;
        turn *= -1;
        moveHistory.push({row, col, botSymbol, false});

        // Vẽ biểu tượng bot
        sf::Text marker;
        marker.setFont(font);
        marker.setString(botSymbol);
        marker.setCharacterSize(cellSize / 2);
        marker.setFillColor(botSymbol == 'X' ? sf::Color::Blue : sf::Color::Red);
        marker.setPosition(col * cellSize + cellSize / 4, row * cellSize + cellSize / 4);
        markers.push_back(marker);
    }

    botCheck = false;

    // Kiểm tra xem bot có thắng không
    if (checkWin(row, col, botSymbol, botCheck, winningLine))
    {
        playerWinner = false;
        gameOver = true;
        gameDraw = false;
        return;
    }

    // Kiểm tra xem trò chơi có hòa không
    if (checkDraw())
    {
        gameOver = true;
        gameDraw = true;
        return;
    }
}
sf::Vector2i GameBot::hardBotMove()
{
    sf::Vector2i move;
    int row, col;

    // In thông tin trước khi thực hiện Alpha-Beta Pruning
    // cout << "Starting Alpha-Beta Pruning..." << endl;

    // Thực hiện Alpha-Beta Pruning
    alphaBetaPruning(depth, boardValue, nextBound, INT_MIN, INT_MAX, true);

    // In thông tin về các giá trị sau khi chạy Alpha-Beta Pruning
    // cout << "Alpha-Beta Pruning complete. Evaluated bounds: " << nextBound.size() << " currentI: " << currentI << " currentJ: " << currentJ << endl;

    // Kiểm tra tính hợp lệ của nước đi
    if (isValidHard(currentI, currentJ, true))
    {
        row = currentI;
        col = currentJ;
        updateBound(row, col, nextBound);
        // cout << " isValidHard: true" << endl;
        // printBound(nextBound);
    }
    else
    {
        // Nếu nước đi không hợp lệ, tìm các vị trí khác có thể đi
        cout << "Invalid move detected. Searching for alternative moves..." << endl;
        updateBound(currentI, currentJ, nextBound);

        // Sắp xếp các bound theo giá trị
        vector<pair<pair<int, int>, int>> boundSorted(nextBound.begin(), nextBound.end());
        sort(boundSorted.begin(), boundSorted.end(), [](const auto &a, const auto &b)
             { return a.second > b.second; });
        // printBound(nextBound);
        cout << "Current boundSorted List:" << endl;
        for (const auto &entry : boundSorted)
        {
            cout << "Position: (" << entry.first.first << ", " << entry.first.second << ")"
                 << " - Value: " << entry.second << endl;
        }

        bool foundValidMove = false;
        // Chọn vị trí có giá trị cao nhất
        for (const auto &entry : boundSorted)
        {
            int candidateRow = entry.first.first;
            int candidateCol = entry.first.second;

            if (isValidHard(candidateRow, candidateCol, true))
            {
                row = candidateRow;
                col = candidateCol;
                foundValidMove = true;
                break;
            }
        }
        if (!foundValidMove)
        {
            cout << "No valid moves found. Defaulting to first bound entry..." << endl;
            // Nếu không tìm thấy nước đi hợp lệ, chọn nước đi đầu tiên
            row = boundSorted.front().first.first;
            col = boundSorted.front().first.second;
        }

        // Cập nhật lại các giá trị vị trí của bot
        currentI = row;
        currentJ = col;
        boundSorted.clear();
    }

    // Trả về kết quả
    move.x = col;
    move.y = row;
    // cout << "Selected move: (" << row << ", " << col << ")" << endl;

    return move;
}

void GameBot::printBound(map<pair<int, int>, int> &bound)
{
    cout << "Current Bound List:" << endl;
    for (const auto &entry : bound)
    {
        cout << "Position: (" << entry.first.first << ", " << entry.first.second << ")"
             << " - Value: " << entry.second << endl;
    }
}

map<vector<int>, int> GameBot::create_pattern_dict()
{
    map<vector<int>, int> patternDict;
    int x = -1;

    while (x < 2)
    {
        int y = -x;
        if (boardSize == 3)
        {
            patternDict[{x, x, x}] = 1000000 * x;
            patternDict[{x, x, 0}] = 100000 * x;
            patternDict[{0, x, x}] = 100000 * x;
            patternDict[{x, 0, x}] = 100000 * x;
            patternDict[{0, x, 0}] = 5000 * x;
            patternDict[{x, 0, 0}] = 1000 * x;
            patternDict[{0, x, 0}] = 1000 * x;
            patternDict[{0, 0, x}] = 1000 * x;
            patternDict[{x, x, y}] = -10 * x;
            patternDict[{x, y, x}] = -10 * x;
        }
        else
        {
            patternDict[{x, x, x, x, x}] = 1000000 * x;
            patternDict[{0, x, x, x, x, 0}] = 100000 * x;
            patternDict[{0, x, x, x, 0, x, 0}] = 100000 * x;
            patternDict[{0, x, 0, x, x, x, 0}] = 100000 * x;
            patternDict[{0, x, x, 0, x, x, 0}] = 100000 * x;
            patternDict[{0, x, x, x, x, y}] = 10000 * x;
            patternDict[{y, x, x, x, x, 0}] = 10000 * x;
            patternDict[{y, x, x, x, x, y}] = -10 * x;
            patternDict[{0, x, x, x, 0}] = 1000 * x;
            patternDict[{0, x, 0, x, x, 0}] = 1000 * x;
            patternDict[{0, x, x, 0, x, 0}] = 1000 * x;
            patternDict[{0, 0, x, x, x, y}] = 100 * x;
            patternDict[{y, x, x, x, 0, 0}] = 100 * x;
            patternDict[{0, x, 0, x, x, y}] = 100 * x;
            patternDict[{y, x, x, 0, x, 0}] = 100 * x;
            patternDict[{0, x, x, 0, x, y}] = 100 * x;
            patternDict[{y, x, 0, x, x, 0}] = 100 * x;
            patternDict[{x, 0, 0, x, x}] = 100 * x;
            patternDict[{x, x, 0, 0, x}] = 100 * x;
            patternDict[{x, 0, x, 0, x}] = 100 * x;
            patternDict[{y, 0, x, x, x, 0, y}] = 100 * x;
            patternDict[{y, x, x, x, y}] = -10 * x;
            patternDict[{0, 0, x, x, 0}] = 100 * x;
            patternDict[{0, x, x, 0, 0}] = 100 * x;
            patternDict[{0, x, 0, x, 0}] = 100 * x;
            patternDict[{0, x, 0, 0, x, 0}] = 100 * x;
            patternDict[{0, 0, 0, x, x, y}] = 10 * x;
            patternDict[{y, x, x, 0, 0, 0}] = 10 * x;
            patternDict[{0, 0, x, 0, x, y}] = 10 * x;
            patternDict[{y, x, 0, x, 0, 0}] = 10 * x;
            patternDict[{0, x, 0, 0, x, y}] = 10 * x;
            patternDict[{y, x, 0, 0, x, 0}] = 10 * x;
            patternDict[{x, 0, 0, 0, x}] = 10 * x;
            patternDict[{y, 0, x, 0, x, 0, y}] = 10 * x;
            patternDict[{y, 0, x, x, 0, 0, y}] = 10 * x;
            patternDict[{y, 0, 0, x, x, 0, y}] = 10 * x;
            patternDict[{y, x, x, y}] = -10 * x;
        }
        x += 2;
    }
    return patternDict;
}

bool GameBot::isValidHard(int i, int j, bool state)
{

    if (i < 0 || i >= boardSize || j < 0 || j >= boardSize)
    {
        return false;
    }

    if (state && IntBoard[i][j] != 0)
    {
        return false;
    }

    return true;
}

void GameBot::setState(int x, int y, int state)
{
    assert(state == -1 || state == 0 || state == 1);
    // Cập nhật giá trị trên bảng
    IntBoard[x][y] = state;
    // Cập nhật trạng thái cuối cùng đã chơi
    lastPlayed = state;
}

int GameBot::countDirection(int x, int y, int xdir, int ydir, int state)
{

    int count = 0;
    for (int step = 1; step <= 4; ++step)
    {
        int newX = x + xdir * step;
        int newY = y + ydir * step;

        // Kiểm tra biên của bảng
        if (newX < 0 || newX >= boardSize || newY < 0 || newY >= boardSize)
        {
            break;
        }

        // Kiểm tra trạng thái ô
        if (IntBoard[newX][newY] == state)
        {
            count++;
        }
        else
        {
            break;
        }
    }

    return count;
}

bool GameBot::isFive(int i, int j, int state)
{
    // Các hướng kiểm tra (ngang, dọc, chéo)
    int directions[4][2][2] = {
        {{-1, 0}, {1, 0}},  // Hướng dọc
        {{0, -1}, {0, 1}},  // Hướng ngang
        {{-1, 1}, {1, -1}}, // Hướng chéo lên
        {{-1, -1}, {1, 1}}  // Hướng chéo xuống
    };

    for (int dir = 0; dir < 4; ++dir)
    {
        int axis_count = 1; // Đếm cả ô đang xét

        for (int d = 0; d < 2; ++d)
        {
            int xdir = directions[dir][d][0];
            int ydir = directions[dir][d][1];

            int count = countDirection(i, j, xdir, ydir, state);
            axis_count += count;

            if (axis_count >= 5)
            {
                return true;
            }
        }
    }

    return false;
}

vector<pair<int, int>> GameBot::childNodes(map<pair<int, int>, int> &bound)
{
    // In nội dung ban đầu của bound

    // Chuyển bound sang vector để sắp xếp
    static vector<pair<pair<int, int>, int>> sortedNodes;

    sortedNodes.assign(bound.begin(), bound.end());
    sort(sortedNodes.begin(), sortedNodes.end(), [](const auto &a, const auto &b)
         { return a.second > b.second; });

    // Trích xuất danh sách vị trí
    vector<pair<int, int>> positions;
    for (const auto &node : sortedNodes)
    {
        positions.push_back(node.first);
    }

    // In danh sách vị trí trả về

    return positions;
}

void GameBot::updateBound(int new_i, int new_j, map<pair<int, int>, int> &bound)
{
    pair<int, int> played = {new_i, new_j};

    // In thông tin về vị trí được chơi

    // Kiểm tra và xóa vị trí đã chơi khỏi bound
    if (bound.find(played) != bound.end())
    {
        bound.erase(played);
    }

    // Các hướng cần kiểm tra
    vector<pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, 1}, {1, -1}, {-1, -1}, {1, 1}};
    for (auto &dir : directions)
    {
        int new_row = new_i + dir.first;
        int new_col = new_j + dir.second;

        // Kiểm tra ô xung quanh có hợp lệ và chưa tồn tại trong bound
        if (isValidHard(new_row, new_col, true) && bound.find({new_row, new_col}) == bound.end())
        {
            bound[{new_row, new_col}] = 0; // Giá trị ban đầu là 0
        }
    }
}

int GameBot::countPattern(int i_0, int j_0, vector<int> pattern, int score, map<pair<int, int>, int> &bound, int flag)
{
    // Đếm số lượng mẫu một chiều trên bàn cờ
    vector<pair<int, int>> directions = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}};
    int length = pattern.size();
    int count = 0;

    for (auto &dir : directions)
    {

        int steps_back = 0;

        if (dir.first * dir.second == 0)
        {
            steps_back = dir.first * min(5, j_0) + dir.second * min(5, i_0);
        }
        else if (dir.first == 1)
        {
            steps_back = min(5, min(j_0, i_0));
        }
        else
        {
            steps_back = min(5, min(boardSize - 1 - j_0, i_0));
        }

        int i_start = i_0 - steps_back * dir.second;
        int j_start = j_0 - steps_back * dir.first;

        int z = 0;
        while (z <= steps_back)
        {
            int i_new = i_start + z * dir.second;
            int j_new = j_start + z * dir.first;
            int index = 0;
            vector<pair<int, int>> remember;

            while (index < length && isValidHard(i_new, j_new, false) && IntBoard[i_new][j_new] == pattern[index])
            {

                if (isValidHard(i_new, j_new, true))
                {
                    remember.push_back({i_new, j_new});
                }

                i_new += dir.second;
                j_new += dir.first;
                index++;
            }

            if (index == length)
            {
                count++;
                for (auto &pos : remember)
                {
                    if (bound.find(pos) == bound.end())
                    {
                        bound[pos] = 0;
                    }
                    bound[pos] += flag * score;
                }
                z += index; // Skip the length of the pattern
            }
            else
            {
                z++;
            }
        }
    }

    return count;
}

int GameBot::evaluate(int new_i, int new_j, int board_value, int turn, map<pair<int, int>, int> &bound)
{
    // Đánh giá giá trị của bảng sau mỗi lượt đi
    int value_before = 0;
    int value_after = 0;

    for (auto &pattern : patternDict)
    {
        const auto &patternVector = pattern.first;
        int score = pattern.second;

        // Tính giá trị trước khi cập nhật bàn cờ
        int count_before = countPattern(new_i, new_j, patternVector, abs(score), bound, -1);
        value_before += count_before * score;

        // Cập nhật bảng để kiểm tra giá trị sau
        IntBoard[new_i][new_j] = turn;

        int count_after = countPattern(new_i, new_j, patternVector, abs(score), bound, 1);
        value_after += count_after * score;

        // Đặt lại ô để trả về trạng thái trước đó
        IntBoard[new_i][new_j] = 0;
    }

    int final_value = board_value + value_after - value_before;

    return final_value;
}

void GameBot::initZobrist()
{
    zobristTable = vector<vector<vector<uint64_t>>>(boardSize, vector<vector<uint64_t>>(boardSize, vector<uint64_t>(2)));

    for (int i = 0; i < boardSize; ++i)
    {
        for (int j = 0; j < boardSize; ++j)
        {
            zobristTable[i][j][0] = static_cast<uint64_t>(rand()) << 32 | rand();
            zobristTable[i][j][1] = static_cast<uint64_t>(rand()) << 32 | rand();
        }
    }
}

void GameBot::updateTTable(map<uint64_t, pair<int, int>> &TTable, uint64_t hash, int score, int depth)
{

    // Hiển thị thông tin chi tiết về giá trị băm và các thông số

    // Cập nhật TTable với các giá trị mới
    TTable[hash] = {score, depth};
}

optional<int> GameBot::checkResult()
{
    if (isFive(currentI, currentJ, lastPlayed) && (lastPlayed == -1 || lastPlayed == 1))
    {
        return lastPlayed; // 1 bot thang, -1 nguoi thang
    }
    else if (emptyCells <= 0)
    {
        return 0; // hoa
    }
    else
    {
        return nullopt; // t;
    }
}

int GameBot::alphaBetaPruning(int depth, int board_value, map<pair<int, int>, int> &bound, int alpha, int beta, bool maximizingPlayer)
{

    if (depth <= 0 || checkResult() != nullopt) // Điều kiện dừng đệ quy
    {
        return board_value;
    }

    // Kiểm tra bảng tra cứu (Transposition Table)
    if (TTable.find(rollingHash) != TTable.end() && TTable[rollingHash].second >= depth)
    {
        return TTable[rollingHash].first;
    }

    if (maximizingPlayer) // Tìm nước đi tối ưu cho người chơi tối đa hóa
    {
        int max_val = INT_MIN;
        vector<pair<int, int>> children = childNodes(bound);
        for (auto &child : children)
        {
            int i = child.first, j = child.second;
            if (i >= 0 && i < boardSize && j >= 0 && j < boardSize)
            {
                map<pair<int, int>, int> new_bound = bound;
                int new_val = evaluate(i, j, board_value, 1, new_bound);

                IntBoard[i][j] = 1;
                rollingHash ^= zobristTable[i][j][0];
                updateBound(i, j, new_bound);

                // Đệ quy gọi alphaBetaPruning cho lượt tiếp theo
                int eval = alphaBetaPruning(depth - 1, new_val, new_bound, alpha, beta, false);

                // Cập nhật giá trị tối đa
                if (eval > max_val)
                {
                    max_val = eval;
                    if (depth == this->depth) // Cập nhật nước đi của bot nếu là ở độ sâu ban đầu
                    {
                        currentI = i;
                        currentJ = j;
                        boardValue = eval;
                        nextBound = new_bound;
                    }
                }

                alpha = max(alpha, eval);
                IntBoard[i][j] = 0;
                rollingHash ^= zobristTable[i][j][0];

                // Cắt tỉa Alpha-Beta
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        updateTTable(TTable, rollingHash, max_val, depth);
        return max_val;
    }
    else // Tìm nước đi tối ưu cho người chơi tối thiểu hóa
    {
        int min_val = INT_MAX;
        vector<pair<int, int>> children = childNodes(bound);

        for (auto &child : children)
        {
            int i = child.first, j = child.second;
            if (i >= 0 && i < boardSize && j >= 0 && j < boardSize)
            {
                map<pair<int, int>, int> new_bound = bound;
                int new_val = evaluate(i, j, board_value, -1, new_bound);

                IntBoard[i][j] = -1;
                rollingHash ^= zobristTable[i][j][1];
                updateBound(i, j, new_bound);

                // Đệ quy gọi alphaBetaPruning cho lượt tiếp theo
                int eval = alphaBetaPruning(depth - 1, new_val, new_bound, alpha, beta, true);

                // Cập nhật giá trị tối thiểu
                if (eval < min_val)
                {
                    min_val = eval;
                    if (depth == this->depth) // Cập nhật nước đi của bot nếu là ở độ sâu ban đầu
                    {
                        currentI = i;
                        currentJ = j;
                        boardValue = eval;
                        nextBound = new_bound;
                    }
                }

                beta = min(beta, eval);
                IntBoard[i][j] = 0;
                rollingHash ^= zobristTable[i][j][1];

                // Cắt tỉa Alpha-Beta
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        updateTTable(TTable, rollingHash, min_val, depth);
        return min_val;
    }
}
