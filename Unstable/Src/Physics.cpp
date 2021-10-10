#include "Physics.h"
#include "Engine.h"


void Unstable::Physics::Init(Engine* engine, Physics** physics)
{
	Physics* p = *physics = (Physics*)calloc(1, sizeof(Physics));

	b2Vec2 g(0, 0);
	b2World* w = p->world = new b2World(g);
	p->draw = new DebugDraw();
	p->draw->SetFlags(b2Draw::e_shapeBit);
	w->SetDebugDraw(p->draw);

	p->callback = new RayIntersectCallback();
	p->LastQuery = (RaycastData*)calloc(1024, sizeof(RaycastData));
	p->QueryCount = 0;

	_engine = engine;

}

void Unstable::Physics::Update(Engine* engine)
{
	engine->Physics->world->Step(engine->fixedDeltatime, 5, 5);
}

bool Unstable::Physics::CircleRayIntersect(sf::Vector2f point, float radius)
{
	b2World* world = _engine->Physics->world;

	for (s32 i = 0; i < _engine->Physics->QueryCount; i++)
	{
		_engine->Physics->LastQuery[i].Body = nullptr;
	}
	_engine->Physics->QueryCount = 0;

	_engine->Physics->CircleCast = true;

	_engine->Physics->CastRadius = radius;
	_engine->Physics->CastCenter = point;

	world->QueryAABB(_engine->Physics->callback, b2AABB{ {point.x - radius, point.y - radius}, {point.x + radius, point.y + radius} });

	_engine->Physics->CircleCast = false;

	if (_engine->Physics->QueryCount > 0)
	{
		return true;
	}

	return false;
}

bool Unstable::Physics::RayIntersect(sf::Vector2f point, sf::Vector2f size)
{
	b2World* world = _engine->Physics->world;

	for (s32 i = 0; i < _engine->Physics->QueryCount; i++)
	{
		_engine->Physics->LastQuery[i].Body = nullptr;
	}
	_engine->Physics->QueryCount = 0;

	world->QueryAABB(_engine->Physics->callback, b2AABB{ {point.x - size.x / 2, point.y - size.y / 2}, {point.x + size.x / 2, point.y + size.y / 2} });

	if (_engine->Physics->QueryCount > 0)
	{
		return true;
	}

	return false;
}

bool Unstable::Physics::RayIntersectCallback::ReportFixture(b2Fixture* fixture)
{
	if (_engine->Physics->QueryCount >= 1024)
	{
		return false;
	}

	if (fixture != nullptr)
	{
		RaycastData* dr = _engine->Physics->LastQuery;

		b2Body* b = fixture->GetBody();

		if (_engine->Physics->CircleCast) 
		{
			b2Vec2 v0 = b->GetPosition() - b2Vec2(_engine->Physics->CastCenter.x, _engine->Physics->CastCenter.y);

			float f = v0.LengthSquared();

			if (f < _engine->Physics->CastRadius)
			{
				dr[_engine->Physics->QueryCount++].Body = b;
			}
		}
		else
		{
			dr[_engine->Physics->QueryCount++].Body = b;
		}

		return true;
	}

	return false;
}








void Unstable::Physics::DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	sf::ConvexShape* cs = new sf::ConvexShape(vertexCount);
	for (s32 i = 0; i < vertexCount; i++)
	{
		cs->setPoint(i, sf::Vector2f(vertices[i].x, vertices[i].y));
	}
	cs->setFillColor(sf::Color(color.r * 255, color.g * 255, color.b * 255, color.a * 255));
	Rendering::AddDrawable(_engine->Rendering, cs, 9999);
}

void Unstable::Physics::DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	sf::ConvexShape* cs = new sf::ConvexShape(vertexCount);
	for (s32 i = 0; i < vertexCount; i++)
	{
		cs->setPoint(i, sf::Vector2f(vertices[i].x, vertices[i].y));
	}
	cs->setFillColor(sf::Color(color.r * 255, color.g * 255, color.b * 255, color.a * 255));
	Rendering::AddDrawable(_engine->Rendering, cs, 9999);
}

void Unstable::Physics::DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	sf::CircleShape* cs = new sf::CircleShape(radius);
	cs->setOrigin(4, 4);
	cs->setPosition(center.x, center.y);
	cs->setFillColor(sf::Color(color.r, color.g, color.b, color.a));

	Rendering::AddDrawable(_engine->Rendering, cs, 9999);
}

void Unstable::Physics::DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	sf::CircleShape* cs = new sf::CircleShape(radius);
	cs->setOrigin(4, 4);
	cs->setPosition(center.x, center.y);
	cs->setFillColor(sf::Color(color.r * 255, color.g * 255, color.b * 255, color.a * 255));

	Rendering::AddDrawable(_engine->Rendering, cs, 9999);
}

void Unstable::Physics::DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
}

void Unstable::Physics::DebugDraw::DrawTransform(const b2Transform& xf)
{
}

void Unstable::Physics::DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
}