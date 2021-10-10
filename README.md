# Ludum-Dare-49 "Unstable"
Hosting for [Ludum dare 49](https://ldjam.com/events/ludum-dare/49)

[Our entry](https://ldjam.com/events/ludum-dare/49/unstable-17)

Source code to the game & engine as of the deadline on 2021-10-4.
Built all from scratch (as long as you draw the line at window libs like sfml).
We decided a few high level things before starting to keep things simple enough to actually finish in 72hrs such as:

1. Single threaded - adding multi threading / concurrency causes lots of bugs if architecture isn't perfect and set up in a way to accommodate it.
2. Used Box2D and SFML sprite rendering, for speed in development, in the future we would replace SFML for our own custom rendering.
3. We used CPP in order to be compatible with absolutely everything, and have no issues with wrappers (C#).
4. Made the general file/module structure of Engine Struct, containing a data struct for each system.
Each system then contained Init() Update() Shutdown(). This wasn't a rule, but where we started with to get a multi part system up and going where two people could work on it and not step on each others toes.

Various used libs:

  - [SFML](https://www.sfml-dev.org/) for window creation & 2d rendering

  - [FLECS](https://github.com/SanderMertens/flecs) for ecs, all game state and logic

  - [S7](https://ccrma.stanford.edu/software/snd/snd/s7.html) Scheme for all on disk assets, scripting, level loading

  - [FMOD](https://www.fmod.com/) For audio
  
  - [Box2D](https://box2d.org/) Physics

## UI
```Unstable/Src/UI.h / .cpp```

Quick n dirty immediate mode UI, definitely will be improved on in the future.

## ECS
```Unstable/Src/ECS.h / .cpp```

Using flecs cpp api, all systems stored into a few vectors and manually ran instead of using their default system ordering.
This was done to support a simulation tick, with a rendering (every frame) interpolation to decouple simulation rate from render rate.


