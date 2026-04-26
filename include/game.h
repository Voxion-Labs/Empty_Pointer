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
    void RequestStartOrRestart();
    void RequestToggleGuide();
    void RequestTogglePause();
    void RequestAttack();
    void RequestMainMenu();
    void RequestUnlockAudio();

private:
    struct Player
    {
        int gridX;
        int gridY;
        Vector2 drawPosition;
        Vector2 targetPosition;
        Color color;
        float moveTimer;
        float attackCooldown;
        float attackFlashTimer;
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
    void ClearRecords();
    void StartGame();
    void TriggerGameOver();

    void Update(float deltaTime);
    void UpdateMenu();
    void UpdatePlaying(float deltaTime);
    void UpdateGameOver(float deltaTime);
    void UpdateParticles(float deltaTime);
    void UpdatePauseAndGuideInput();
    void UpdatePlayerInput(float deltaTime);
    void UpdatePointerInput();

    void Draw() const;
    void DrawGrid() const;
    void DrawPlayer() const;
    void DrawParticles() const;
    void DrawButton(Rectangle bounds, const char* text) const;
    void DrawCenteredText(const char* text, int y, int fontSize, Color color) const;
    void DrawHud() const;
    void DrawGuideOverlay() const;
    void DrawPausedOverlay() const;

    bool WasActionPressed() const;
    bool WasButtonPressed(Rectangle bounds) const;
    void TryMovePlayer(int dx, int dy);
    void Attack();
    void SpawnEnemy();
    void SpawnBossEncounter();
    void SpawnDeathParticles();
    void SpawnEnemyParticles(Vector2 center);
    bool IsPlayerReadyForThreats() const;
    bool IsEnemyInPulseRange(Rectangle enemyBounds, Vector2 playerCenter) const;
    bool IsPlayerInsideBossAura(const Enemy& enemy) const;
    bool HasActiveBoss() const;
    void SetPaused(bool paused);
    void SyncPauseUi() const;
    void EnsureAudio();
    void InitializeAudio();
    void ShutdownAudio();
    void PlayUiSound();
    void PlayMoveSound();
    void PlayPulseSound();
    void PlayEnemyHitSound();
    void PlayGameOverSound();

    Rectangle GetMenuButtonBounds() const;
    Rectangle GetGameOverButtonBounds() const;
    Rectangle GetGuideButtonBounds() const;
    Rectangle GetPauseMenuButtonBounds() const;
    Rectangle GetPlayerBounds() const;
    Vector2 GetPlayerCenter() const;
    Vector2 GridToWorld(int gridX, int gridY) const;
    bool IsPointerOverUi(Vector2 pointer) const;

    GameState state_;
    int screenWidth_;
    int screenHeight_;
    int columns_;
    int rows_;
    float enemySpawnTimer_;
    float enemySpawnInterval_;
    float survivalTime_;
    int score_;
    int kills_;
    int nextBossKillTarget_;
    float bestSurvivalTime_;
    int bestScore_;
    int bestKills_;
    float screenShakeTimer_;
    float screenShakeMagnitude_;
    float bossWarningTimer_;
    bool paused_;
    bool guideOpen_;
    bool inputConsumed_;
    bool touchDownLastFrame_;
    bool touchPressedThisFrame_;
    bool audioReady_;
    Sound uiSound_;
    Sound moveSound_;
    Sound pulseSound_;
    Sound hitSound_;
    Sound gameOverSound_;
    Player player_;
    std::vector<Enemy> enemies_;
    std::vector<Particle> particles_;
};
