#include "game.h"

#include "math_utils.h"

#include <cmath>

namespace
{
    constexpr int kCellSize = 40;
    constexpr float kPlayerPadding = 6.0f;
    constexpr float kMoveLerpSpeed = 14.0f;
    constexpr float kSnapDistanceSqr = 0.04f;

    constexpr float kEnemySize = 28.0f;
    constexpr float kEnemyBaseSpeed = 92.0f;
    constexpr float kEnemySpeedRamp = 4.0f;
    constexpr float kInitialSpawnInterval = 1.15f;
    constexpr float kMinSpawnInterval = 0.34f;

    constexpr float kParticleMinSpeed = 90.0f;
    constexpr float kParticleMaxSpeed = 260.0f;
    constexpr float kParticleLife = 0.72f;
    constexpr float kParticleDrag = 6.0f;

    constexpr Color kBackground = { 18, 18, 24, 255 };
    constexpr Color kGridLine = { 42, 44, 54, 255 };
    constexpr Color kPlayer = { 60, 210, 170, 255 };
    constexpr Color kPlayerAccent = { 165, 255, 225, 255 };
    constexpr Color kText = { 232, 236, 244, 255 };
    constexpr Color kMutedText = { 150, 156, 170, 255 };
    constexpr Color kDanger = { 255, 92, 108, 255 };
}

Game::Game()
    : Game(800, 600, "Empty_Pointer")
{
}

Game::Game(int screenWidth, int screenHeight, const char* title)
    : state_(MENU),
      screenWidth_(screenWidth),
      screenHeight_(screenHeight),
      columns_(screenWidth / kCellSize),
      rows_(screenHeight / kCellSize),
      enemySpawnTimer_(0.0f),
      enemySpawnInterval_(kInitialSpawnInterval),
      survivalTime_(0.0f),
      player_{ columns_ / 2, rows_ / 2, {}, {}, kPlayer },
      enemies_(),
      particles_()
{
    InitWindow(screenWidth_, screenHeight_, title);
    SetTargetFPS(60);

    ResetGame();
    state_ = MENU;
}

Game::~Game()
{
    if (IsWindowReady())
    {
        CloseWindow();
    }
}

bool Game::ShouldClose() const
{
    return WindowShouldClose();
}

void Game::Tick()
{
    Update(GetFrameTime());
    Draw();
}

void Game::ResetGame()
{
    player_.gridX = columns_ / 2;
    player_.gridY = rows_ / 2;
    player_.color = kPlayer;
    player_.targetPosition = GridToWorld(player_.gridX, player_.gridY);
    player_.drawPosition = player_.targetPosition;

    enemySpawnTimer_ = 0.0f;
    enemySpawnInterval_ = kInitialSpawnInterval;
    survivalTime_ = 0.0f;
    enemies_.clear();
    particles_.clear();
}

void Game::StartGame()
{
    ResetGame();
    state_ = PLAYING;
}

void Game::TriggerGameOver()
{
    if (state_ == GAMEOVER)
    {
        return;
    }

    SpawnDeathParticles();
    state_ = GAMEOVER;
}

void Game::Update(float deltaTime)
{
    switch (state_)
    {
    case MENU:
        UpdateMenu();
        break;
    case PLAYING:
        UpdatePlaying(deltaTime);
        break;
    case GAMEOVER:
        UpdateGameOver(deltaTime);
        break;
    }
}

void Game::UpdateMenu()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        StartGame();
    }
}

void Game::UpdatePlaying(float deltaTime)
{
    int dx = 0;
    int dy = 0;

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        dx = -1;
    }
    else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        dx = 1;
    }
    else if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        dy = -1;
    }
    else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        dy = 1;
    }

    if (dx != 0 || dy != 0)
    {
        TryMovePlayer(dx, dy);
    }

    player_.drawPosition = math::Lerp(
        player_.drawPosition,
        player_.targetPosition,
        deltaTime * kMoveLerpSpeed
    );

    if (math::LengthSqr(math::Sub(player_.targetPosition, player_.drawPosition)) <= kSnapDistanceSqr)
    {
        player_.drawPosition = player_.targetPosition;
    }

    survivalTime_ += deltaTime;
    enemySpawnTimer_ += deltaTime;
    enemySpawnInterval_ = kInitialSpawnInterval - survivalTime_ * 0.018f;
    if (enemySpawnInterval_ < kMinSpawnInterval)
    {
        enemySpawnInterval_ = kMinSpawnInterval;
    }

    while (enemySpawnTimer_ >= enemySpawnInterval_)
    {
        enemySpawnTimer_ -= enemySpawnInterval_;
        SpawnEnemy();
    }

    const Vector2 playerCenter = GetPlayerCenter();
    const Rectangle playerBounds = GetPlayerBounds();
    for (Enemy& enemy : enemies_)
    {
        enemy.Update(playerCenter, deltaTime);

        if (CheckCollisionRecs(playerBounds, enemy.GetBounds()))
        {
            TriggerGameOver();
            break;
        }
    }
}

void Game::UpdateGameOver(float deltaTime)
{
    UpdateParticles(deltaTime);

    if (IsKeyPressed(KEY_ENTER))
    {
        StartGame();
    }
}

void Game::UpdateParticles(float deltaTime)
{
    for (Particle& particle : particles_)
    {
        particle.life -= deltaTime;
        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
        particle.velocity = math::Lerp(particle.velocity, { 0.0f, 0.0f }, deltaTime * kParticleDrag);
    }

    for (int i = static_cast<int>(particles_.size()) - 1; i >= 0; --i)
    {
        if (particles_[i].life <= 0.0f)
        {
            particles_.erase(particles_.begin() + i);
        }
    }
}

