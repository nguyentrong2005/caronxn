#ifndef GAMEBOT_H
#define GAMEBOT_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "MenuCarogame.h"
#include <tuple>
#include <stack>
#include <climits>
#include <iostream>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <optional>
#include <cassert>

using namespace std;

class GameBot
{
public:
    GameBot(int boardSize, sf::Color boardColor, MenuCarogame &menu);
    void run();
    void showDrawScreen();

private:
    //////////////////
    int boardSize;
    int cellSize;
    int maxDepth;
    int winCondition;
    sf::Color boardColor;
    sf::RenderWindow window;
    char botSymbol;
    char playerSymbol;
    bool gameOver = false;
    bool gameDraw = false;
    bool isDifficultySelected = false;
    bool isSymbolSelected = false;
    bool playerWinner = false;
    bool botCheck = false;
    bool dangerous = false;
    string botDifficulty;

    MenuCarogame &menu;

    sf::Font font;
    sf::Text winnerText;
    sf::Text playAgainText;
    sf::Text newGameText;

    vector<vector<char>> board;
    vector<vector<int>> IntBoard;
    vector<sf::RectangleShape> gridLines;
    vector<sf::Text> markers;
    sf::VertexArray winLine;
    sf::RectangleShape easyButton;
    sf::Text easyText;
    sf::RectangleShape mediumButton;
    sf::Text mediumText;
    sf::RectangleShape hardButton;
    sf::Text hardText;
    sf::RectangleShape xButton;
    sf::Text xText;
    sf::RectangleShape oButton;
    sf::Text oText;
    sf::RectangleShape undoButton;
    sf::Text undoText;

    sf::Vector2i easyBotMove();
    sf::Vector2i findBlockMove();
    sf::Vector2i findExtendMove();
    sf::Vector2i findSurroundMove();
    int countDirection(int row, int col, int dRow, int dCol, char symbol);
    bool isValid(int row, int col);
    sf::Vector2i mediumBotMove();
    sf::Vector2i getBestBlockingMove();
    sf::Vector2i findBlockingMove(int row, int col, const int dir[2], int count);
    int checkOpenEnds(int row, int col, const int dir[2], int length);
    sf::Vector2i detectAlmostFive();
    sf::Vector2i detectFourBlocked();
    sf::Vector2i detectThreeOpen();
    sf::Vector2i detectDoubleThree();
    sf::Vector2i detectFourThree();
    bool isGapPresent(int row, int col, const int dir[2]);
    bool isLShape(int row, int col, const int dir1[2], const int dir2[2]);
    sf::Vector2i detectTwoFourParallel();
    sf::Vector2i detectGapPair();
    sf::Vector2i detectLOrTShape();
    sf::Vector2i detectOpenTwo();
    sf::Vector2i detectTwoThreeCombo();
    sf::Vector2i getBestExtendMove();
    sf::Vector2i detectTwoConsecutive();
    sf::Vector2i hardBotMove();
    int adjustDepth();

    void processEvents();
    void render();
    void handleMouseClick(const sf::Vector2i &mousePosition);
    void resetGame();
    void showWinScreen(const string &winner);
    bool checkWin(int row, int col, char player, bool botCheck, vector<sf::Vector2i> &winningLine);
    bool checkDraw();
    void showDifficultySelection(sf::RenderWindow &window);
    void botMove();

    void undoMove();
    bool isUndoing = false;
    bool reset = false;
    //
    int emptyCells;
    int currentI, currentJ, depth;
    int boardValue;
    int lastPlayed;
    int turn;
    int dem;

    map<pair<int, int>, int> bound;
    vector<vector<vector<uint64_t>>> zobristTable;
    map<uint64_t, pair<int, int>> TTable;
    uint64_t rollingHash;
    map<pair<int, int>, int> nextBound;
    map<vector<int>, int> patternDict;
    vector<pair<int, int>> remember;
    vector<sf::Vector2i> winningLine;

    map<vector<int>, int> create_pattern_dict();
    bool isValidHard(int i, int j, bool state = true);
    void setState(int x, int y, int state);
    int countDirection(int x, int y, int xdir, int ydir, int state);
    bool isFive(int i, int j, int state);
    vector<pair<int, int>> childNodes(map<pair<int, int>, int> &bound);
    void updateBound(int new_i, int new_j, map<pair<int, int>, int> &bound);
    int countPattern(int i_0, int j_0, vector<int> pattern, int score, map<pair<int, int>, int> &bound, int flag);
    int evaluate(int new_i, int new_j, int board_value, int turn, map<pair<int, int>, int> &bound);
    void initZobrist();
    void updateTTable(map<uint64_t, pair<int, int>> &TTable, uint64_t hash, int score, int depth);
    optional<int> checkResult();
    int alphaBetaPruning(int depth, int board_value, map<pair<int, int>, int> &bound, int alpha, int beta, bool maximizingPlayer);
    void printBound(map<pair<int, int>, int> &bound);
};

#endif
