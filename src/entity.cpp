#include "entity.h"

#include <cmath>

namespace
{
    constexpr Color kEnemyColor = { 232, 82, 92, 255 };
    constexpr Color kEnemyCoreColor = { 255, 178, 156, 255 };
    constexpr Color kBossColor = { 74, 28, 116, 255 };
    constexpr Color kBossCoreColor = { 188, 120, 255, 255 };
    constexpr Color kBossAuraColor = { 160, 70, 255, 170 };
}

Enemy::Enemy()
    : Enemy({ 0.0f, 0.0f }, 28.0f, 120.0f)
{
}

Enemy::Enemy(Vector2 startPosition, float size, float speed)
    : Enemy(startPosition, size, speed, EnemyKind::Normal, 1, 0.0f)
{
}

Enemy::Enemy(Vector2 startPosition, float size, float speed, EnemyKind kind, int health, float auraRadius)
    : position_(startPosition),
      size_(size),
      speed_(speed),
      auraRadius_(auraRadius),
      health_(health),
      kind_(kind),
      color_(kind == EnemyKind::Boss ? kBossColor : kEnemyColor)
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

    if (IsBoss())
    {
        const Vector2 center = GetCenter();
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), auraRadius_, kBossAuraColor);
        DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), auraRadius_ - 4.0f, kBossAuraColor);
    }

    DrawRectangleRec(body, color_);
    DrawRectangleLinesEx(body, IsBoss() ? 3.0f : 2.0f, IsBoss() ? kBossCoreColor : kEnemyCoreColor);

    if (IsBoss())
    {
        DrawText(TextFormat("%d", health_), static_cast<int>(body.x + body.width * 0.5f - 6.0f), static_cast<int>(body.y + body.height * 0.5f - 10.0f), 20, kBossCoreColor);
    }
}

bool Enemy::Damage()
{
    --health_;
    return health_ <= 0;
}

Rectangle Enemy::GetBounds() const
{
    return { position_.x, position_.y, size_, size_ };
}

Vector2 Enemy::GetCenter() const
{
    return {
        position_.x + size_ * 0.5f,
        position_.y + size_ * 0.5f
    };
}

Vector2 Enemy::GetPosition() const
{
    return position_;
}

float Enemy::GetAuraRadius() const
{
    return auraRadius_;
}

int Enemy::GetHealth() const
{
    return health_;
}

bool Enemy::IsBoss() const
{
    return kind_ == EnemyKind::Boss;
}
