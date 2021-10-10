#include "UI.h"
#include "Engine.h"
#include <filesystem>
#include "Scheme.h"

using namespace Unstable;

sf::Sprite* bg;

void Unstable::UI::Init(Engine* engine)
{
	engine->GUIState = (UIState*)calloc(1, sizeof(UIState));
	engine->GUIState->Elements = (Element*)calloc(UI::MaxElements, sizeof(Element));

	engine->GUIState->Textures = std::vector<sf::Texture*>();
	engine->GUIState->Fonts = std::vector<sf::Font*>();

	_engine = engine;

	LoadUITextures();
	LoadUIFonts();


	sf::Texture* tex = new sf::Texture();
	tex->loadFromFile("Assets/titlecard.png");

	bg = new sf::Sprite(*tex);
	bg->setScale(0.25f, 0.25f);
}

void Unstable::UI::Layout(Unstable::UI::UIState* state)
{

}

void Unstable::UI::Update(Engine* engine)
{
	s32 ww = engine->Window->getSize().x * engine->UIScale;
	s32 wh = engine->Window->getSize().y * engine->UIScale;
	//engine->CurrentUI = 2;
	switch (engine->CurrentUI)
	{
	case 0:
	{
		//start
		break;
	}
	case 1:
	{
		//loading
		//do shit
		//fmod
		engine->CurrentUI = 2;
		break;
	}
	case 2:
	{
		engine->Window->draw(*bg);

		Text("Unstable", Rect(25, wh - 400, 200, (50)), UI::White, nullptr, engine->GUIState->Fonts[0]);

		if (Button(Rect(25, wh - 300, (ww * 0.25f), (50)), UI::White, nullptr))
		{
			Scheme::Eval(engine, "(load-level-1)");
			engine->CurrentUI = 0;
		}
		Text("Play", Rect(40, wh - 305, 200, (30)), UI::White, nullptr, engine->GUIState->Fonts[0]);

		if (Button(Rect(25, wh - 225, (ww * 0.25f), (50)), UI::White, nullptr))
		{
			Scheme::Eval(engine, "(load-level-edit)");
			engine->CurrentUI = 0;
		}
		Text("Editor", Rect(40, wh - 235, 200, (30)), UI::White, nullptr, engine->GUIState->Fonts[0]);

		if (Button(Rect(25, wh - 150, (ww * 0.25f), (50)), UI::White, nullptr))
		{
			engine->CurrentUI = 3;
		}
		Text("Credits", Rect(40, wh - 160, 200, (30)), UI::White, nullptr, engine->GUIState->Fonts[0]);

		if (Button(Rect(25, wh - 75, (ww * 0.25f), (50)), UI::White, nullptr))
		{

		}
		Text("Quit", Rect(40, wh - 85, 200, (30)), UI::White, nullptr, engine->GUIState->Fonts[0]);
		break;
	}
	case 3:
	{
		if (Button(Rect(25, wh - 75, (ww * 0.25f), (50)), UI::White, nullptr))
		{
			engine->CurrentUI = 2;
		}
		Text("Quit", Rect(40, wh - 85, 200, (30)), UI::White, nullptr, engine->GUIState->Fonts[0]);

		break;
	}
	default:
		break;
	}

}

