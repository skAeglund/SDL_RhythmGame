#include "musicManager.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>

bool MusicManager::initialize(Beat inputBeats[NUMBER_OF_BEATS])
{
	bool result = true;
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		result = false;
	}
	// load music
	for (int i = 0; i < NUMBER_OF_BEATS; i++)
	{
		beats[i] = inputBeats[i];
		beats[i].music = Mix_LoadMUS(inputBeats[i].path);
		if (beats[i].music == NULL)
		{
			printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
			result = false;
		}
	}
	currentBeat = beats[0];
	data = new MusicData();
	//load SFX
	transitionSound = Mix_LoadWAV("Content/Audio/SFX_Transition.wav");
	if (transitionSound == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		result = false;
	}
	//load SFX
	glitchSound = Mix_LoadWAV("Content/Audio/SFX_CollisionGlitch.wav");
	if (glitchSound == NULL)
	{
		printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		result = false;
	}
	for (int i = 0; i < 4; i++)
	{
		std::string path = "Content/Audio/SFX_Laser" + std::to_string(i + 1) + ".wav";
		laserSounds[i] = Mix_LoadWAV(path.c_str());
	}

	data->quarterNoteLength = 60 / (double)currentBeat.BPM;
	data->halfNoteLength = data->quarterNoteLength * 2;
	data->wholeNoteLength = data->quarterNoteLength * currentBeat.timeSignature;

	Mix_ReserveChannels(2);
	return result;
}
void MusicManager::startPlaying()
{
	Mix_FadeInMusic(currentBeat.music, -1, (int)(data->wholeNoteLength*2000));
	loadTime = 0.2;
}
void MusicManager::stopPlaying()
{
	Mix_FadeOutMusic(2000);
	int channel = Mix_PlayChannel(transitionChannel, transitionSound, 0);
	Mix_Volume(transitionChannel, MIX_MAX_VOLUME);
	MusicManager::isTransitioning = true;
}
void MusicManager::changeBeat(int index)
{
	int transitionVolume = currentBeatIndex == 0 ? 50 : MIX_MAX_VOLUME;
	if (index == -1)
		currentBeatIndex = ++currentBeatIndex % 3; // toggle
	else
		currentBeatIndex = index;
	Mix_FadeOutMusic(2000);
	isChangingBeat = true;
	data->quarterNoteLength = 60 / (double)beats[currentBeatIndex].BPM;
	data->halfNoteLength = data->quarterNoteLength * 2;
	data->wholeNoteLength = data->quarterNoteLength * currentBeat.timeSignature;

	// play transition sfx to mask the transition
	if (Mix_PlayingMusic() && !Mix_Playing(transitionChannel))
	{
		Mix_PlayChannel(transitionChannel, transitionSound, 0);
		
		Mix_Volume(transitionChannel, transitionVolume);
		MusicManager::isTransitioning = true;
	}
}


