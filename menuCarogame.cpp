#include "menuCarogame.h"
#include <iostream>

MenuCarogame::MenuCarogame()
    : window(sf::VideoMode(1600, 1100), "Menu Caro Game"),
      blackSelected(true), boardSize(3), playGameRequested(false)
{
    if (!font.loadFromFile("D:/DATA STRUCTURE & ALGORITHM_EXERCISES/Project/caro_nxn/assets/fonts/Roboto/Roboto-Medium.ttf"))
    {
        throw std::runtime_error("Failed to load font.");
    }

    if (!backgroundTexture.loadFromFile("D:/DATA STRUCTURE & ALGORITHM_EXERCISES/Project/caro_nxn/assets/images/background.jpg"))
    {
        std::cerr << "Warning: Failed to load background image. Using default background color." << std::endl;
        backgroundSprite.setColor(sf::Color(100, 100, 100));
    }
    else
    {
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
            window.getSize().x / float(backgroundTexture.getSize().x),
            window.getSize().y / float(backgroundTexture.getSize().y));
    }

    titleText.setFont(font);
    titleText.setString("Carogame nxn");
    titleText.setCharacterSize(100);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(
        (window.getSize().x - titleText.getGlobalBounds().width) / 2,
        50);

    colorText.setFont(font);
    colorText.setString("Please select game board color");
    colorText.setCharacterSize(50);
    colorText.setFillColor(sf::Color::White);
    colorText.setPosition(
        (window.getSize().x - colorText.getGlobalBounds().width) / 2,
        200);

    float squareYOffset = 300;
    BlackSquare.setSize(sf::Vector2f(120, 120));
    BlackSquare.setPosition(window.getSize().x / 2 - 150, squareYOffset);
    BlackSquare.setFillColor(sf::Color::Black);

    whiteSquare.setSize(sf::Vector2f(120, 120));
    whiteSquare.setPosition(window.getSize().x / 2 + 30, squareYOffset);
    whiteSquare.setFillColor(sf::Color::White);

    float sizeTextX = 450;
    float sizeTextY = 500;
    float sizeBoxX = 800;
    float sizeBoxY = 500;
    float triangleOffset = 60;

    sizeText.setFont(font);
    sizeText.setString("Board size:");
    sizeText.setCharacterSize(50);
    sizeText.setFillColor(sf::Color::White);
    sizeText.setPosition(sizeTextX, sizeTextY);

    sizeBox.setSize(sf::Vector2f(120, 60));
    sizeBox.setPosition(sizeBoxX, sizeBoxY);
    sizeBox.setFillColor(sf::Color::White);

    downTriangle.setPointCount(3);
    downTriangle.setPoint(0, sf::Vector2f(sizeBoxX - triangleOffset, sizeBoxY + 15));
    downTriangle.setPoint(1, sf::Vector2f(sizeBoxX - triangleOffset + 20, sizeBoxY + 45));
    downTriangle.setPoint(2, sf::Vector2f(sizeBoxX - triangleOffset + 40, sizeBoxY + 15));
    downTriangle.setFillColor(sf::Color::Magenta);

    upTriangle.setPointCount(3);
    upTriangle.setPoint(0, sf::Vector2f(sizeBoxX + sizeBox.getSize().x + triangleOffset - 40, sizeBoxY + 45));
    upTriangle.setPoint(1, sf::Vector2f(sizeBoxX + sizeBox.getSize().x + triangleOffset - 20, sizeBoxY + 15));
    upTriangle.setPoint(2, sf::Vector2f(sizeBoxX + sizeBox.getSize().x + triangleOffset, sizeBoxY + 45));
    upTriangle.setFillColor(sf::Color::Magenta);

    playButton.setSize(sf::Vector2f(300, 80));
    playButton.setPosition(
        (window.getSize().x - playButton.getSize().x) / 2,
        750);
    playButton.setFillColor(sf::Color::Green);

    playButtonText.setFont(font);
    playButtonText.setString("Play");
    playButtonText.setCharacterSize(40);
    playButtonText.setFillColor(sf::Color::White);
    playButtonText.setPosition(
        playButton.getPosition().x + (playButton.getSize().x - playButtonText.getGlobalBounds().width) / 2,
        playButton.getPosition().y + (playButton.getSize().y - playButtonText.getGlobalBounds().height) / 2 - 5);
}

void MenuCarogame::run()
{
    playGameRequested = false;
    while (window.isOpen())
    {
        processEvents();
        update();
        render();
    }
}

sf::Color MenuCarogame::getBoardColor() const
{
    return blackSelected ? sf::Color::Black : sf::Color::White;
}

int MenuCarogame::getBoardSize() const
{
    return boardSize;
}

bool MenuCarogame::isPlayGameRequested() const
{
    return playGameRequested;
}

void MenuCarogame::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
            return;
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            handleMouseClick(sf::Mouse::getPosition(window));
        }
    }
}

void MenuCarogame::update()
{
    BlackSquare.setOutlineThickness(blackSelected ? 3.f : 0.f);
    whiteSquare.setOutlineThickness(!blackSelected ? 3.f : 0.f);
    BlackSquare.setOutlineColor(sf::Color::Red);
    whiteSquare.setOutlineColor(sf::Color::Red);
}

void MenuCarogame::render()
{
    window.clear();
    window.draw(backgroundSprite);
    window.draw(titleText);
    window.draw(colorText);
    window.draw(BlackSquare);
    window.draw(whiteSquare);
    window.draw(sizeText);
    window.draw(downTriangle);
    window.draw(upTriangle);
    window.draw(sizeBox);
    window.draw(playButton);
    window.draw(playButtonText);

    sf::Text sizeNumber;
    sizeNumber.setFont(font);
    sizeNumber.setString(std::to_string(boardSize));
    sizeNumber.setCharacterSize(30);
    sizeNumber.setFillColor(sf::Color::Black);
    sizeNumber.setPosition(sizeBox.getPosition().x + 30, sizeBox.getPosition().y + 10);
    window.draw(sizeNumber);

    window.display();
}

void MenuCarogame::handleMouseClick(const sf::Vector2i &mousePosition)
{
    if (BlackSquare.getGlobalBounds().contains(mousePosition.x, mousePosition.y))
    {
        blackSelected = true;
    }
    else if (whiteSquare.getGlobalBounds().contains(mousePosition.x, mousePosition.y))
    {
        blackSelected = false;
    }

    if (downTriangle.getGlobalBounds().contains(mousePosition.x, mousePosition.y) && boardSize > 3)
    {
        boardSize--;
    }
    else if (upTriangle.getGlobalBounds().contains(mousePosition.x, mousePosition.y) && boardSize < 100)
    {
        boardSize++;
    }

    if (playButton.getGlobalBounds().contains(mousePosition.x, mousePosition.y))
    {
        if (window.isOpen())
        {
            playGameRequested = true;
            window.close();
        }
    }
}
