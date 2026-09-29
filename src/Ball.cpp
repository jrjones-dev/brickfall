#include "Ball.h"
#include "Config.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr size_t TRAIL_LENGTH = 8;
}

Ball::Ball() : radius(Config::BALL_RADIUS) {}

void Ball::stickTo(const Rectangle& paddle) {
    stuck = true;
    velocity = {0.0f, 0.0f};
    position = {paddle.x + paddle.width / 2.0f, paddle.y - radius - 1.0f};
    trail.clear();
}

void Ball::launch(float speed) {
    stuck = false;
    // Pick a random angle between -30 and +30 degrees from straight up
    float angle = GetRandomValue(-30, 30) * DEG2RAD;
    velocity = {speed * std::sin(angle), -speed * std::cos(angle)};
}

void Ball::update(float dt) {
    if (stuck) return;

    trail.push_front(position);
    if (trail.size() > TRAIL_LENGTH) trail.pop_back();

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    // Walls. We set the sign of the velocity (instead of just flipping it)
    // so the ball can never get "stuck" vibrating inside a wall.
    if (position.x - radius < 0) {
        position.x = radius;
        velocity.x = std::fabs(velocity.x);
    }
    if (position.x + radius > Config::SCREEN_WIDTH) {
        position.x = Config::SCREEN_WIDTH - radius;
        velocity.x = -std::fabs(velocity.x);
    }
    if (position.y - radius < Config::HUD_HEIGHT) {
        position.y = Config::HUD_HEIGHT + radius;
        velocity.y = std::fabs(velocity.y);
    }
}

void Ball::bounceOff(const Rectangle& rect) {
    // How far the ball has pushed into the rectangle from each side.
    float overlapLeft   = (position.x + radius) - rect.x;
    float overlapRight  = (rect.x + rect.width) - (position.x - radius);
    float overlapTop    = (position.y + radius) - rect.y;
    float overlapBottom = (rect.y + rect.height) - (position.y - radius);

    float minX = std::min(overlapLeft, overlapRight);
    float minY = std::min(overlapTop, overlapBottom);

    // The side with the smallest overlap is the side we hit.
    if (minX < minY) {
        if (overlapLeft < overlapRight) { position.x -= overlapLeft;  velocity.x = -std::fabs(velocity.x); }
        else                            { position.x += overlapRight; velocity.x =  std::fabs(velocity.x); }
    } else {
        if (overlapTop < overlapBottom) { position.y -= overlapTop;    velocity.y = -std::fabs(velocity.y); }
        else                            { position.y += overlapBottom; velocity.y =  std::fabs(velocity.y); }
    }
}

bool Ball::fellOffScreen() const {
    return position.y - radius > Config::SCREEN_HEIGHT;
}

void Ball::draw() const {
    // Trail: older positions are smaller and more transparent
    for (size_t i = 0; i < trail.size(); ++i) {
        float t = 1.0f - static_cast<float>(i + 1) / (TRAIL_LENGTH + 1);
        DrawCircleV(trail[i], radius * t, Fade(SKYBLUE, 0.35f * t));
    }
    DrawCircleV(position, radius, RAYWHITE);
}
