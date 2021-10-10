#pragma once

#include "Engine.Forward.h"
#include <SFML/Graphics.hpp>

namespace Unstable
{
    namespace Utility
    {
        const double PI = 3.14159265358979323846;
        u32 Add(u32 a, u32 b);
        f32 Min(f32 v, f32 min);
        f32 Max(f32 v, f32 max);
        f32 Clamp(f32 v, f32 min, f32 max);
        f32 Clamp01(f32 v);
        f32 Lerp(f32 a, f32 b, f32 t);
        f32 LerpClamped(f32 a, f32 b, f32 t);
        bool AlmostZero(f32 a, f32 threshold);
        f32 SnapToZero(f32 a, f32 threshold);
        sf::Vector2f GetMouseWorldPos(Unstable::Engine* engine);
        sf::Vector2f GetMouseTilePos(Unstable::Engine* engine);
        f32 Random01();

        f32 Magnitude(sf::Vector2f v0);
        sf::Vector2f Normalize(sf::Vector2f v0);
        sf::Vector2f GetLookDirection(Unstable::Engine* engine, sf::Vector2f point);
        sf::Vector2f GetLookToDirection(sf::Vector2f from, sf::Vector2f to);
        f32 Distance(sf::Vector2f from, sf::Vector2f to);
        f32 AngleRad(sf::Vector2f from, sf::Vector2f to);
        f32 AngleDeg(sf::Vector2f from, sf::Vector2f to);
    }
}