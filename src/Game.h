#pragma once
#include "raylib.h"
#include "Paddle.h"
#include "Ball.h"
#include <vector>

// Which "screen" the game is on. A small state machine drives the whole game.
enum class GameState {
    Title,
    Playing,
    Paused,
    LevelComplete,
    GameOver
};

struct Brick {
    Rectangle rect;
    Color color;
    int hitsLeft;    // 0 = destroyed
    int maxHits;
    int points;
};

// A tiny spark that flies out when a brick breaks.
struct Particle {
    Vector2 position;
    Vector2 velocity;
    float life;      // seconds remaining
    float maxLife;
    Color color;
};

class Game {
public:
    Game();

    void update(float dt);
    void draw() const;

private:
    // Game flow
    void startNewGame();
    void loadLevel(int levelNumber);
    void loseLife();

    // Collisions
    void handlePaddleCollision();
    void handleBrickCollisions();
    int  bricksRemaining() const;

    // Effects
    void spawnParticles(Vector2 where, Color color, int count);
    void updateParticles(float dt);

    // High score (saved to a text file between runs)
    void loadHighScore();
    void saveHighScore() const;

    // Drawing helpers
    void drawHud() const;
    void drawOverlay(const char* title, const char* subtitle) const;

    GameState state = GameState::Title;
    Paddle paddle;
    Ball ball;
    std::vector<Brick> bricks;
    std::vector<Particle> particles;

    int score = 0;
    int highScore = 0;
    int lives = 0;
    int level = 1;
    float ballSpeed = 0.0f;
    float stateTimer = 0.0f;   // countdown used by the "Level Complete" screen
    bool newHighScore = false;
};
