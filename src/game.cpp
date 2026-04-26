#include "game.h"

#include "math_utils.h"

#include <cmath>
#include <vector>

#ifdef PLATFORM_WEB
    #include <emscripten/emscripten.h>
#endif

namespace
{
    constexpr int kCellSize = 40;
    constexpr float kPlayerPadding = 6.0f;
    constexpr float kMoveLerpSpeed = 14.0f;
    constexpr float kSnapDistanceSqr = 0.04f;
    constexpr float kHeldMoveInterval = 0.13f;

    constexpr float kEnemySize = 28.0f;
    constexpr float kEnemyBaseSpeed = 92.0f;
    constexpr float kEnemySpeedRamp = 4.0f;
    constexpr float kInitialSpawnInterval = 1.15f;
    constexpr float kMinSpawnInterval = 0.34f;
    constexpr float kOpeningGraceSeconds = 0.55f;
    constexpr int kBossKillInterval = 20;
    constexpr int kBossEscortCount = 5;
    constexpr float kBossSize = 68.0f;
    constexpr float kBossSpeed = 42.0f;
    constexpr float kBossAuraRadius = 62.0f;
    constexpr float kBossWarningTime = 3.0f;

    constexpr float kParticleMinSpeed = 90.0f;
    constexpr float kParticleMaxSpeed = 260.0f;
    constexpr float kParticleLife = 0.72f;
    constexpr float kParticleDrag = 6.0f;
    constexpr float kButtonWidth = 260.0f;
    constexpr float kButtonHeight = 52.0f;
    constexpr float kSmallButtonWidth = 112.0f;
    constexpr float kSmallButtonHeight = 38.0f;
    constexpr float kAttackRadius = 136.0f;
    constexpr float kAttackCooldown = 0.82f;
    constexpr float kAttackFlashTime = 0.24f;

    constexpr Color kBackground = { 18, 18, 24, 255 };
    constexpr Color kGridLine = { 42, 44, 54, 255 };
    constexpr Color kPlayer = { 60, 210, 170, 255 };
    constexpr Color kPlayerAccent = { 165, 255, 225, 255 };
    constexpr Color kText = { 232, 236, 244, 255 };
    constexpr Color kMutedText = { 150, 156, 170, 255 };
    constexpr Color kDanger = { 255, 92, 108, 255 };
    constexpr Color kBossWarning = { 255, 55, 72, 255 };
    constexpr Color kButton = { 38, 168, 142, 255 };
    constexpr Color kButtonHover = { 56, 208, 176, 255 };
    constexpr Color kButtonText = { 10, 16, 18, 255 };
    constexpr Color kOverlay = { 5, 6, 10, 245 };
    constexpr Color kGuidePanel = { 3, 4, 8, 255 };
    constexpr Color kAttackRing = { 165, 255, 225, 110 };

