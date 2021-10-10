#pragma once

#include "Utility.h"
#include "Engine.h"

#include <SFML/Graphics.hpp>

namespace sf
{
	
}

namespace Unstable
{
	namespace Utility
	{
		u32 Add(u32 a, u32 b)
		{
			return a + b;
		}

		f32 Min(f32 v, f32 min)
		{
			return v < min ? v : min;
		}

		f32 Max(f32 v, f32 max)
		{
			return v > max ? v : max;
		}

		f32 Clamp(f32 v, f32 min, f32 max)
		{
			return v <= min ? min : v >= max ? max : v;
		}

		f32 Clamp01(f32 v)
		{
			return v <= 0 ? 0 : v >= 1 ? 1 : v;
		}

		f32 Lerp(f32 a, f32 b, f32 t)
		{
			return a * (1 - t) + b * t;
		}

		f32 LerpClamped(f32 a, f32 b, f32 t)
		{
			f32 tt = Clamp01(t);
			return a * (1 - tt) + b * tt;
		}

		bool AlmostZero(f32 a, f32 threshold)
		{
			return
				a <= threshold &&
				a >= -threshold;
		}

		f32 SnapToZero(f32 a, f32 threshold)
		{
			return AlmostZero(a, threshold) ? 0 : a;
		}

		sf::Vector2f GetMouseWorldPos(Unstable::Engine* engine)
		{
			return engine->Window->mapPixelToCoords({ (s32)engine->MouseX, (s32)engine->MouseY });
		}

		sf::Vector2f GetMouseTilePos(Unstable::Engine* engine)
		{
			auto pos = engine->Window->mapPixelToCoords({ (s32)engine->MouseX, (s32)engine->MouseY });
			f32 tileX = floorf((pos.x + 4) / 8.0f) * 8;
			f32 tileY = floorf((pos.y + 4) / 8.0f) * 8;
			return { tileX, tileY };
		}

		f32 Random01()
		{
			return (f32)std::rand() / RAND_MAX;
		}

		f32 Magnitude(sf::Vector2f v0)
		{
			return sqrt(v0.x * v0.x + v0.y * v0.y);
		}

		sf::Vector2f Normalize(sf::Vector2f v0)
		{
			f32 m = Magnitude(v0);
			if (m != 0)
			{
				return v0 / m;
			}
			else
			{
				return sf::Vector2f();
			}
		}

		sf::Vector2f GetLookDirection(Unstable::Engine* engine, sf::Vector2f point)
		{
			return Normalize(GetMouseWorldPos(engine) - point);
		}

		sf::Vector2f GetLookToDirection(sf::Vector2f from, sf::Vector2f to)
		{
			return Normalize(from - to);
		}

		f32 Distance(sf::Vector2f from, sf::Vector2f to)
		{
			return abs(Magnitude(from - to));
		}

		f32 AngleRad(sf::Vector2f from, sf::Vector2f to)
		{
			return (f32)atan2(to.y - from.y, to.x - from.x);
		}
		f32 AngleDeg(sf::Vector2f from, sf::Vector2f to)
		{
			return AngleRad(from, to) * 180.0f / PI;
		}
	}
}
