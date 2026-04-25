#pragma once

#include "raylib.h"

class Enemy
{
public:
    Enemy();
    Enemy(Vector2 startPosition, float size, float speed);

    void Update(Vector2 targetCenter, float deltaTime);
    void Draw() const;

    Rectangle GetBounds() const;
    Vector2 GetPosition() const;

private:
    Vector2 position_;
    float size_;
    float speed_;
    Color color_;
};
