#include "MenuCarogame.h"
#include "Game.h"

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
        else
        {
            break;
        }
    }
    return 0;
}