void Unstable::UI::Render(Engine* engine)
{
	std::vector<sf::Drawable*> drawables;
	for (s32 i = 0; i < engine->GUIState->ElementIndex; i++)
	{
		Element* e = &engine->GUIState->Elements[i];
		switch (e->Type)
		{
		case UI::ElementType::BoxType:
		case UI::ElementType::ButtonType:
		{
			sf::RectangleShape* r = new sf::RectangleShape();
			r->setFillColor(
				sf::Color(
					e->BackgroundColor.r * 255,
					e->BackgroundColor.g * 255,
					e->BackgroundColor.b * 255,
					e->BackgroundColor.a * 255));

			r->setPosition(e->Rect.x, e->Rect.y);
			r->setSize({ e->Rect.width, e->Rect.height });

			if (e->Texture != nullptr)
			{
				r->setTexture(e->Texture);
			}

			drawables.push_back(r);
			break;
		}
		case UI::ElementType::TextType:
		{
			sf::Text* t = new sf::Text();
			t->setFont(*e->Font);
			t->setString(*e->Text);
			t->setColor(
				sf::Color(
					e->BackgroundColor.r * 255,
					e->BackgroundColor.g * 255,
					e->BackgroundColor.b * 255,
					e->BackgroundColor.a * 255));

			t->setPosition(e->Rect.x, e->Rect.y);
			// increases font res
			t->setCharacterSize(30 / engine->UIScale);
			t->setScale(engine->UIScale, engine->UIScale);

			sf::FloatRect frec = t->getGlobalBounds();

			f32 ws = e->Rect.width / frec.width;
			f32 hs = e->Rect.height / frec.height;
			f32 scaleToFit = ws > hs ? hs : ws;

			t->scale(scaleToFit, scaleToFit);

			delete e->Text;

			drawables.push_back(t);
			break;
		}
		default:
			break;
		}
	}

	for (s32 i = 0; i < drawables.size(); i++)
	{
		engine->Window->draw(*drawables[i]);

		delete drawables[i];
	}

	for (s32 i = 0; i < engine->GUIState->ElementIndex; i++)
	{
		Element* elm = &engine->GUIState->Elements[i];

		elm->Texture = nullptr;
		elm->ConstraintIndex = 0;
		elm->BackgroundColor = { 0,0,0,0 };
		elm->Parent = nullptr;
		elm->Rect = { 0,0,0,0 };
	}
	engine->GUIState->ElementIndex = 0;
}

UI::Element* Unstable::UI::GetNextElement()
{
	if (_engine->GUIState->ElementIndex > UI::MaxElements)
	{
		return nullptr;
	}

	return &_engine->GUIState->Elements[_engine->GUIState->ElementIndex++];
}

void Unstable::UI::SetActiveFont(sf::Font* font)
{
	_engine->GUIState->ActiveFont = font;
}

void Unstable::UI::LoadUITextures()
{
	const char* textureFilePath = "./assets/ui.png";

	const s32 sw = 32;

	sf::Image raw;
	raw.loadFromFile(textureFilePath);

	for (s32 y = 0; y < raw.getSize().y; y += sw)
	{
		for (s32 x = 0; x < raw.getSize().x; x += sw)
		{
			sf::Texture* tex = new sf::Texture();
			if (tex->loadFromImage(raw, { x,y,sw,sw }))
			{
				_engine->GUIState->Textures.push_back(tex);
			}
		}
	}
}

void Unstable::UI::LoadUIFonts()
{
	namespace fs = std::filesystem;

	std::string path = "./assets/fonts";
	for (const auto& entry : fs::directory_iterator(path))
	{
		sf::Font* f = new sf::Font();
		f->loadFromFile(entry.path().string());
		_engine->GUIState->Fonts.push_back(f);
	}
}

void Unstable::UI::SetConstraint(Element* element, ConstraintType type, ConstraintAxis axis, ConstraintProperty prop, float value)
{
	assert(element->ConstraintIndex < 32);
	element->Constraints[element->ConstraintIndex++] = { type, axis, prop, value };
}

void Unstable::UI::SetParent(Element* child, Element* parent)
{
	child->Parent = parent;
}

void Unstable::UI::SetColor(Element* element, Color color)
{
	element->BackgroundColor = color;
}

void Unstable::UI::SetTexture(Element* element, sf::Texture* texture)
{
	element->Texture = texture;
}

UI::Element* Unstable::UI::Texture(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture)
{
	return Box(rect, color, parent, texture);

}

void Unstable::UI::SetFont(UI::Element* element, sf::Font* font)
{
	element->Font = font;
}