    Sound CreateToneSound(float frequency, float duration, float volume, float decay)
    {
        constexpr int sampleRate = 22050;
        constexpr float twoPi = 6.28318530718f;
        const int sampleCount = static_cast<int>(sampleRate * duration);
        std::vector<short> samples(static_cast<unsigned int>(sampleCount));

        for (int i = 0; i < sampleCount; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            const float envelope = std::exp(-decay * t);
            const float tone = std::sin(twoPi * frequency * t) * 0.72f
                + std::sin(twoPi * frequency * 2.0f * t) * 0.18f;
            samples[static_cast<unsigned int>(i)] = static_cast<short>(tone * envelope * volume * 32767.0f);
        }

        Wave wave = {};
        wave.frameCount = static_cast<unsigned int>(sampleCount);
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        wave.data = samples.data();

        return LoadSoundFromWave(wave);
    }
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
      score_(0),
      kills_(0),
      nextBossKillTarget_(kBossKillInterval),
      bestSurvivalTime_(0.0f),
      bestScore_(0),
      bestKills_(0),
      screenShakeTimer_(0.0f),
      screenShakeMagnitude_(0.0f),
      bossWarningTimer_(0.0f),
      paused_(false),
      guideOpen_(false),
      inputConsumed_(false),
      touchDownLastFrame_(false),
      touchPressedThisFrame_(false),
      audioReady_(false),
      uiSound_(),
      moveSound_(),
      pulseSound_(),
      hitSound_(),
      gameOverSound_(),
      bossWarningSound_(),
      player_{ columns_ / 2, rows_ / 2, {}, {}, kPlayer, 0.0f, 0.0f, 0.0f },
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
    ShutdownAudio();

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

void Game::RequestStartOrRestart()
{
    if (state_ == MENU || state_ == GAMEOVER)
    {
        PlayUiSound();
        StartGame();
    }
}

void Game::RequestToggleGuide()
{
    PlayUiSound();
    guideOpen_ = !guideOpen_;
    if (guideOpen_)
    {
        SetPaused(false);
    }
}

void Game::RequestTogglePause()
{
    if (state_ == PLAYING)
    {
        PlayUiSound();
        SetPaused(!paused_);
        guideOpen_ = false;
    }
}

void Game::RequestAttack()
{
    if (state_ == PLAYING && !paused_ && !guideOpen_)
    {
        Attack();
    }
}

void Game::RequestMainMenu()
{
    if (state_ == PLAYING && paused_)
    {
        PlayUiSound();
        ResetGame();
        ClearRecords();
        state_ = MENU;
    }
}

void Game::RequestUnlockAudio()
{
    EnsureAudio();
}

void Game::ResetGame()
{
    player_.gridX = columns_ / 2;
    player_.gridY = rows_ / 2;
    player_.color = kPlayer;
    player_.targetPosition = GridToWorld(player_.gridX, player_.gridY);
    player_.drawPosition = player_.targetPosition;
    player_.moveTimer = 0.0f;
    player_.attackCooldown = 0.0f;
    player_.attackFlashTimer = 0.0f;

    enemySpawnTimer_ = 0.0f;
    enemySpawnInterval_ = kInitialSpawnInterval;
    survivalTime_ = 0.0f;
    score_ = 0;
    kills_ = 0;
    nextBossKillTarget_ = kBossKillInterval;
    screenShakeTimer_ = 0.0f;
    screenShakeMagnitude_ = 0.0f;
    bossWarningTimer_ = 0.0f;
    SetPaused(false);
    guideOpen_ = false;
    inputConsumed_ = false;
    enemies_.clear();
    particles_.clear();
}

void Game::ClearRecords()
{
    bestSurvivalTime_ = 0.0f;
    bestScore_ = 0;
    bestKills_ = 0;
}

void Game::StartGame()
{
    ResetGame();
    enemySpawnTimer_ = -kOpeningGraceSeconds;
    state_ = PLAYING;
}

void Game::TriggerGameOver()
{
    if (state_ == GAMEOVER)
    {
        return;
    }

    SpawnDeathParticles();
    PlayGameOverSound();
    screenShakeTimer_ = 0.28f;
    screenShakeMagnitude_ = 8.0f;

    if (survivalTime_ > bestSurvivalTime_)
    {
        bestSurvivalTime_ = survivalTime_;
    }

    if (score_ > bestScore_)
    {
        bestScore_ = score_;
    }

    if (kills_ > bestKills_)
    {
        bestKills_ = kills_;
    }

    state_ = GAMEOVER;
}

void Game::Update(float deltaTime)
{
    const bool touchDown = GetTouchPointCount() > 0;
    touchPressedThisFrame_ = touchDown && !touchDownLastFrame_;
    touchDownLastFrame_ = touchDown;

    inputConsumed_ = false;
    UpdatePauseAndGuideInput();

    if (screenShakeTimer_ > 0.0f)
    {
        screenShakeTimer_ -= deltaTime;
        if (screenShakeTimer_ < 0.0f)
        {
            screenShakeTimer_ = 0.0f;
        }
    }

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
    if (inputConsumed_)
    {
        return;
    }

    if (WasActionPressed())
    {
        PlayUiSound();
        StartGame();
    }
}

void Game::UpdatePlaying(float deltaTime)
{
    if (paused_ || guideOpen_)
    {
        return;
    }

    UpdatePlayerInput(deltaTime);
    UpdatePointerInput();

    player_.drawPosition = math::Lerp(
        player_.drawPosition,
        player_.targetPosition,
        deltaTime * kMoveLerpSpeed
    );

    if (math::LengthSqr(math::Sub(player_.targetPosition, player_.drawPosition)) <= kSnapDistanceSqr)
    {
        player_.drawPosition = player_.targetPosition;
    }

    if (player_.attackCooldown > 0.0f)
    {
        player_.attackCooldown -= deltaTime;
    }

    if (player_.attackFlashTimer > 0.0f)
    {
        player_.attackFlashTimer -= deltaTime;
    }

    if (bossWarningTimer_ > 0.0f)
    {
        bossWarningTimer_ -= deltaTime;
        if (bossWarningTimer_ < 0.0f)
        {
            bossWarningTimer_ = 0.0f;
        }
    }

    if (kills_ >= nextBossKillTarget_ && !HasActiveBoss())
    {
        SpawnBossEncounter();
        nextBossKillTarget_ += kBossKillInterval;
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

        if (CheckCollisionRecs(playerBounds, enemy.GetBounds()) || IsPlayerInsideBossAura(enemy))
        {
            TriggerGameOver();
            break;
        }
    }
}

void Game::UpdateGameOver(float deltaTime)
{
    UpdateParticles(deltaTime);

    if (inputConsumed_)
    {
        return;
    }

    if (WasActionPressed())
    {
        PlayUiSound();
        StartGame();
    }
}

void Game::UpdatePauseAndGuideInput()
{
    if (WasButtonPressed(GetGuideButtonBounds()) || IsKeyPressed(KEY_G))
    {
        PlayUiSound();
        guideOpen_ = !guideOpen_;
        if (guideOpen_)
        {
            SetPaused(false);
        }
        inputConsumed_ = true;
        return;
    }

    if (guideOpen_ && (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || touchPressedThisFrame_))
    {
        PlayUiSound();
        guideOpen_ = false;
        inputConsumed_ = true;
        return;
    }

    if (state_ == PLAYING && IsKeyPressed(KEY_P))
    {
        PlayUiSound();
        SetPaused(!paused_);
        guideOpen_ = false;
        inputConsumed_ = true;
        return;
    }

    if (state_ == PLAYING && paused_ && WasButtonPressed(GetPauseMenuButtonBounds()))
    {
        PlayUiSound();
        ResetGame();
        ClearRecords();
        state_ = MENU;
        inputConsumed_ = true;
    }
}

void Game::UpdatePlayerInput(float deltaTime)
{
    int dx = 0;
    int dy = 0;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        dx = -1;
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        dx = 1;
    }
    else if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        dy = -1;
    }
    else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        dy = 1;
    }

    if (IsKeyPressed(KEY_E))
    {
        Attack();
    }

    if (dx == 0 && dy == 0)
    {
        player_.moveTimer = 0.0f;
        return;
    }

    player_.moveTimer -= deltaTime;
    if (player_.moveTimer <= 0.0f)
    {
        TryMovePlayer(dx, dy);
        player_.moveTimer = kHeldMoveInterval;
    }
}

