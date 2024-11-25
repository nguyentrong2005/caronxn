#ifndef MENU_CARO_GAME_H
#define MENU_CARO_GAME_H

#include <SFML/Graphics.hpp>
#include <stdexcept>
#include <string>

class MenuCarogame
{
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Text titleText;
    sf::Text colorText;
    sf::RectangleShape BlackSquare;
    sf::RectangleShape whiteSquare;
    sf::Text sizeText;
    sf::ConvexShape downTriangle;
    sf::ConvexShape upTriangle;
    sf::RectangleShape sizeBox;
    sf::RectangleShape playButton;
    sf::Text playButtonText;
    bool blackSelected;
    int boardSize;
    bool playGameRequested;

    void processEvents();
    void update();
    void render();
    void handleMouseClick(const sf::Vector2i &mousePosition);

public:
    MenuCarogame();
    void run();
    sf::Color getBoardColor() const;
    int getBoardSize() const;
    bool isPlayGameRequested() const;
};

#endif