void Unstable::UI::Text(std::string text, UI::Rect rect, UI::Color color, UI::Element* parent, sf::Font* font)
{
	Element* elm = UI::GetNextElement();
	if (elm == nullptr)
	{
		// we are over the 2048 element limit
		assert(elm && "Failed to get new UI element. Have more than 2048 been drawn?");
	}
	elm->Type = UI::ElementType::TextType;
	SetParent(elm, parent);
	SetColor(elm, color);
	SetFont(elm, font);

	elm->Text = new std::string(text);

	UI::ConstraintType ct = parent != nullptr ? UI::ConstraintType::Relative : UI::ConstraintType::Static;

	// x pos
	SetConstraint(elm, ct, UI::ConstraintAxis::Horizontal, UI::ConstraintProperty::Anchor, rect.x);
	// y pos
	SetConstraint(elm, ct, UI::ConstraintAxis::Vertical, UI::ConstraintProperty::Anchor, rect.y);
	// w
	SetConstraint(elm, UI::ConstraintType::Static, UI::ConstraintAxis::Horizontal, UI::ConstraintProperty::Size, rect.width);
	// h
	SetConstraint(elm, UI::ConstraintType::Static, UI::ConstraintAxis::Vertical, UI::ConstraintProperty::Size, rect.height);

	DoLayout(elm);
}

void Unstable::UI::NWayLayout(UI::Rect rect, UI::Color color, Element* parent, s32 tIndex)
{
	UIState* s = _engine->GUIState;

	f32 ar = rect.width / rect.height;
	s32 tl = rect.width / 3 > 32 ? 32 : rect.width / 3;
	s32 br = rect.height / 3 > 32 ? 32 : rect.height / 3;
	float vtu = tl > br ? br : tl;

	Element* e0 = Box({ rect.x, rect.y, vtu, vtu }, color, parent, s->Textures[tIndex]);
	Element* e1 = Box({ rect.x + vtu, rect.y, rect.width - (vtu * 2), vtu }, color, parent, s->Textures[tIndex + 1]);
	Element* e2 = Box({ rect.x + rect.width - vtu, rect.y, vtu, vtu }, color, parent, s->Textures[tIndex + 2]);

	Element* e3 = Box({ rect.x, rect.y + vtu, vtu, rect.height - (vtu * 2) }, color, parent, s->Textures[tIndex + 3]);
	Element* e4 = Box({ rect.x + vtu, rect.y + vtu, rect.width - (vtu * 2), rect.height - (vtu * 2) }, color, parent, s->Textures[tIndex + 4]);
	Element* e5 = Box({ rect.x + rect.width - vtu, rect.y + vtu, vtu, rect.height - (vtu * 2) }, color, parent, s->Textures[tIndex + 5]);

	Element* e6 = Box({ rect.x, rect.y + rect.height - vtu, vtu, vtu }, color, parent, s->Textures[tIndex + 6]);
	Element* e7 = Box({ rect.x + vtu, rect.y + rect.height - vtu, rect.width - (vtu * 2), vtu }, color, parent, s->Textures[tIndex + 7]);
	Element* e8 = Box({ rect.x + rect.width - vtu, rect.y + rect.height - vtu, vtu, vtu }, color, parent, s->Textures[tIndex + 8]);
}

bool Unstable::UI::Button(UI::Rect rect, UI::Color color, UI::Element* parent)
{
	Element* e = Box(rect, UI::Clear, parent, nullptr);

	bool m = false;
	sf::Vector2i mpos = { (s32)(_engine->MouseX * _engine->UIScale), (s32)(_engine->MouseY * _engine->UIScale) };
	UI::Color cd = color;
	if (UI::PointRectIntersect(mpos, e->Rect))
	{
		cd = UI::Color(color.r * 0.9f, color.g * 0.9f, color.b * 0.9f, color.a);
		if (_engine->MouseButtonState[sf::Mouse::Button::Left])
		{
			cd = UI::Color(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, color.a);
			m = true;
		}
	}

	NWayLayout(rect, cd, parent, 9);

	return m;
}

UI::Element* Unstable::UI::Window(UI::Rect rect, UI::Color color, UI::Element* parent)
{
	Element* e0 = Box(rect, UI::Clear, parent, nullptr);
	NWayLayout({ 0,0,rect.width, rect.height }, color, e0, 0);
	return e0;
}

