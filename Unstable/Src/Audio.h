#pragma once

#include <cstdint>
#include <vector>
// fmod docs
// https://www.fmod.com/resources/documentation-api?version=2.02&page=white-papers-getting-started.html
#include <fmod.h>


#include "Engine.Forward.h"

namespace Unstable
{
    namespace Audio
    {
        struct AudioHandle
        {
            uint32_t ID;
            //fmod stuff
        };

        struct AudioSystem
        {
            FMOD_SYSTEM* fmod;
            std::vector<AudioHandle> Handles;
        };

        void Init(AudioSystem** system);
        void Shutdown(AudioSystem* system);
        void Update(Unstable::Engine* engine, AudioSystem* system);

        AudioHandle* CreateHandle(AudioSystem* system);
        AudioHandle* UpdateHandle(AudioSystem* system, AudioHandle handle);
    }
}