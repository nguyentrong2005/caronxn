#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "MenuCarogame.h"
using namespace std;

class Game
{
public:
    Game(int boardSize, sf::Color boardColor, MenuCarogame &menu);
    void run();
    void showDrawScreen();

private:
    int boardSize;
    int cellSize;
    sf::Color boardColor;
    sf::RenderWindow window;
    char currentPlayer;
    bool gameOver = false;
    bool gameDraw = false;
    bool playPvBmode;
    string botDifficulty;

    MenuCarogame &menu;
    int emptyCells;

    sf::Font font;
    sf::Text winnerText;
    sf::Text playAgainText;
    sf::Text newGameText;

    std::vector<std::vector<char>> board;
    std::vector<sf::RectangleShape> gridLines;
    std::vector<sf::Text> markers;
    sf::VertexArray winLine;

    void processEvents();
    void render();
    void handleMouseClick(const sf::Vector2i &mousePosition);
    void resetGame();
    void openMenu();
    void showWinScreen(const std::string &winner);
    bool checkWin(int row, int col, char player);
    bool checkDraw();
};

#endif
