#pragma once

//All core engine system declares, for resolving types correctly
#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

namespace Unstable
{
	struct Engine;
	struct ConsoleContext;

	namespace Scheme
	{
		struct Data;
	}

	namespace AI {}

	namespace Asset {}

	namespace Audio
	{
		struct AudioHandle;
		struct AudioSystem;
	}

	namespace ECS
	{
		struct ECSWorld;
	}

	namespace Input {}

	namespace Physics {}

	namespace Rendering
	{
		struct Rendering;
	}

	namespace UI
	{
		struct Rect;
		struct Color;
		struct Constraint;
		struct Element;
		struct UIState;
	}

	namespace World
	{
		struct World;
	}
}

