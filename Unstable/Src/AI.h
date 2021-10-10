#pragma once

#include "Engine.Forward.h"
#include <SFML/Graphics.hpp>
#include <vector>

namespace Unstable
{
    namespace AI
    {
        struct NavMesh {
            s32* Data;
            s32 GridWidth;
            s32 GridHeight;
            f32 GridSize;
            sf::Vector2f WorldBounds[2];
        };

        static Engine* _engine;
        static NavMesh* _navmesh;

        void Init(Engine* engine);
        void GenerateNavMesh();
        std::vector<sf::Vector2f> GeneratePath(Engine* engine, sf::Vector2f start, sf::Vector2f end);
        sf::Vector2i WorldToGrid(sf::Vector2f point);
        std::vector<sf::Vector2i> GetNeighbors(sf::Vector2i point);
    }
}
