#include <stdio.h>
#include <SFML/Graphics.hpp>

#include "Engine.h"

void Unstable::Engine_Init(Engine* engine)
{
	//rmt_CreateGlobalInstance(&rmt);
	/*if (!ConsoleInit(engine))
	{
		DebugBreak();
		return;
	}*/

	//Window init logic
	engine->Window = new sf::RenderWindow(sf::VideoMode(1280, 720), "Unstable (Development)");
	engine->Window->setVerticalSyncEnabled(true);

	TimeInit(engine);
	std::srand(std::time(nullptr)); // use current time as seed for random generator
	engine->fixedDeltatime = 1 / 30.0;
	engine->ScaledWorldX = 240;
	engine->UIScale = 0.75f;
	engine->CurrentUI = 1;
	//engine->ShowPlayerDebug = true;

	engine->minX = -1;
	engine->minY = -1;
	engine->maxX = 1;
	engine->maxY = 1;

	AI::Init(engine);
	Audio::Init(&engine->Audio);
	Rendering::Init(&engine->Rendering);
	Physics::Init(engine, &engine->Physics);
	ECS::Init(engine);
	UI::Init(engine);
	Scheme::Init(engine);
	Debug::Init(engine);
}

void Unstable::Engine_Run(Engine* engine)
{
	bool innerBreak = false;
	while (engine->Window->isOpen())
	{
		//rmt_BeginCPUSample(MainLoop, 0);

		//Clear any pressed keys
		for (u32 i = 0; i < sf::Keyboard::Key::KeyCount; i++)
		{
			engine->KeyPressedState[i] = false;
		}
		for (u32 i = 0; i < sf::Mouse::ButtonCount; i++)
		{
			engine->MouseButtonPressedState[i] = false;
		}
		engine->MouseDeltaX = 0;
		engine->MouseDeltaY = 0;
		engine->MouseScrollwheel = 0;
		engine->controllerX = 0;
		engine->controllerY = 0;
		engine->controllerZ = 0;

		//Window event pump
		//rmt_BeginCPUSample(EventPump, 0);
		sf::Event event;
		while (engine->Window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				engine->Window->close();
				ConsoleClose(engine);
				innerBreak = true;
				break;
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code < sf::Keyboard::KeyCount && event.key.code >= 0)
				{
					if (engine->KeyState[event.key.code] == false)
					{
						//last frame was false, is now true
						engine->KeyPressedState[event.key.code] = true;
					}
					else
					{
						engine->KeyPressedState[event.key.code] = false;
					}
					engine->KeyState[event.key.code] = true;
				}
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code < sf::Keyboard::KeyCount && event.key.code >= 0)
				{
					engine->KeyPressedState[event.key.code] = false;
					engine->KeyState[event.key.code] = false;
				}
			}
			else if (event.type == sf::Event::MouseMoved)
			{
				engine->MouseDeltaX = engine->MouseX - event.mouseMove.x;
				engine->MouseDeltaY = engine->MouseY - event.mouseMove.y;

				engine->MouseX = event.mouseMove.x;
				engine->MouseY = event.mouseMove.y;
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				if (engine->MouseButtonState[event.mouseButton.button] == false)
				{
					//last frame was false, is now true
					engine->MouseButtonPressedState[event.mouseButton.button] = true;
				}
				else
				{
					engine->MouseButtonPressedState[event.mouseButton.button] = false;
				}
				engine->MouseButtonState[event.mouseButton.button] = true;
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				engine->MouseButtonPressedState[event.mouseButton.button] = false;
				engine->MouseButtonState[event.mouseButton.button] = false;
			}
			else if (event.type == sf::Event::MouseWheelScrolled)
			{
				engine->MouseScrollwheel = event.mouseWheelScroll.delta;
			}
			else if (event.type == sf::Event::JoystickMoved)
			{
				if (event.joystickMove.axis == sf::Joystick::Axis::X)
				{
					engine->controllerX = event.joystickMove.position / 100.0f - engine->controllerXPos;
					engine->controllerXPos = event.joystickMove.position / 100.0f;
				}
				if (event.joystickMove.axis == sf::Joystick::Axis::Y)
				{
					engine->controllerY = event.joystickMove.position / 100.0f - engine->controllerYPos;
					engine->controllerYPos = event.joystickMove.position / 100.0f;
				}
				if (event.joystickMove.axis == sf::Joystick::Axis::Z)
				{
					engine->controllerZ = event.joystickMove.position / 100.0f - engine->controllerZPos;
					engine->controllerZPos = event.joystickMove.position / 100.0f;
				}
			}
			else if (event.type == sf::Event::JoystickButtonPressed)
			{
				//printf("Joystick:%i\n", event.joystickButton.button);
			}
		}
		if (innerBreak) break;
		//rmt_EndCPUSample();

		//Update time
		double elapsed = GetTime(engine);
		engine->deltatime = elapsed - engine->elapsed;
		engine->elapsed = elapsed;
		//printf("\rTime:%f %f", 1.0f / engine->deltatime, engine->elapsed);
		engine->framecount++;

		//Update all input buffers etc
		ECS::EarlyFrameUpdate(engine);

		//Evaluate any console commands (scheme)
		if (engine->ConsoleContext != nullptr && engine->ConsoleContext->IsCommandQueued)
		{
			Scheme::Eval(engine, (char*)engine->ConsoleContext->Command);
			free((void*)engine->ConsoleContext->Command);
			engine->ConsoleContext->IsCommandQueued = false;
		}
		if (engine->KeyPressedState[sf::Keyboard::F2])
		{
			engine->fixedDeltatime = 1 / 10.0;
			engine->fixedAccumlator = 0;
		}
		if (engine->KeyPressedState[sf::Keyboard::F3])
		{
			engine->fixedDeltatime = 1 / 30.0;
			engine->fixedAccumlator = 0;
		}
		if (engine->KeyPressedState[sf::Keyboard::F4])
		{
			engine->fixedDeltatime = 1 / 60.0;
			engine->fixedAccumlator = 0;
		}
		if (engine->KeyPressedState[sf::Keyboard::F5])
		{
			engine->ShowPlayerDebug = !engine->ShowPlayerDebug;
		}
		if (engine->KeyPressedState[sf::Keyboard::Num1])
		{
			engine->ScaledWorldX = 240;
		}
		if (engine->KeyPressedState[sf::Keyboard::Num2])
		{
			engine->ScaledWorldX = 320;
		}
		if (engine->KeyPressedState[sf::Keyboard::Num3])
		{
			engine->ScaledWorldX = 640;
		}
		if (engine->KeyPressedState[sf::Keyboard::Num4])
		{
			engine->ScaledWorldX = 1280;
		}
		if (engine->KeyPressedState[sf::Keyboard::Num0])
		{
			engine->DebugPhysics = !engine->DebugPhysics;
		}

		//Clear rendering (and draw lists)
		Rendering::Clear(engine);
		Debug::PostRender();

		//Scale the view to a ScaledWorldX px wide world
		{
			auto wsize = engine->Window->getSize();
			f32 scale = engine->ScaledWorldX / wsize.x;
			engine->ScaledWorldY = wsize.y * scale;

			sf::FloatRect visibleArea(engine->cameraX, engine->cameraY, engine->ScaledWorldX, engine->ScaledWorldY);
			engine->Window->setView(sf::View(visibleArea));
		}

		engine->particlesThisFrame = 0;

		//Fixed update
		engine->fixedAccumlator += engine->deltatime;
		engine->tickPercent = 0.0f;
		u32 ticks = 0;
		while (engine->fixedAccumlator >= engine->fixedDeltatime)
		{
			engine->fixedAccumlator -= engine->fixedDeltatime;
			if (ticks < engine->MaxFixedStepsPerFrame)
			{
				ECS::PrePhysicsUpdate(engine);
				Physics::Update(engine);
				ECS::PostPhysicsUpdate(engine);

				engine->tick++;
			}
			ticks++;
		}

		//Turn interpolation on/off
		if (engine->KeyPressedState[sf::Keyboard::F1])
			engine->IsTickIterpolationOn = !engine->IsTickIterpolationOn;

		//If interpolation is on, we display a percent between ticks (for interpolation)
		//else, we set the percent to 1.0 which causes the interpolation to allway be 100% to the current ticks value
		if (engine->IsTickIterpolationOn)
			engine->tickPercent = engine->fixedAccumlator / engine->fixedDeltatime;
		else
			engine->tickPercent = 1.0f;

		if (engine->DebugPhysics)
		{
			engine->Physics->world->DebugDraw();
		}

		//Every frame updates
		Scheme::Update(engine);
		ECS::Update(engine);
		UI::Update(engine);
		Audio::Update(engine, engine->Audio);

		AI::GenerateNavMesh();

		//std::vector<sf::Vector2f> path = AI::GeneratePath(engine, sf::Vector2f(-99, -99), sf::Vector2f(99, 99));
		
		//Rendering
		Rendering::Render(engine);

		//Scale the view back to absolute
		{
			auto wsize = engine->Window->getSize();
			sf::FloatRect visibleArea(0, 0, wsize.x * engine->UIScale, wsize.y * engine->UIScale);
			engine->Window->setView(sf::View(visibleArea));
		}

		// temp fps
		{
			char buffer[512];
			sprintf(buffer, "FPS: %f", 1.0 / engine->deltatime);
			UI::Text(buffer, UI::Rect(0, 0, 100, 25), UI::White, nullptr, engine->GUIState->Fonts[0]);
		}

		UI::Render(engine);

		//Rendering End

		Rendering::Present(engine);
		//rmt_EndCPUSample();
	}

	//Tell each system to shutdown
	Audio::Shutdown(engine->Audio);
	Scheme::Shutdown(engine);

	//Cleanup remotery
	//rmt_DestroyGlobalInstance(rmt);

	ConsoleClose(engine);
}

