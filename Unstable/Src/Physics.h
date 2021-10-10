#pragma once

#include "Engine.Forward.h"
#include <box2d/box2d.h>
#include <stdlib.h>
#include <SFML/Graphics.hpp>

namespace Unstable
{
    namespace Physics
    {
        class DebugDraw : public b2Draw {
            // Inherited via b2Draw
            virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
            virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
            virtual void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
            virtual void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
            virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
            virtual void DrawTransform(const b2Transform& xf) override;
            virtual void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
        };

        class RayIntersectCallback : public b2QueryCallback {
            // Inherited via b2QueryCallback
            virtual bool ReportFixture(b2Fixture* fixture) override;
        };

        struct RaycastData {
            b2Body* Body;
        };

        struct Physics {
            b2World* world;
            DebugDraw* draw;
            RayIntersectCallback* callback;
            RaycastData* LastQuery;
            s32 QueryCount;
            bool CircleCast;
            float CastRadius;
            sf::Vector2f CastCenter;
        };

        static Engine* _engine;

        void Init(Engine* engine, Physics** physics);
        void Update(Engine* engine);
        bool RayIntersect(sf::Vector2f point, sf::Vector2f size);
        bool CircleRayIntersect(sf::Vector2f point, float radius);
    }
}