void Game::UpdatePointerInput()
{
    bool hasPointerPress = false;
    Vector2 pointer = {};

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        hasPointerPress = true;
        pointer = GetMousePosition();
    }
    else if (GetTouchPointCount() > 0)
    {
        hasPointerPress = true;
        pointer = GetTouchPosition(0);
    }

    if (!hasPointerPress)
    {
        return;
    }

    if (IsPointerOverUi(pointer))
    {
        return;
    }

    const int gridX = static_cast<int>(pointer.x) / kCellSize;
    const int gridY = static_cast<int>(pointer.y) / kCellSize;

    if (gridX < 0 || gridX >= columns_ || gridY < 0 || gridY >= rows_)
    {
        return;
    }

    player_.gridX = gridX;
    player_.gridY = gridY;
    player_.targetPosition = GridToWorld(player_.gridX, player_.gridY);
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

    Camera2D camera = {};
    camera.zoom = 1.0f;
    if (screenShakeTimer_ > 0.0f)
    {
        const float power = screenShakeMagnitude_ * (screenShakeTimer_ / 0.28f);
        camera.offset = {
            static_cast<float>(GetRandomValue(-100, 100)) * 0.01f * power,
            static_cast<float>(GetRandomValue(-100, 100)) * 0.01f * power
        };
    }

    BeginMode2D(camera);

    DrawGrid();

    for (const Enemy& enemy : enemies_)
    {
        enemy.Draw();
    }

    if (state_ == PLAYING)
    {
        DrawPlayer();
    }

    DrawParticles();
    EndMode2D();

    DrawHud();
    DrawBossWarning();

    switch (state_)
    {
    case MENU:
        DrawCenteredText("EMPTY_POINTER", screenHeight_ / 2 - 72, 44, kText);
        DrawCenteredText("Press ENTER or click START", screenHeight_ / 2 - 14, 22, kMutedText);
        DrawButton(GetMenuButtonBounds(), "START");
        break;
    case PLAYING:
        DrawText(TextFormat("TIME %.1f", survivalTime_), 18, 16, 22, kText);
        break;
    case GAMEOVER:
        DrawCenteredText("GAME OVER", screenHeight_ / 2 - 64, 48, kDanger);
        DrawCenteredText(TextFormat("Survived %.1f seconds", survivalTime_), screenHeight_ / 2 - 8, 24, kText);
        DrawCenteredText(TextFormat("%d kills  |  %d points", kills_, score_), screenHeight_ / 2 + 24, 22, kText);
        DrawCenteredText("Press ENTER or click RESTART", screenHeight_ / 2 + 54, 20, kMutedText);
        DrawButton(GetGameOverButtonBounds(), "RESTART");
        break;
    }

    if (paused_)
    {
        DrawPausedOverlay();
    }

    if (guideOpen_)
    {
        DrawGuideOverlay();
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

    if (player_.attackFlashTimer > 0.0f)
    {
        DrawCircleLines(
            static_cast<int>(GetPlayerCenter().x),
            static_cast<int>(GetPlayerCenter().y),
            kAttackRadius,
            kAttackRing
        );
    }
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

void Game::DrawButton(Rectangle bounds, const char* text) const
{
    const bool hovering = CheckCollisionPointRec(GetMousePosition(), bounds);
    const Color fill = hovering ? kButtonHover : kButton;
    const int fontSize = 24;
    const int textWidth = MeasureText(text, fontSize);
    const int textX = static_cast<int>(bounds.x + bounds.width * 0.5f - textWidth * 0.5f);
    const int textY = static_cast<int>(bounds.y + bounds.height * 0.5f - fontSize * 0.5f);

    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 2.0f, kPlayerAccent);
    DrawText(text, textX, textY, fontSize, kButtonText);
}

