#pragma once

#include "raylib.h"

enum class EnemyKind
{
    Normal,
    Boss
};

class Enemy
{
public:
    Enemy();
    Enemy(Vector2 startPosition, float size, float speed);
    Enemy(Vector2 startPosition, float size, float speed, EnemyKind kind, int health, float auraRadius);

    void Update(Vector2 targetCenter, float deltaTime);
    void Draw() const;
    bool Damage();

    Rectangle GetBounds() const;
    Vector2 GetCenter() const;
    Vector2 GetPosition() const;
    float GetAuraRadius() const;
    int GetHealth() const;
    bool IsBoss() const;

private:
    Vector2 position_;
    float size_;
    float speed_;
    float auraRadius_;
    int health_;
    EnemyKind kind_;
    Color color_;
};
