#pragma once

#include "raylib.h"

namespace math
{
    inline constexpr float Clamp01(float value)
    {
        return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    }

    inline Vector2 Add(Vector2 a, Vector2 b)
    {
        return { a.x + b.x, a.y + b.y };
    }

    inline Vector2 Sub(Vector2 a, Vector2 b)
    {
        return { a.x - b.x, a.y - b.y };
    }

    inline Vector2 Scale(Vector2 value, float scalar)
    {
        return { value.x * scalar, value.y * scalar };
    }

    inline float LengthSqr(Vector2 value)
    {
        return value.x * value.x + value.y * value.y;
    }

    inline Vector2 Lerp(Vector2 from, Vector2 to, float t)
    {
        t = Clamp01(t);
        return {
            from.x + (to.x - from.x) * t,
            from.y + (to.y - from.y) * t
        };
    }
}
