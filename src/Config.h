#pragma once

// All the "tuning knobs" for the game live here, so gameplay can be
// tweaked without hunting through the code.
namespace Config {
    // Window
    constexpr int   SCREEN_WIDTH  = 800;
    constexpr int   SCREEN_HEIGHT = 600;
    constexpr int   TARGET_FPS    = 60;
    constexpr float HUD_HEIGHT    = 40.0f;   // top strip for score/lives; acts as the ceiling

    // Paddle
    constexpr float PADDLE_WIDTH  = 100.0f;
    constexpr float PADDLE_HEIGHT = 14.0f;
    constexpr float PADDLE_SPEED  = 540.0f;  // pixels per second
    constexpr float PADDLE_Y      = SCREEN_HEIGHT - 50.0f;

    // Ball
    constexpr float BALL_RADIUS          = 7.0f;
    constexpr float BALL_BASE_SPEED      = 360.0f;  // pixels per second on level 1
    constexpr float BALL_SPEEDUP_PER_LVL = 40.0f;
    constexpr float BALL_MAX_SPEED       = 640.0f;
    constexpr float MAX_BOUNCE_ANGLE     = 1.0472f; // 60 degrees, in radians

    // Bricks
    constexpr int   BRICK_COLUMNS = 10;
    constexpr int   MAX_ROWS      = 8;
    constexpr float BRICK_HEIGHT  = 22.0f;
    constexpr float BRICK_GAP     = 4.0f;
    constexpr float BRICK_TOP     = 70.0f;
    constexpr float SIDE_MARGIN   = 20.0f;

    // Rules
    constexpr int STARTING_LIVES = 3;
    constexpr const char* HIGHSCORE_FILE = "highscore.txt";
}
