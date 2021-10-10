#pragma once

#include <cstdint>

#include "Engine.Forward.h"

namespace Unstable
{
    namespace World
    {
        struct World
        {
            uint32_t LoadedChunks;
        };

        void CreateWorld(World* world);
        void DeleteWorld(World* world);

        void LoadChunk(World* world);
        void UnloadChunk(World* world);
    }
}
