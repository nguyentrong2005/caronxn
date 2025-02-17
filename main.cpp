#include "MenuCarogame.h"
#include "Game.h"
#include "GameBot.h"
#include <iostream>
using namespace std;

int main()
{
    while (true)
    {
        MenuCarogame menu;
        menu.run();

        if (menu.isPlayGameRequested())
        {
            Game game(menu.getBoardSize(), menu.getBoardColor(), menu);
            game.run();
        }
        else if (menu.isPlayPvBmode())
        {
            GameBot gamebot(menu.getBoardSize(), menu.getBoardColor(), menu);
            gamebot.run();
        }
        else
        {
            break;
        }
    }
    return 0;
}