UI::Element* Unstable::UI::Box(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture)
{
	Element* elm = UI::GetNextElement();
	if (elm == nullptr)
	{
		// we are over the 2048 element limit
		assert(elm && "Failed to get new UI element. Have more than 2048 been drawn?");
	}

	SetParent(elm, parent);
	SetColor(elm, color);
	SetTexture(elm, texture);

	UI::ConstraintType ct = parent != nullptr ? UI::ConstraintType::Relative : UI::ConstraintType::Static;

	// x pos
	SetConstraint(elm, ct, UI::ConstraintAxis::Horizontal, UI::ConstraintProperty::Anchor, rect.x);
	// y pos
	SetConstraint(elm, ct, UI::ConstraintAxis::Vertical, UI::ConstraintProperty::Anchor, rect.y);
	// w
	SetConstraint(elm, UI::ConstraintType::Static, UI::ConstraintAxis::Horizontal, UI::ConstraintProperty::Size, rect.width);
	// h
	SetConstraint(elm, UI::ConstraintType::Static, UI::ConstraintAxis::Vertical, UI::ConstraintProperty::Size, rect.height);

	DoLayout(elm);

	return elm;
}

UI::Rect Unstable::UI::ResolveHorizontalAxisConstraint(Element* element, s32 constraintIndex)
{
	Constraint c = element->Constraints[constraintIndex];
	Rect r = { 0,0,0,0 };
	switch (c.Property)
	{
	case UI::ConstraintProperty::Anchor:
		r.x = c.Value;
		break;
	case UI::ConstraintProperty::Size:
		r.width = c.Value;
		break;
	default:
		break;
	}

	return r;
}

UI::Rect Unstable::UI::ResolveVerticalAxisConstraint(Element* element, s32 constraintIndex)
{
	Constraint c = element->Constraints[constraintIndex];
	Rect r = { 0,0,0,0 };
	switch (c.Property)
	{
	case UI::ConstraintProperty::Anchor:
		r.y = c.Value;
		break;
	case UI::ConstraintProperty::Size:
		r.height = c.Value;
		break;
	default:
		break;
	}

	return r;
}

UI::Rect Unstable::UI::ResolveParentRect(Element* element)
{
	if (element == nullptr)
	{
		sf::Vector2u size = _engine->Window->getSize();
		return { 0,0,0,0 };
	}

	Rect pr = ResolveParentRect(element->Parent);

	for (s32 i = 0; i < element->ConstraintIndex; i++)
	{
		Constraint c = element->Constraints[i];
		switch (c.Axis)
		{
		case UI::ConstraintAxis::Horizontal:
		{
			Rect r = ResolveHorizontalAxisConstraint(element, i);
			if (c.Property == UI::ConstraintProperty::Anchor)
				pr = AddRect(pr, { r.x, r.y, 0, 0 });
			else
				pr.width = r.width;
			break;
		}
		case UI::ConstraintAxis::Vertical:
		{
			Rect r = ResolveVerticalAxisConstraint(element, i);
			if (c.Property == UI::ConstraintProperty::Anchor)
				pr = AddRect(pr, { r.x, r.y, 0, 0 });
			else
				pr.height = r.height;
			break;
		}
		default:
			break;
		}
	}

	return pr;
}

void Unstable::UI::DoLayout(Element* element)
{
	Rect r = ResolveParentRect(element);

	element->Rect = AddRect(element->Rect, r);
}

UI::Element* Unstable::UI::Circle(UI::Rect rect, UI::Color color, UI::Element* parent, sf::Texture* texture)
{
	return nullptr;
}

Unstable::UI::Rect::Rect(float x, float y, float w, float h)
{
	this->x = x;
	this->y = y;
	this->width = w;
	this->height = h;
}

Unstable::UI::Color::Color(float r, float g, float b, float a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

UI::Rect Unstable::UI::AddRect(Rect r1, Rect r2)
{
	return { r1.x + r2.x, r1.y + r2.y, r1.width + r2.width, r1.height + r2.height };
}

bool Unstable::UI::PointRectIntersect(sf::Vector2i point, Rect rect)
{
	return point.x >= rect.x && point.x <= rect.x + rect.width && point.y >= rect.y && point.y <= rect.y + rect.height;
}
