#include "Rendering.h"

#include "Engine.h"

namespace Unstable
{
	namespace Rendering
	{
		void Init(Rendering** rendering)
		{
			Rendering* render = (Rendering*)calloc(1, sizeof(Rendering));
			*rendering = render;
			(*rendering)->Drawing = std::vector<DepthRenderable>();
		}

		void ClearDrawables(Rendering* rendering)
		{
			rendering->Drawing.clear();
		}

		void AddDrawable(Rendering* rendering, sf::Drawable* drawable, s32 depth)
		{
			rendering->Drawing.push_back({ depth, drawable });
		}

		void Clear(Unstable::Engine* engine)
		{
			engine->Window->clear();
			ClearDrawables(engine->Rendering);
		}

		void Render(Unstable::Engine* engine)
		{
			std::sort(engine->Rendering->Drawing.begin(), engine->Rendering->Drawing.end(), 
				[](DepthRenderable const& v1, DepthRenderable const & v2) {
					return v1.Depth < v2.Depth;
				});

			for (size_t i = 0; i < engine->Rendering->Drawing.size(); i++)
			{
				sf::Drawable* shape = engine->Rendering->Drawing[i].Drawable;
				engine->Window->draw(*shape);
			}
		}

		void Present(Unstable::Engine* engine)
		{
			engine->Window->display();
		}
	}
}