void Game::DrawHud() const
{
    DrawButton(GetGuideButtonBounds(), guideOpen_ ? "CLOSE" : "GUIDE");

    if (state_ == PLAYING)
    {
        DrawText(TextFormat("SCORE %d", score_), 18, 44, 20, kText);
        DrawText(TextFormat("KILLS %d", kills_), 18, 70, 20, kText);
        DrawText(TextFormat("PULSE %.1f", player_.attackCooldown > 0.0f ? player_.attackCooldown : 0.0f), 18, 96, 18, kMutedText);
        DrawText("P: PAUSE", screenWidth_ - 116, 18, 18, kMutedText);
    }
    else if (state_ == GAMEOVER)
    {
        DrawText(TextFormat("BEST %.1fs  %d pts  %d kills", bestSurvivalTime_, bestScore_, bestKills_), 18, 18, 18, kMutedText);
    }
}

void Game::DrawBossWarning() const
{
    if (state_ != PLAYING || bossWarningTimer_ <= 0.0f)
    {
        return;
    }

    const Rectangle banner = {
        screenWidth_ - 318.0f,
        72.0f,
        300.0f,
        40.0f
    };

    DrawRectangleRec(banner, { 42, 8, 14, 245 });
    DrawRectangleLinesEx(banner, 2.0f, kBossWarning);
    DrawText("WARNING: BOSS INCOMING", static_cast<int>(banner.x + 15.0f), static_cast<int>(banner.y + 11.0f), 18, kBossWarning);
}