bool Unstable::ConsoleInit(Engine* engine)
{
	ConsoleContext* console = (ConsoleContext*)calloc(1, sizeof(ConsoleContext));
	if (console == nullptr)
	{
		DebugBreak();
		return false;
	}
	engine->ConsoleContext = console;

	// try and allocate a console to this program
	if (AllocConsole())
	{
		freopen_s(&engine->ConsoleContext->OutputStream, "CONOUT$", "w", stdout);
		freopen_s(&engine->ConsoleContext->ErrorStream, "CONOUT$", "w", stderr);
		freopen_s(&engine->ConsoleContext->InputStream, "CONIN$", "r", stdin);
		engine->ConsoleContext->ConsoleOpen = true;
		// try and create the console input thread
		engine->ConsoleContext->ConsoleHandle = CreateThread(nullptr, 0, ConsoleThread, console, 0, nullptr);
		if (engine->ConsoleContext->ConsoleHandle == nullptr)
		{
			DebugBreak();
			return false;
		}
		return true;
	}
	else
	{
		engine->ConsoleContext->ConsoleOpen = false;
	}
	return false;
}

void Unstable::ConsoleClose(Engine* engine)
{
	if (engine->ConsoleContext != nullptr && engine->ConsoleContext->ConsoleHandle != nullptr)
	{
		engine->ConsoleContext->ConsoleOpen = false;
		// Wait for the thread to join
		WaitForSingleObject(engine->ConsoleContext->ConsoleHandle, 5000);
	}

	// free the windows console instance
	FreeConsole();

	// free the console context we created
	free(engine->ConsoleContext);
}

