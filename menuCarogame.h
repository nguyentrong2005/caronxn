#ifndef MENU_CARO_GAME_H
#define MENU_CARO_GAME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
using namespace std;

class MenuCarogame
{
private:
    // SFML window and graphical elements
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    // UI Elements
    sf::Text titleText;
    sf::Text colorText;
    sf::RectangleShape BlackSquare;
    sf::RectangleShape whiteSquare;
    sf::Text sizeText;
    sf::ConvexShape downTriangle;
    sf::ConvexShape upTriangle;
    sf::RectangleShape sizeBox;

    // Buttons
    sf::RectangleShape PvPButton;
    sf::Text PvPButtonText;
    sf::RectangleShape PvBButton;
    sf::Text PvBButtonText;

    // State variables
    bool blackSelected;
    int boardSize;
    bool playGameRequested = false;
    bool playPvBmode = false;

    // Private methods
    void processEvents();
    void update();
    void render();
    void handleMouseClick(const sf::Vector2i &mousePosition);

public:
    // Constructor
    MenuCarogame();

    // Main loop
    void run();

    // Getters
    sf::Color getBoardColor() const;
    int getBoardSize() const;
    bool isPlayGameRequested() const;
    bool isPlayPvBmode() const;
};

#endif // MENU_CARO_GAME_H
