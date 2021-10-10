#include "Audio.h"


using namespace Unstable::Audio;

void Unstable::Audio::Init(AudioSystem** system)
{
	// create and init fmod
	AudioSystem* sys = (AudioSystem*)calloc(1, sizeof(AudioSystem));
	(*system) = sys;

	sys->fmod = NULL;
	FMOD_RESULT fres = FMOD_System_Create(&sys->fmod, FMOD_VERSION);
	if (fres != FMOD_OK)
	{
		// error here
		// Should make an error system
		// I have a good one I made in Fir that I'll add tomorrow, its just an easy to use macro and does the proper asserts
		printf("Fmod broken, can't start. Error code:%i", fres);
		exit(fres);
	}

	fres = FMOD_System_Init(sys->fmod, 512, FMOD_INIT_NORMAL, NULL);
	// after this fmod should be fully initialized and can be used

	// create the sound, loaded from a file
	FMOD_SOUND* sound = NULL;
	FMOD_System_CreateSound(sys->fmod, "./assets/audio/beep.mp3", FMOD_DEFAULT, NULL, &sound);

	// play the sound and return a channel, essentially a audio source handle
	FMOD_CHANNEL* channel = NULL;
	//FMOD_System_PlaySound(sys->fmod, sound, NULL, false, &channel);

}

void Unstable::Audio::Shutdown(AudioSystem* system)
{
	FMOD_System_Close(system->fmod);
}

void Unstable::Audio::Update(Unstable::Engine* engine, AudioSystem* system)
{
}

AudioHandle* Unstable::Audio::CreateHandle(AudioSystem* system)
{
	return nullptr;
}

AudioHandle* Unstable::Audio::UpdateHandle(AudioSystem* system, AudioHandle handle)
{
	return nullptr;
}