DWORD WINAPIV Unstable::ConsoleThread(void* data)
{
	ConsoleContext* context = (ConsoleContext*)data;

	char inputBuffer[512];
	char formatBuffer[1024];
	while (context->ConsoleOpen)
	{
		if (context->IsCommandQueued) continue;

		fprintf(stdout, "\n>"); //Prompt for input
		fgets(inputBuffer, 512, context->InputStream);
		if (inputBuffer[0] != '\n' &&
			strlen(inputBuffer) > 1)
		{
			snprintf(formatBuffer, 1024, "(write %s)", inputBuffer);
			//s7_eval_c_string(sc, formatBuffer);
			if (!context->IsCommandQueued)
			{
				char* buff = (char*)malloc(sizeof(char) * 1024);
				if (buff != NULL)
				{
					strcpy(buff, formatBuffer);
					context->Command = buff;
					context->IsCommandQueued = true;
				}
			}
		}
	}
	printf("[Scheme] Console thread exit\n");

	return 0;
}

void Unstable::TimeInit(Engine* engine)
{
	QueryPerformanceFrequency(&engine->ticksPerSecond);
	engine->start = GetTime(engine);
}

double Unstable::GetTime(Engine* engine)
{
	LARGE_INTEGER ct;
	QueryPerformanceCounter(&ct);
	return ((double)ct.QuadPart / (double)engine->ticksPerSecond.QuadPart) - engine->start;
}
