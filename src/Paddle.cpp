#include "Paddle.h"
#include "Config.h"
#include <algorithm>

Paddle::Paddle() {
    reset();
}

void Paddle::reset() {
    rect = {
        (Config::SCREEN_WIDTH - Config::PADDLE_WIDTH) / 2.0f,
        Config::PADDLE_Y,
        Config::PADDLE_WIDTH,
        Config::PADDLE_HEIGHT
    };
}

void Paddle::update(float dt) {
    // -1 = left, +1 = right, 0 = not moving (or both keys held)
    float direction = 0.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) direction -= 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) direction += 1.0f;

    // Multiplying by dt (seconds since last frame) makes movement
    // frame-rate independent: same speed at 30 FPS or 144 FPS.
    rect.x += direction * Config::PADDLE_SPEED * dt;

    // Keep the paddle on screen
    rect.x = std::clamp(rect.x, 0.0f, Config::SCREEN_WIDTH - rect.width);
}

void Paddle::draw() const {
    DrawRectangleRounded(rect, 0.6f, 8, RAYWHITE);
}
