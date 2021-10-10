#pragma once

#include "Engine.Forward.h"
#include <SFML/Graphics.hpp>
#include <Remotery.h>
#include <Windows.h>

#include "Debug.h"
#include "AI.h"
#include "Audio.h"
#include "Physics.h"
#include "UI.h"
#include "Rendering.h"
#include "ECS.h"
#include "Scheme.h"

namespace Unstable
{
    struct Engine
    {
        // remotery docs
        // https://github.com/Celtoys/Remotery
        Remotery* rmt;

        Audio::AudioSystem* Audio;
        World::World* World;
        Rendering::Rendering* Rendering;
        ECS::ECSWorld ECSWorld;
        Scheme::Data Scheme;
        UI::UIState* GUIState;
        Physics::Physics* Physics;

        bool KeyState[sf::Keyboard::Key::KeyCount];
        bool KeyPressedState[sf::Keyboard::Key::KeyCount];
        bool MouseButtonState[sf::Mouse::ButtonCount];
        bool MouseButtonPressedState[sf::Mouse::ButtonCount];
        u32 MouseDeltaX;
        u32 MouseDeltaY;
        u32 MouseX;
        u32 MouseY;
        f32 MouseScrollwheel;

        f32 controllerXPos;
        f32 controllerX;
        f32 controllerYPos;
        f32 controllerY;
        f32 controllerZPos;
        f32 controllerZ;

        f32 ScaledWorldX = 320;
        f32 ScaledWorldY;
        f32 cameraX;
        f32 cameraY;
        f32 UIScale;

        f32 minX;
        f32 minY;
        f32 maxX;
        f32 maxY;

        u32 maxParticlesPerFrame = 16;
        u32 particlesThisFrame;

        bool ShowPlayerDebug = false;

        std::vector<b2Body*> allBodies;
        std::map<std::string, sf::Texture*> textureCache;

        LARGE_INTEGER ticksPerSecond;
        double start = 0;
        double elapsed = 0;
        double deltatime = 0;
        double fixedAccumlator = 0;
        double fixedDeltatime = 1/60;
        u64 tick;
        u64 framecount;
        f32 tickPercent;
        u32 MaxFixedStepsPerFrame = 10;
        bool IsTickIterpolationOn = true;
        bool DebugPhysics;
        s32 CurrentUI;

        //SFML Window management
        sf::RenderWindow* Window;
        ConsoleContext* ConsoleContext;
    };

    void Engine_Init(Engine* engine);
    void Engine_Run(Engine* engine);

    void TimeInit(Engine* engine);
    double GetTime(Engine* engine);

    struct ConsoleContext
    {
        HANDLE ConsoleHandle;
        FILE* OutputStream;
        FILE* ErrorStream;
        FILE* InputStream;
        volatile bool ConsoleOpen;
        volatile bool IsCommandQueued;
        volatile char* Command;
    };

    bool ConsoleInit(Engine* engine);
    void ConsoleClose(Engine* engine);
    DWORD WINAPIV ConsoleThread(void* data);

}