#pragma once

#include <s7.h>

#include "Engine.Forward.h"

namespace Unstable
{
	namespace Scheme
	{
		struct Data
		{
			s7_scheme* s7;
		};

		void Init(Engine* engine);
		void Shutdown(Engine* engine);
		void Update(Engine* engine);
		void Eval(Engine* engine, char* eval);

		//Scheme Math
		s7_pointer s_Math_LeftShift(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Math_RightShift(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Math_Or(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Math_And(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Math_Not(s7_scheme* sc, s7_pointer args);

		//Engine
		s7_pointer s_GetTime(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Random(s7_scheme* sc, s7_pointer args);
		s7_pointer s_BindKeycode(s7_scheme* sc, s7_pointer args);

		//Memory
		s7_pointer s_MemSetF32(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemSetU32(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemSetU16(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemSetU8(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemGetF32(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemGetU32(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemGetU16(s7_scheme* sc, s7_pointer args);
		s7_pointer s_MemGetU8(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Free(s7_scheme* sc, s7_pointer args);
		s7_pointer s_Malloc(s7_scheme* sc, s7_pointer args);
	}
}