void Game::Draw() const
{
    BeginDrawing();
    ClearBackground(kBackground);

    DrawGrid();

    for (const Enemy& enemy : enemies_)
    {
        enemy.Draw();
    }

    if (state_ != GAMEOVER)
    {
        DrawPlayer();
    }

    DrawParticles();

    switch (state_)
    {
    case MENU:
        DrawCenteredText("EMPTY_POINTER", screenHeight_ / 2 - 72, 44, kText);
        DrawCenteredText("Press ENTER to Start", screenHeight_ / 2, 24, kMutedText);
        break;
    case PLAYING:
        DrawText(TextFormat("TIME %.1f", survivalTime_), 18, 16, 22, kText);
        break;
    case GAMEOVER:
        DrawCenteredText("GAME OVER", screenHeight_ / 2 - 64, 48, kDanger);
        DrawCenteredText(TextFormat("Survived %.1f seconds", survivalTime_), screenHeight_ / 2 - 8, 24, kText);
        DrawCenteredText("Press ENTER to Restart", screenHeight_ / 2 + 34, 22, kMutedText);
        break;
    }

    EndDrawing();
}

void Game::DrawGrid() const
{
    for (int x = 0; x <= screenWidth_; x += kCellSize)
    {
        DrawLine(x, 0, x, screenHeight_, kGridLine);
    }

    for (int y = 0; y <= screenHeight_; y += kCellSize)
    {
        DrawLine(0, y, screenWidth_, y, kGridLine);
    }
}

void Game::DrawPlayer() const
{
    const Rectangle playerRect = GetPlayerBounds();
    DrawRectangleRec(playerRect, player_.color);
    DrawRectangleLinesEx(playerRect, 2.0f, kPlayerAccent);
}

void Game::DrawParticles() const
{
    for (const Particle& particle : particles_)
    {
        Color faded = particle.color;
        faded.a = static_cast<unsigned char>(255.0f * (particle.life / particle.maxLife));
        DrawRectangleV(particle.position, { particle.size, particle.size }, faded);
    }
}

void Game::DrawCenteredText(const char* text, int y, int fontSize, Color color) const
{
    const int width = MeasureText(text, fontSize);
    DrawText(text, screenWidth_ / 2 - width / 2, y, fontSize, color);
}

void Game::TryMovePlayer(int dx, int dy)
{
    const int nextGridX = player_.gridX + dx;
    const int nextGridY = player_.gridY + dy;

    if (nextGridX < 0 || nextGridX >= columns_ || nextGridY < 0 || nextGridY >= rows_)
    {
        return;
    }

    player_.gridX = nextGridX;
    player_.gridY = nextGridY;
    player_.targetPosition = GridToWorld(player_.gridX, player_.gridY);
}

void Game::SpawnEnemy()
{
    const int side = GetRandomValue(0, 3);
    int gridX = 0;
    int gridY = 0;

    switch (side)
    {
    case 0:
        gridX = GetRandomValue(0, columns_ - 1);
        gridY = 0;
        break;
    case 1:
        gridX = columns_ - 1;
        gridY = GetRandomValue(0, rows_ - 1);
        break;
    case 2:
        gridX = GetRandomValue(0, columns_ - 1);
        gridY = rows_ - 1;
        break;
    default:
        gridX = 0;
        gridY = GetRandomValue(0, rows_ - 1);
        break;
    }

    Vector2 spawnPosition = GridToWorld(gridX, gridY);
    spawnPosition.x += (kCellSize - kEnemySize) * 0.5f;
    spawnPosition.y += (kCellSize - kEnemySize) * 0.5f;

    enemies_.emplace_back(
        spawnPosition,
        kEnemySize,
        kEnemyBaseSpeed + survivalTime_ * kEnemySpeedRamp
    );
}

void Game::SpawnDeathParticles()
{
    const Vector2 center = GetPlayerCenter();
    particles_.clear();
    particles_.reserve(20);

    for (int i = 0; i < 20; ++i)
    {
        const float angle = static_cast<float>(GetRandomValue(0, 359)) * DEG2RAD;
        const float speed = static_cast<float>(GetRandomValue(
            static_cast<int>(kParticleMinSpeed),
            static_cast<int>(kParticleMaxSpeed)
        ));

        Particle particle = {};
        particle.position = center;
        particle.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };
        particle.size = static_cast<float>(GetRandomValue(3, 7));
        particle.life = kParticleLife;
        particle.maxLife = kParticleLife;
        particle.color = kPlayerAccent;

        particles_.push_back(particle);
    }
}

Rectangle Game::GetPlayerBounds() const
{
    return {
        player_.drawPosition.x + kPlayerPadding,
        player_.drawPosition.y + kPlayerPadding,
        static_cast<float>(kCellSize) - kPlayerPadding * 2.0f,
        static_cast<float>(kCellSize) - kPlayerPadding * 2.0f
    };
}

Vector2 Game::GetPlayerCenter() const
{
    const Rectangle bounds = GetPlayerBounds();
    return {
        bounds.x + bounds.width * 0.5f,
        bounds.y + bounds.height * 0.5f
    };
}

Vector2 Game::GridToWorld(int gridX, int gridY) const
{
    return {
        static_cast<float>(gridX * kCellSize),
        static_cast<float>(gridY * kCellSize)
    };
}