void Game::DrawGuideOverlay() const
{
    const Rectangle panel = { 70.0f, 70.0f, 660.0f, 460.0f };
    DrawRectangle(0, 0, screenWidth_, screenHeight_, kGuidePanel);
    DrawRectangleRec(panel, kGuidePanel);
    DrawRectangleLinesEx(panel, 2.0f, kPlayerAccent);

    DrawCenteredText("HOW TO PLAY", 98, 30, kText);
    DrawText("Start / restart: ENTER, SPACE, or START / RESTART.", 104, 148, 18, kText);
    DrawText("Move: hold WASD or the keyboard arrow keys.", 104, 180, 18, kText);
    DrawText("Mouse / touch: tap a grid cell to move there.", 104, 212, 18, kText);
    DrawText("Mobile: use the browser control dock below the game.", 104, 244, 18, kText);
    DrawText("Pulse: press E or PULSE to clear nearby enemies.", 104, 276, 18, kText);
    DrawText("Pulse has a cooldown, so save it for close enemies.", 104, 308, 18, kMutedText);
    DrawText("Boss: every 20 kills, avoid the purple aura.", 104, 340, 18, kText);
    DrawText("Boss needs two pulses and arrives with five escorts.", 104, 372, 18, kText);
    DrawText("Stats: records clear on main menu.", 104, 404, 18, kText);
    DrawText("Pause: press P or PAUSE. Sounds unlock after input.", 104, 436, 18, kMutedText);
    DrawText("Tap anywhere or press ESC to close.", 104, 466, 18, kMutedText);
}

void Game::DrawPausedOverlay() const
{
    DrawRectangle(0, 0, screenWidth_, screenHeight_, kOverlay);
    DrawCenteredText("PAUSED", screenHeight_ / 2 - 74, 42, kText);
    DrawCenteredText("Press P to resume", screenHeight_ / 2 - 18, 22, kMutedText);
    DrawButton(GetPauseMenuButtonBounds(), "MAIN MENU");
}

void Game::DrawCenteredText(const char* text, int y, int fontSize, Color color) const
{
    const int width = MeasureText(text, fontSize);
    DrawText(text, screenWidth_ / 2 - width / 2, y, fontSize, color);
}

bool Game::WasActionPressed() const
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        return true;
    }

    if (state_ == MENU)
    {
        return WasButtonPressed(GetMenuButtonBounds());
    }

    if (state_ == GAMEOVER)
    {
        return WasButtonPressed(GetGameOverButtonBounds());
    }

    return false;
}

bool Game::WasButtonPressed(Rectangle bounds) const
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), bounds))
    {
        return true;
    }

    if (!touchPressedThisFrame_)
    {
        return false;
    }

    for (int i = 0; i < GetTouchPointCount(); ++i)
    {
        if (CheckCollisionPointRec(GetTouchPosition(i), bounds))
        {
            return true;
        }
    }

    return false;
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
    PlayMoveSound();
}

void Game::Attack()
{
    if (player_.attackCooldown > 0.0f)
    {
        return;
    }

    const Vector2 playerCenter = GetPlayerCenter();
    player_.attackCooldown = kAttackCooldown;
    player_.attackFlashTimer = kAttackFlashTime;
    PlayPulseSound();

    int destroyed = 0;
    int pulseHits = 0;
    for (int i = static_cast<int>(enemies_.size()) - 1; i >= 0; --i)
    {
        const Rectangle bounds = enemies_[i].GetBounds();
        const Vector2 enemyCenter = {
            bounds.x + bounds.width * 0.5f,
            bounds.y + bounds.height * 0.5f
        };

        if (IsEnemyInPulseRange(bounds, playerCenter))
        {
            ++pulseHits;
            SpawnEnemyParticles(enemyCenter);

            if (enemies_[i].IsBoss())
            {
                if (enemies_[i].Damage())
                {
                    SpawnEnemyParticles(enemyCenter);
                    enemies_.erase(enemies_.begin() + i);
                    score_ += 50;
                    ++kills_;
                    ++destroyed;
                }
            }
            else
            {
                enemies_.erase(enemies_.begin() + i);
                score_ += 10;
                ++kills_;
                ++destroyed;
            }
        }
    }

    if (pulseHits > 0)
    {
        PlayEnemyHitSound();
    }

    if (kills_ >= nextBossKillTarget_ && !HasActiveBoss())
    {
        SpawnBossEncounter();
        nextBossKillTarget_ += kBossKillInterval;
    }
}

