#pragma once

#include "Engine.Forward.h"
#include <SFML/Graphics.hpp>

namespace Unstable
{
	namespace UI
	{
		struct Rect {
			float x;
			float y;
			float width;
			float height;

			Rect(float x, float y, float w, float h);
		};

		struct Color {
			float r;
			float g;
			float b;
			float a;

			Color(float r, float g, float b, float a);
		};

		const UI::Color Black = { 0,0,0,1 };
		const UI::Color White = { 1,1,1,1 };
		const UI::Color Red = { 1,0,0,1 };
		const UI::Color Green = { 0,1,0,1 };
		const UI::Color Blue = { 0,0,1,1 };
		const UI::Color Clear = { 0,0,0,0 };

		enum ConstraintAxis {
			Vertical,
			Horizontal
		};

		enum ConstraintType {
			Relative,
			Static
		};

		enum ConstraintProperty {
			Size,
			Anchor
		};

		struct Constraint {
			ConstraintType Type;
			ConstraintAxis Axis;
			ConstraintProperty Property;
			float Value;
		};

		enum ElementType {
			BoxType,
			CircleType,
			ButtonType,
			TextType
		};

		struct Element {
			s32 ConstraintIndex;
			Color BackgroundColor;
			Rect Rect;
			ElementType Type;
			std::string* Text;
			sf::Font* Font;
			sf::Texture* Texture;
			Element* Parent;
			Constraint Constraints[32];
		};

		struct UIState {
			s32 ElementIndex;
			sf::Font* ActiveFont;
			Element* Elements;
			std::vector<sf::Texture*> Textures;
			std::vector<sf::Font*> Fonts;
		};

		const s32 MaxElements = 2048;
		static Engine* _engine;

		void Layout(Unstable::UI::UIState* state);
		void Init(Engine* engine);
		void Update(Engine* engine);
		void Render(Engine* engine);

		UI::Element* GetNextElement();
		void SetActiveFont(sf::Font* font);

		// loading
		void LoadUITextures();
		void LoadUIFonts();

		// util
		UI::Rect AddRect(Rect r1, Rect r2);
		bool PointRectIntersect(sf::Vector2i point, Rect rect);

		// Layout
		void NWayLayout(UI::Rect rect, UI::Color color, Element* parent, s32 tIndex);
		void SetConstraint(Element* element, ConstraintType type, ConstraintAxis axis, ConstraintProperty prop, float value);
		void SetParent(Element* child, Element* parent);
		void SetColor(Element* element, Color color);
		void SetTexture(Element* element, sf::Texture* texture);
		void SetFont(Element* element, sf::Font* font);
		void DoLayout(Element* element);
		UI::Rect ResolveParentRect(Element* element);
		UI::Rect ResolveHorizontalAxisConstraint(Element* element, s32 constraintIndex);
		UI::Rect ResolveVerticalAxisConstraint(Element* element, s32 constraintIndex);

		bool Button(UI::Rect rect, UI::Color color, UI::Element* parent);
		UI::Element* Box(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture);
		UI::Element* Circle(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture);
		UI::Element* Window(UI::Rect rect, UI::Color color, UI::Element* parent);
		UI::Element* Texture(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture);
		void Text(std::string text, UI::Rect rect, UI::Color color, UI::Element* parent, sf::Font* font);
}
}