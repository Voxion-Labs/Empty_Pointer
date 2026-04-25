#pragma once

#include "entity.h"
#include "raylib.h"

#include <vector>

enum GameState
{
    MENU,
    PLAYING,
    GAMEOVER
};

class Game
{
public:
    Game();
    Game(int screenWidth, int screenHeight, const char* title);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool ShouldClose() const;
    void Tick();

private:
    struct Player
    {
        int gridX;
        int gridY;
        Vector2 drawPosition;
        Vector2 targetPosition;
        Color color;
    };

    struct Particle
    {
        Vector2 position;
        Vector2 velocity;
        float size;
        float life;
        float maxLife;
        Color color;
    };

    void ResetGame();
    void StartGame();
    void TriggerGameOver();

    void Update(float deltaTime);
    void UpdateMenu();
    void UpdatePlaying(float deltaTime);
    void UpdateGameOver(float deltaTime);
    void UpdateParticles(float deltaTime);

    void Draw() const;
    void DrawGrid() const;
    void DrawPlayer() const;
    void DrawParticles() const;
    void DrawButton(Rectangle bounds, const char* text) const;
    void DrawCenteredText(const char* text, int y, int fontSize, Color color) const;

    bool WasActionPressed() const;
    void TryMovePlayer(int dx, int dy);
    void SpawnEnemy();
    void SpawnDeathParticles();
    bool IsPlayerReadyForThreats() const;

    Rectangle GetMenuButtonBounds() const;
    Rectangle GetGameOverButtonBounds() const;
    Rectangle GetPlayerBounds() const;
    Vector2 GetPlayerCenter() const;
    Vector2 GridToWorld(int gridX, int gridY) const;

    GameState state_;
    int screenWidth_;
    int screenHeight_;
    int columns_;
    int rows_;
    float enemySpawnTimer_;
    float enemySpawnInterval_;
    float survivalTime_;
    Player player_;
    std::vector<Enemy> enemies_;
    std::vector<Particle> particles_;
};
