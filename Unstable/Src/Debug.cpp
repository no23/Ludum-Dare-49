#include "Debug.h"
#include "Engine.h"
#ifdef DrawText
#undef DrawText
#endif

void Unstable::Debug::Init(Engine* engine)
{
	_engine = engine;
	_boxPool = std::vector<sf::RectangleShape*>();
	_usedBoxes = 0;
}

void Unstable::Debug::PostRender()
{
	_usedBoxes = 0;
}

void Unstable::Debug::DrawText(const sf::String& text, sf::Vector2f point)
{
	sf::Text* t = new sf::Text(text, *_engine->GUIState->Fonts[0]);
	t->setPosition(point);

	Rendering::AddDrawable(_engine->Rendering, t, 9999);
}

void Unstable::Debug::DrawBox(sf::Vector2f point, sf::Vector2f size, bool filled)
{
	sf::RectangleShape* rect;

	if (_usedBoxes < _boxPool.size())
	{
		rect = _boxPool[_usedBoxes++];
	}
	else
	{
		rect = new sf::RectangleShape();
		_boxPool.push_back(rect);
	}

	rect->setSize(size);
	rect->setOrigin(size.x / 2, size.y / 2);
	rect->setPosition(point);
	if (filled)
	{
		rect->setFillColor(sf::Color::Green);
	}
	else
	{
		rect->setFillColor(sf::Color(0,0,0,0));
		rect->setOutlineThickness(0.25f);
		rect->setOutlineColor(sf::Color::Green);
	}

	Rendering::AddDrawable(_engine->Rendering, rect, 9999);
}

void Unstable::Debug::DrawLine(sf::Vector2f p1, sf::Vector2f p2)
{
	const sf::Vertex vx[]{
		sf::Vertex(p1),
		sf::Vertex(p2),
	};
	_engine->Window->draw(vx, 2, sf::PrimitiveType::Lines);
}

void Unstable::Debug::DrawCircle(sf::Vector2f point, float radius, bool filled)
{
	sf::CircleShape* rect = new sf::CircleShape(radius);
	rect->setOrigin(radius, radius);
	rect->setPosition(point);

	if (filled)
	{
		rect->setFillColor(sf::Color::Green);
	}
	else
	{
		rect->setFillColor(sf::Color(0, 0, 0, 0));
		rect->setOutlineThickness(1);
		rect->setOutlineColor(sf::Color::Green);
	}

	Rendering::AddDrawable(_engine->Rendering, rect, 9999);
}
