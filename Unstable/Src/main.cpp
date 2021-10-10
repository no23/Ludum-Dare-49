#include "Engine.Forward.h"
#include "Engine.h"


int main()
{
	Unstable::Engine engine{};
	Unstable::Engine_Init(&engine);
	Unstable::Engine_Run(&engine);
	
	return 0;
}