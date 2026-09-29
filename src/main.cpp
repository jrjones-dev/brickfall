#include "raylib.h"
#include "Config.h"
#include "Game.h"

int main() {
    InitWindow(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, "Brickfall");
    SetTargetFPS(Config::TARGET_FPS);

    Game game;

    // The classic game loop: update the world, then draw it, every frame.
    while (!WindowShouldClose()) {       // ESC or the window's close button quits
        game.update(GetFrameTime());     // seconds since the last frame

        BeginDrawing();
        game.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
