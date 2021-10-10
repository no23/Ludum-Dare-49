#pragma once

#include "Engine.Forward.h"
#include <vector>
#include <SFML/Graphics.hpp>

namespace Unstable
{
    namespace Rendering
    {
        struct DepthRenderable {
            s32 Depth;
            sf::Drawable* Drawable;
        };
        struct Rendering
        {
            std::vector<DepthRenderable> Drawing;
        };

        void Init(Rendering** rendering);
        void Clear(Engine* engine);

        void ClearDrawables(Rendering* rendering);
        void AddDrawable(Rendering* rendering, sf::Drawable* drawable, s32 depth);
        void Render(Engine* engine); //Walk all ecs entities with rendering components
        void Present(Unstable::Engine* engine);
    }
}
