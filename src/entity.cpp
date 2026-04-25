#include "entity.h"

#include <cmath>

namespace
{
    constexpr Color kEnemyColor = { 232, 82, 92, 255 };
    constexpr Color kEnemyCoreColor = { 255, 178, 156, 255 };
}

Enemy::Enemy()
    : Enemy({ 0.0f, 0.0f }, 28.0f, 120.0f)
{
}

Enemy::Enemy(Vector2 startPosition, float size, float speed)
    : position_(startPosition),
      size_(size),
      speed_(speed),
      color_(kEnemyColor)
{
}

void Enemy::Update(Vector2 targetCenter, float deltaTime)
{
    const Vector2 center = {
        position_.x + size_ * 0.5f,
        position_.y + size_ * 0.5f
    };

    const Vector2 toTarget = {
        targetCenter.x - center.x,
        targetCenter.y - center.y
    };

    const float distanceSqr = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
    if (distanceSqr <= 0.0001f)
    {
        return;
    }

    const float invDistance = 1.0f / std::sqrt(distanceSqr);
    position_.x += toTarget.x * invDistance * speed_ * deltaTime;
    position_.y += toTarget.y * invDistance * speed_ * deltaTime;
}

void Enemy::Draw() const
{
    const Rectangle body = GetBounds();
    DrawRectangleRec(body, color_);
    DrawRectangleLinesEx(body, 2.0f, kEnemyCoreColor);
}

Rectangle Enemy::GetBounds() const
{
    return { position_.x, position_.y, size_, size_ };
}

Vector2 Enemy::GetPosition() const
{
    return position_;
}
