#pragma once
#include <SFML/Graphics.hpp>
#include "Engine.Forward.h"
#include <vector>

namespace Unstable
{
	namespace Debug
	{
		static Engine* _engine;
		static std::vector<sf::RectangleShape*> _boxPool;
		static s32 _usedBoxes;

		void PostRender();
		void Init(Engine* engine);
		void DrawText(const sf::String& text, sf::Vector2f point);
		void DrawBox(sf::Vector2f point, sf::Vector2f size, bool filled);
		void DrawLine(sf::Vector2f p1, sf::Vector2f p2);
		void DrawCircle(sf::Vector2f point, float radius, bool filled);
	}
}