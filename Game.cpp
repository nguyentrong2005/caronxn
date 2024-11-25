#include "Game.h"
#include <stdexcept>
#include <iostream>

Game::Game(int boardSize, sf::Color boardColor, MenuCarogame &menu)
    : boardSize(boardSize),
      boardColor(boardColor),
      window(sf::VideoMode(1100, 1100), "Caro Game"),
      currentPlayer('X'),
      menu(menu)
{
    if (!font.loadFromFile("D:/DATA STRUCTURE & ALGORITHM_EXERCISES/Project/caro_nxn/assets/fonts/Roboto/Roboto-Medium.ttf"))
    {
        throw std::runtime_error("Failed to load font.");
    }

    cellSize = window.getSize().x / boardSize;
    board.resize(boardSize, std::vector<char>(boardSize, ' '));

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
}

void Game::run()
{
    while (window.isOpen())
    {
        processEvents();
        if (!window.isOpen())
            break;
        render();
    }
}

void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
            return;
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
                    openMenu();
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

void Game::openMenu()
{
    window.close();
    MenuCarogame newMenu;
    newMenu.run();
}

void Game::render()
{
    window.clear(boardColor);

    float boardAreaSize = std::min(window.getSize().x, window.getSize().y);
    cellSize = boardAreaSize / boardSize;

    for (int row = 0; row < boardSize; ++row)
    {
        for (int col = 0; col < boardSize; ++col)
        {
            sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
            cell.setPosition(col * cellSize, row * cellSize);
            if (boardColor == sf::Color::Black)
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
        else
        {
            showWinScreen(std::string("Player ") + currentPlayer);
        }
    }

    window.display();
}

void Game::handleMouseClick(const sf::Vector2i &mousePosition)
{
    int row = mousePosition.y / cellSize;
    int col = mousePosition.x / cellSize;

    if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && board[row][col] == ' ' && !gameOver)
    {
        board[row][col] = currentPlayer;

        sf::Text marker;
        marker.setFont(font);
        marker.setString(currentPlayer);
        marker.setCharacterSize(cellSize / 2);
        marker.setFillColor(currentPlayer == 'X' ? sf::Color::Blue : sf::Color::Red);
        marker.setPosition(col * cellSize + cellSize / 4, row * cellSize + cellSize / 4);
        markers.push_back(marker);

        if (checkWin(row, col, currentPlayer))
        {
            gameOver = true;
            gameDraw = false;
            return;
        }

        if (checkDraw())
        {
            gameOver = true;
            gameDraw = true;
            return;
        }

        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }
}

bool Game::checkDraw()
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

bool Game::checkWin(int row, int col, char player)
{
    int winCondition = (boardSize <= 4) ? 3 : 5;

    auto checkDirection = [&](int dr, int dc)
    {
        int count = 1;

        for (int step = 1; step < winCondition; ++step)
        {
            int nr = row + step * dr, nc = col + step * dc;
            if (nr >= 0 && nr < boardSize && nc >= 0 && nc < boardSize && board[nr][nc] == player)
                ++count;
            else
                break;
        }

        for (int step = 1; step < winCondition; ++step)
        {
            int nr = row - step * dr, nc = col - step * dc;
            if (nr >= 0 && nr < boardSize && nc >= 0 && nc < boardSize && board[nr][nc] == player)
                ++count;
            else
                break;
        }

        return count >= winCondition;
    };

    if (checkDirection(1, 0) || checkDirection(0, 1) || checkDirection(1, 1) || checkDirection(1, -1))
    {
        gameOver = true;
        return true;
    }
    return false;
}

void Game::showWinScreen(const std::string &winner)
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
    else
    {
        winnerText.setString(winner + " Wins!");
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

void Game::resetGame()
{
    board.assign(boardSize, std::vector<char>(boardSize, ' '));
    markers.clear();
    gameOver = false;
    currentPlayer = 'X';
    winLine = sf::VertexArray();
}