void Game::SpawnEnemy()
{
    if (!IsPlayerReadyForThreats())
    {
        return;
    }

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

void Game::SpawnBossEncounter()
{
    const int side = GetRandomValue(0, 3);
    Vector2 spawnPosition = {};

    switch (side)
    {
    case 0:
        spawnPosition = { static_cast<float>(GetRandomValue(1, columns_ - 3) * kCellSize), -kBossSize * 0.25f };
        break;
    case 1:
        spawnPosition = { static_cast<float>(screenWidth_) - kBossSize * 0.75f, static_cast<float>(GetRandomValue(1, rows_ - 3) * kCellSize) };
        break;
    case 2:
        spawnPosition = { static_cast<float>(GetRandomValue(1, columns_ - 3) * kCellSize), static_cast<float>(screenHeight_) - kBossSize * 0.75f };
        break;
    default:
        spawnPosition = { -kBossSize * 0.25f, static_cast<float>(GetRandomValue(1, rows_ - 3) * kCellSize) };
        break;
    }

    const Vector2 escortOffsets[kBossEscortCount] = {
        { -34.0f, -34.0f },
        { 34.0f, -34.0f },
        { -44.0f, 20.0f },
        { 44.0f, 20.0f },
        { 0.0f, 48.0f }
    };

    for (int i = 0; i < kBossEscortCount; ++i)
    {
        enemies_.emplace_back(
            math::Add(spawnPosition, escortOffsets[i]),
            kEnemySize,
            kEnemyBaseSpeed + survivalTime_ * (kEnemySpeedRamp * 0.55f)
        );
    }

    enemies_.emplace_back(spawnPosition, kBossSize, kBossSpeed, EnemyKind::Boss, 2, kBossAuraRadius);
    bossWarningTimer_ = kBossWarningTime;
    screenShakeTimer_ = 0.18f;
    screenShakeMagnitude_ = 4.0f;
    PlayBossWarningSound();
}

bool Game::IsPlayerReadyForThreats() const
{
    return survivalTime_ >= kOpeningGraceSeconds;
}

bool Game::IsEnemyInPulseRange(Rectangle enemyBounds, Vector2 playerCenter) const
{
    const float closestX = playerCenter.x < enemyBounds.x
        ? enemyBounds.x
        : (playerCenter.x > enemyBounds.x + enemyBounds.width ? enemyBounds.x + enemyBounds.width : playerCenter.x);

    const float closestY = playerCenter.y < enemyBounds.y
        ? enemyBounds.y
        : (playerCenter.y > enemyBounds.y + enemyBounds.height ? enemyBounds.y + enemyBounds.height : playerCenter.y);

    const Vector2 closestPoint = { closestX, closestY };
    const Vector2 delta = math::Sub(closestPoint, playerCenter);
    return math::LengthSqr(delta) <= kAttackRadius * kAttackRadius;
}

bool Game::IsPlayerInsideBossAura(const Enemy& enemy) const
{
    if (!enemy.IsBoss())
    {
        return false;
    }

    const Rectangle playerBounds = GetPlayerBounds();
    const Vector2 bossCenter = enemy.GetCenter();
    const float closestX = bossCenter.x < playerBounds.x
        ? playerBounds.x
        : (bossCenter.x > playerBounds.x + playerBounds.width ? playerBounds.x + playerBounds.width : bossCenter.x);
    const float closestY = bossCenter.y < playerBounds.y
        ? playerBounds.y
        : (bossCenter.y > playerBounds.y + playerBounds.height ? playerBounds.y + playerBounds.height : bossCenter.y);

    const Vector2 delta = math::Sub({ closestX, closestY }, bossCenter);
    return math::LengthSqr(delta) <= enemy.GetAuraRadius() * enemy.GetAuraRadius();
}

bool Game::HasActiveBoss() const
{
    for (const Enemy& enemy : enemies_)
    {
        if (enemy.IsBoss())
        {
            return true;
        }
    }

    return false;
}

void Game::EnsureAudio()
{
    if (!audioReady_)
    {
        InitializeAudio();
    }
}

void Game::SetPaused(bool paused)
{
    paused_ = paused;
    SyncPauseUi();
}

void Game::SyncPauseUi() const
{
#ifdef PLATFORM_WEB
    emscripten_run_script(paused_ ? "setPausedUi(true)" : "setPausedUi(false)");
#endif
}

void Game::InitializeAudio()
{
    InitAudioDevice();
    audioReady_ = IsAudioDeviceReady();

    if (!audioReady_)
    {
        return;
    }

    SetMasterVolume(0.55f);
    uiSound_ = CreateToneSound(520.0f, 0.055f, 0.18f, 24.0f);
    moveSound_ = CreateToneSound(340.0f, 0.035f, 0.11f, 28.0f);
    pulseSound_ = CreateToneSound(190.0f, 0.13f, 0.22f, 12.0f);
    hitSound_ = CreateToneSound(760.0f, 0.075f, 0.18f, 22.0f);
    gameOverSound_ = CreateToneSound(120.0f, 0.32f, 0.20f, 6.5f);
    bossWarningSound_ = CreateToneSound(220.0f, 0.50f, 0.34f, 4.2f);
}

void Game::ShutdownAudio()
{
    if (!audioReady_)
    {
        return;
    }

    UnloadSound(uiSound_);
    UnloadSound(moveSound_);
    UnloadSound(pulseSound_);
    UnloadSound(hitSound_);
    UnloadSound(gameOverSound_);
    UnloadSound(bossWarningSound_);
    CloseAudioDevice();
    audioReady_ = false;
}

void Game::PlayUiSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(uiSound_);
    }
}

