#include "Game.h"
#include "Config.h"
#include <algorithm>
#include <cmath>
#include <fstream>

namespace {
    // One colour per row of bricks, top to bottom
    const Color ROW_COLORS[Config::MAX_ROWS] = {
        RED, ORANGE, GOLD, GREEN, SKYBLUE, BLUE, PURPLE, PINK
    };

    const Color BACKGROUND = {18, 18, 30, 255};

    // Random float in [min, max]
    float randomFloat(float min, float max) {
        return min + (max - min) * (GetRandomValue(0, 10000) / 10000.0f);
    }

    // Draw text centred horizontally on the screen
    void drawCentered(const char* text, int y, int fontSize, Color color) {
        int width = MeasureText(text, fontSize);
        DrawText(text, (Config::SCREEN_WIDTH - width) / 2, y, fontSize, color);
    }
}

Game::Game() {
    loadHighScore();
    loadLevel(1);   // so the bricks are visible behind the title screen
}

// ============================================================================
// Game flow
// ============================================================================

void Game::startNewGame() {
    score = 0;
    lives = Config::STARTING_LIVES;
    newHighScore = false;
    particles.clear();
    loadLevel(1);
    state = GameState::Playing;
}

void Game::loadLevel(int levelNumber) {
    level = levelNumber;

    // Each level: faster ball, more rows, more tough (2-hit) bricks
    ballSpeed = std::min(Config::BALL_BASE_SPEED + (level - 1) * Config::BALL_SPEEDUP_PER_LVL,
                         Config::BALL_MAX_SPEED);
    int rows      = std::min(4 + level, Config::MAX_ROWS);
    int toughRows = std::min(level - 1, 3);

    float brickWidth = (Config::SCREEN_WIDTH - 2 * Config::SIDE_MARGIN
                        - (Config::BRICK_COLUMNS - 1) * Config::BRICK_GAP) / Config::BRICK_COLUMNS;

    bricks.clear();
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < Config::BRICK_COLUMNS; ++col) {
            Brick brick;
            brick.rect = {
                Config::SIDE_MARGIN + col * (brickWidth + Config::BRICK_GAP),
                Config::BRICK_TOP + row * (Config::BRICK_HEIGHT + Config::BRICK_GAP),
                brickWidth,
                Config::BRICK_HEIGHT
            };
            brick.color    = ROW_COLORS[row];
            brick.maxHits  = (row < toughRows) ? 2 : 1;
            brick.hitsLeft = brick.maxHits;
            brick.points   = (rows - row) * 10 * brick.maxHits;  // higher rows are worth more
            bricks.push_back(brick);
        }
    }

    paddle.reset();
    ball.stickTo(paddle.getRect());
}

void Game::loseLife() {
    spawnParticles({ball.position.x, Config::SCREEN_HEIGHT - 5.0f}, RED, 25);
    --lives;

    if (lives <= 0) {
        if (score > highScore) {
            highScore = score;
            newHighScore = true;
            saveHighScore();
        }
        state = GameState::GameOver;
    } else {
        ball.stickTo(paddle.getRect());
    }
}

// ============================================================================
// Update (called once per frame)
// ============================================================================

void Game::update(float dt) {
    // If the window is dragged or the game hitches, dt can spike and the ball
    // could teleport through bricks. Capping it keeps the physics stable.
    dt = std::min(dt, 1.0f / 30.0f);

    updateParticles(dt);

    switch (state) {
        case GameState::Title:
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) startNewGame();
            break;

        case GameState::Playing:
            if (IsKeyPressed(KEY_P)) { state = GameState::Paused; break; }

            paddle.update(dt);

            if (ball.isStuck()) {
                ball.stickTo(paddle.getRect());   // ride along with the paddle
                if (IsKeyPressed(KEY_SPACE)) ball.launch(ballSpeed);
            } else {
                ball.update(dt);
                handlePaddleCollision();
                handleBrickCollisions();
                if (ball.fellOffScreen()) loseLife();
            }

            if (state == GameState::Playing && bricksRemaining() == 0) {
                state = GameState::LevelComplete;
                stateTimer = 2.0f;
            }
            break;

        case GameState::Paused:
            if (IsKeyPressed(KEY_P)) state = GameState::Playing;
            break;

        case GameState::LevelComplete:
            stateTimer -= dt;
            if (stateTimer <= 0.0f) {
                loadLevel(level + 1);
                state = GameState::Playing;
            }
            break;

        case GameState::GameOver:
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) startNewGame();
            break;
    }
}

// ============================================================================
// Collisions
// ============================================================================

void Game::handlePaddleCollision() {
    Rectangle p = paddle.getRect();

    // Only bounce if the ball is moving down (prevents double-bounces)
    if (ball.velocity.y <= 0) return;
    if (!CheckCollisionCircleRec(ball.position, ball.radius, p)) return;

    // Where on the paddle did it hit? -1 = far left edge, 0 = centre, +1 = far right edge.
    float paddleCenter = p.x + p.width / 2.0f;
    float hitOffset = (ball.position.x - paddleCenter) / (p.width / 2.0f);
    hitOffset = std::clamp(hitOffset, -1.0f, 1.0f);

    // Turn that into a bounce angle. This is what lets the player aim.
    float angle = hitOffset * Config::MAX_BOUNCE_ANGLE;
    ball.velocity = {ballSpeed * std::sin(angle), -ballSpeed * std::cos(angle)};
    ball.position.y = p.y - ball.radius;
}

