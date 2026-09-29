#pragma once
#include "raylib.h"
#include <deque>

// The ball. It starts "stuck" to the paddle until the player launches it.
class Ball {
public:
    Ball();

    void stickTo(const Rectangle& paddle);  // sit on top of the paddle, waiting to launch
    void launch(float speed);               // fire upward at a slightly random angle
    void update(float dt);                  // move and bounce off the walls
    void bounceOff(const Rectangle& rect);  // reflect off a brick
    void draw() const;

    bool isStuck() const { return stuck; }
    bool fellOffScreen() const;

    // Kept public because Game needs to read/adjust them during collisions.
    Vector2 position{};
    Vector2 velocity{};
    float radius;

private:
    bool stuck = true;
    std::deque<Vector2> trail;   // recent positions, used to draw a motion trail
};