bool MusicManager::update(double deltaTime)
{
	if (isLoading)
	{
		// Lets the music play muted for a time and then sets the position to the start...
		// I'm unsure about the possible latency of starting music, but I assume that 
		// changing position of a song that's already playing is faster
		double currentTime = Mix_GetMusicPosition(currentBeat.music) * 1000;
		if (currentTime * 0.001 > loadTime)
		{
			isLoading = false;
			// adjust for audio delay - this is not precise but will do for now
			data->timeSinceLastQuarterNote = -0.015;
			data->timeSinceLastWholeNote = -0.015;
			data->timeSinceLastHalfNote = -0.015;
			data->currentQuarterNote = 1;
			Mix_SetMusicPosition(0);
		}
		return false;
	}
	if (isChangingBeat)
	{
		if (Mix_GetMusicVolume(currentBeat.music) == 0)
		{
			MusicManager::isTransitioning = false;
			currentBeat = beats[currentBeatIndex];
			isChangingBeat = false;
			isLoading = true;
			startPlaying();
		}
	}
	if (!Mix_PlayingMusic()) return false;

	data->timeSinceLastWholeNote += deltaTime;
	data->timeSinceLastHalfNote += deltaTime;
	data->timeSinceLastQuarterNote += deltaTime;

	if (data->timeSinceLastQuarterNote >= data->quarterNoteLength)
	{
		
		data->timeSinceLastQuarterNote = data->timeSinceLastQuarterNote - data->quarterNoteLength;
		data->currentQuarterNote = (data->currentQuarterNote++ % currentBeat.timeSignature) + 1;
		onQuarterNote();
		if (data->currentQuarterNote == 1)
		{
			data->timeSinceLastWholeNote = data->timeSinceLastWholeNote - data->wholeNoteLength;
			data->timeSinceLastHalfNote = data->timeSinceLastHalfNote - data->halfNoteLength;
		}
		else if (data->currentQuarterNote == 3)
		{
			data->timeSinceLastHalfNote = data->timeSinceLastHalfNote - data->halfNoteLength;
		}
	}
	data->wholeNoteActive =
		data->timeSinceLastWholeNote < acceptedOffset * 1.25 ||
		data->timeSinceLastWholeNote > data->wholeNoteLength - acceptedOffset * 0.75;

	data->quarterNoteActive =
		data->timeSinceLastQuarterNote < acceptedOffset * 1.25 ||
		data->timeSinceLastQuarterNote > data->quarterNoteLength - acceptedOffset * 0.75;

	data->halfNoteActive =
		data->timeSinceLastHalfNote < acceptedOffset * 1.25 ||
		data->timeSinceLastHalfNote > data->halfNoteLength - acceptedOffset * 0.75;

	data->quarterNoteProgress = data->timeSinceLastQuarterNote / data->quarterNoteLength;
	data->wholeNoteProgress = data->timeSinceLastWholeNote / data->wholeNoteLength;
	data->halfNoteProgress = data->timeSinceLastHalfNote / data->halfNoteLength;

	if (data->halfNoteProgress < 0.5f)
		data->pulseMultiplier = data->halfNoteProgress * 2;
	else
		data->pulseMultiplier = 1 - (data->halfNoteProgress - 0.5f) * 2;

	return true;
}

void MusicManager::printStats()
{
	SHORT x = 80;
	const char* trueString =  "TRUE -       ";
	const char* falseString = "     - FALSE";

	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)0 });
	std::cout << "-------------------------------------- ";
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)1 });
	std::cout << "|                 MUSIC";

	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)2 });
	std::cout << "| Current Quarter note: " << data->currentQuarterNote;
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)3 });
	std::cout << "| Whole note synched:   " << (data->wholeNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)4 });
	std::cout << "| Half note synched:    " << (data->halfNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)5 });
	std::cout << "| Quarter note synched: " << (data->quarterNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)6 });
	std::cout << "| Time since whole note: " << data->timeSinceLastWholeNote << "\n";
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)7 });
	std::cout << "| Time since half note : " << data->timeSinceLastHalfNote << "\n";
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)8 });
	std::cout << "| Is transitioning:     " << (isTransitioning ? trueString : falseString) << "\n";
	SetConsoleCursorPosition(hOut, COORD{ x, (SHORT)9 });
	
	std::cout << "-------------------------------------- ";

	for (int i = 0; i < 8; i++)
	{
		SetConsoleCursorPosition(hOut, { (SHORT)(x + 37), (SHORT)(1 + i) });
		std::cout << "| ";
	}

	SetConsoleCursorPosition(hOut, COORD{ (SHORT)0, (SHORT)10 });
	std::cout.flush();
}



void MusicManager::unload()
{
	for (size_t i = 0; i < sizeof(beats) / sizeof(beats[0]); i++)
	{
		Mix_FreeMusic(beats[i].music);
	}
	for (size_t i = 0; i < sizeof(laserSounds) / sizeof(laserSounds[0]); i++)
	{
		Mix_FreeChunk(laserSounds[i]);
	}
	Mix_FreeChunk(transitionSound);
	Mix_FreeChunk(glitchSound);
	delete data;
	Mix_Quit();
}

void MusicManager::playLaserSound()
{
	// atm only the first laser sound is being played
	// but later I will make slightly different laser sounds in rotation
	
	//int index = data->currentQuarterNote - 1; 

	int channel = Mix_PlayChannel(-1, laserSounds[0], 0);
	Mix_Volume(channel, 50);
}

void  MusicManager::playGlitchSound()
{
	Mix_PlayChannel(glitchChannel, glitchSound, 0);
	int volume = rand() % 20 + 75;
	Mix_Volume(glitchChannel, volume);
}


Beat::Beat(int bpm, int signature, const char* path)
{
	BPM = bpm;
	timeSignature = signature;
	this->path = path;
}
