#pragma once
#include "raylib.h"

// The player-controlled paddle at the bottom of the screen.
class Paddle {
public:
    Paddle();

    void reset();              // move back to the centre
    void update(float dt);     // read keyboard input and move
    void draw() const;

    Rectangle getRect() const { return rect; }

private:
    Rectangle rect{};
};
