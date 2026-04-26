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

#ifdef PLATFORM_WEB
extern "C"
{
    EMSCRIPTEN_KEEPALIVE void EP_Start()
    {
        g_game->RequestStartOrRestart();
    }

    EMSCRIPTEN_KEEPALIVE void EP_Guide()
    {
        g_game->RequestToggleGuide();
    }

    EMSCRIPTEN_KEEPALIVE void EP_Pause()
    {
        g_game->RequestTogglePause();
    }

    EMSCRIPTEN_KEEPALIVE void EP_Attack()
    {
        g_game->RequestAttack();
    }

    EMSCRIPTEN_KEEPALIVE void EP_MainMenu()
    {
        g_game->RequestMainMenu();
    }

    EMSCRIPTEN_KEEPALIVE void EP_UnlockAudio()
    {
        g_game->RequestUnlockAudio();
    }
}
#endif

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