void Game::PlayMoveSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(moveSound_);
    }
}

void Game::PlayPulseSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(pulseSound_);
    }
}

void Game::PlayEnemyHitSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(hitSound_);
    }
}

void Game::PlayGameOverSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(gameOverSound_);
    }
}

void Game::PlayBossWarningSound()
{
    EnsureAudio();
    if (audioReady_)
    {
        PlaySound(bossWarningSound_);
    }
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

void Game::SpawnEnemyParticles(Vector2 center)
{
    for (int i = 0; i < 8; ++i)
    {
        const float angle = static_cast<float>(GetRandomValue(0, 359)) * DEG2RAD;
        const float speed = static_cast<float>(GetRandomValue(70, 180));

        Particle particle = {};
        particle.position = center;
        particle.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };
        particle.size = static_cast<float>(GetRandomValue(2, 5));
        particle.life = 0.36f;
        particle.maxLife = 0.36f;
        particle.color = kDanger;

        particles_.push_back(particle);
    }
}

Rectangle Game::GetMenuButtonBounds() const
{
    return {
        screenWidth_ * 0.5f - kButtonWidth * 0.5f,
        screenHeight_ * 0.5f + 34.0f,
        kButtonWidth,
        kButtonHeight
    };
}

Rectangle Game::GetGameOverButtonBounds() const
{
    return {
        screenWidth_ * 0.5f - kButtonWidth * 0.5f,
        screenHeight_ * 0.5f + 74.0f,
        kButtonWidth,
        kButtonHeight
    };
}

Rectangle Game::GetGuideButtonBounds() const
{
    return { screenWidth_ - 130.0f, 54.0f, kSmallButtonWidth, kSmallButtonHeight };
}

Rectangle Game::GetPauseMenuButtonBounds() const
{
    return {
        screenWidth_ * 0.5f - kButtonWidth * 0.5f,
        screenHeight_ * 0.5f + 32.0f,
        kButtonWidth,
        kButtonHeight
    };
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

bool Game::IsPointerOverUi(Vector2 pointer) const
{
    bool overUi = CheckCollisionPointRec(pointer, GetGuideButtonBounds());

    if (state_ == MENU)
    {
        overUi = overUi || CheckCollisionPointRec(pointer, GetMenuButtonBounds());
    }
    else if (state_ == GAMEOVER)
    {
        overUi = overUi || CheckCollisionPointRec(pointer, GetGameOverButtonBounds());
    }
    else if (state_ == PLAYING && paused_)
    {
        overUi = overUi || CheckCollisionPointRec(pointer, GetPauseMenuButtonBounds());
    }

    return overUi;
}

Vector2 Game::GridToWorld(int gridX, int gridY) const
{
    return {
        static_cast<float>(gridX * kCellSize),
        static_cast<float>(gridY * kCellSize)
    };
}