void Game::handleBrickCollisions() {
    for (Brick& brick : bricks) {
        if (brick.hitsLeft <= 0) continue;
        if (!CheckCollisionCircleRec(ball.position, ball.radius, brick.rect)) continue;

        ball.bounceOff(brick.rect);
        --brick.hitsLeft;

        Vector2 center = {brick.rect.x + brick.rect.width / 2, brick.rect.y + brick.rect.height / 2};
        if (brick.hitsLeft == 0) {
            score += brick.points;
            spawnParticles(center, brick.color, 14);
        } else {
            spawnParticles(center, LIGHTGRAY, 5);   // cracked but not broken
        }

        // Only handle one brick per frame so the bounce direction stays predictable
        break;
    }
}

int Game::bricksRemaining() const {
    return static_cast<int>(std::count_if(bricks.begin(), bricks.end(),
                                          [](const Brick& b) { return b.hitsLeft > 0; }));
}

// ============================================================================
// Particles
// ============================================================================

void Game::spawnParticles(Vector2 where, Color color, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = randomFloat(0.0f, 2.0f * PI);
        float speed = randomFloat(60.0f, 240.0f);
        float life  = randomFloat(0.4f, 0.9f);
        particles.push_back({
            where,
            {std::cos(angle) * speed, std::sin(angle) * speed},
            life, life, color
        });
    }
}

void Game::updateParticles(float dt) {
    for (Particle& p : particles) {
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.velocity.y += 400.0f * dt;   // gravity
        p.life -= dt;
    }
    // Erase-remove idiom: delete every particle whose life has run out
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p) { return p.life <= 0.0f; }),
        particles.end());
}

// ============================================================================
// High score
// ============================================================================

void Game::loadHighScore() {
    std::ifstream file(Config::HIGHSCORE_FILE);
    if (!(file >> highScore)) highScore = 0;   // no file yet (first run) -> 0
}

void Game::saveHighScore() const {
    std::ofstream file(Config::HIGHSCORE_FILE);
    file << highScore;
}

// ============================================================================
// Drawing
// ============================================================================

void Game::draw() const {
    ClearBackground(BACKGROUND);

    // Bricks
    for (const Brick& brick : bricks) {
        if (brick.hitsLeft <= 0) continue;
        bool damaged = brick.hitsLeft < brick.maxHits;
        DrawRectangleRec(brick.rect, damaged ? Fade(brick.color, 0.45f) : brick.color);
        if (brick.maxHits > 1 && !damaged) {
            DrawRectangleLinesEx(brick.rect, 2.0f, Fade(WHITE, 0.7f));  // mark tough bricks
        }
    }

    paddle.draw();
    ball.draw();

    for (const Particle& p : particles) {
        DrawCircleV(p.position, 2.5f, Fade(p.color, p.life / p.maxLife));
    }

    drawHud();

    switch (state) {
        case GameState::Title:
            drawOverlay("BRICKFALL", "Press SPACE to start");
            drawCentered("LEFT/RIGHT or A/D to move     SPACE to launch     P to pause",
                         Config::SCREEN_HEIGHT / 2 + 60, 18, GRAY);
            break;
        case GameState::Playing:
            if (ball.isStuck()) {
                drawCentered("Press SPACE to launch", Config::SCREEN_HEIGHT / 2 + 60, 20, LIGHTGRAY);
            }
            break;
        case GameState::Paused:
            drawOverlay("PAUSED", "Press P to resume");
            break;
        case GameState::LevelComplete:
            drawOverlay(TextFormat("LEVEL %d CLEAR!", level), "Get ready...");
            break;
        case GameState::GameOver:
            drawOverlay("GAME OVER", "Press SPACE to play again");
            drawCentered(newHighScore ? TextFormat("NEW HIGH SCORE: %d", score)
                                      : TextFormat("Final score: %d", score),
                         Config::SCREEN_HEIGHT / 2 + 60, 24, newHighScore ? GOLD : LIGHTGRAY);
            break;
    }

    DrawFPS(Config::SCREEN_WIDTH - 90, Config::SCREEN_HEIGHT - 25);
}

void Game::drawHud() const {
    DrawRectangle(0, 0, Config::SCREEN_WIDTH, static_cast<int>(Config::HUD_HEIGHT), {30, 30, 48, 255});

    DrawText(TextFormat("SCORE %d", score), 16, 10, 20, RAYWHITE);
    drawCentered(TextFormat("LEVEL %d", level), 10, 20, GOLD);
    DrawText(TextFormat("BEST %d", highScore), 200, 10, 20, GRAY);

    // Lives shown as little balls in the top-right
    for (int i = 0; i < lives; ++i) {
        DrawCircle(Config::SCREEN_WIDTH - 24 - i * 22, 20, 7, RAYWHITE);
    }
}

void Game::drawOverlay(const char* title, const char* subtitle) const {
    DrawRectangle(0, 0, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, Fade(BLACK, 0.6f));
    drawCentered(title, Config::SCREEN_HEIGHT / 2 - 50, 50, RAYWHITE);
    drawCentered(subtitle, Config::SCREEN_HEIGHT / 2 + 15, 22, LIGHTGRAY);
}
