#include "game.h"

#ifdef PLATFORM_WEB
    #include <emscripten/emscripten.h>
#endif

namespace
{
    Game* g_game = nullptr;

    void MainLoop()
    {
        g_game->Tick();
    }
}

int main()
{
    static Game game(800, 600, "Empty_Pointer");
    g_game = &game;

#ifdef PLATFORM_WEB
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (!game.ShouldClose())
    {
        MainLoop();
    }
#endif

    g_game = nullptr;
    return 0;
}
