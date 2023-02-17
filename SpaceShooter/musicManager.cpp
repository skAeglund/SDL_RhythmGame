#include "musicManager.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "assets.h"

MusicManager::MusicManager(Beat inputBeats[3])
{
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
	// load music
	for (int i = 0; i < NUMBER_OF_BEATS; i++)
	{
		beats[i] = inputBeats[i];
		beats[i].music = Mix_LoadMUS(inputBeats[i].path);
		if (beats[i].music == nullptr)
		{
			printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
		}
	}
	currentBeat = beats[0];
	data = new MusicData();

	transitionSound = Mix_LoadWAV(Assets::transitionSFX);
	glitchSound = Mix_LoadWAV(Assets::glitchSFX);

	//load laser SFXs
	for (int i = 0; i < 4; i++)
	{
		std::string path = Assets::laserSFX + std::to_string(i + 1) + ".wav";
		laserSounds[i] = Mix_LoadWAV(path.c_str());
	}
	badLaserSound = Mix_LoadWAV(Assets::badLaserSFX);

	if (badLaserSound == nullptr || glitchSound == nullptr || transitionSound == nullptr )
	{
		printf("Failed to load sound! SDL_mixer Error: %s\n", Mix_GetError());
	}

	data->quarterNoteLength = 60.f / currentBeat.BPM;
	data->halfNoteLength = data->quarterNoteLength * 2.f;
	data->wholeNoteLength = data->quarterNoteLength * static_cast<float>(currentBeat.timeSignature);

	Mix_ReserveChannels(2);
}

void MusicManager::startPlaying()
{
	Mix_FadeInMusic(currentBeat.music, -1, (int)(data->wholeNoteLength*2000));
	loadTime = 0.2f;
	data->musicIsPlaying = true;
}
void MusicManager::stopPlaying() const
{
	Mix_FadeOutMusic(2000);
	Mix_PlayChannel(transitionChannel, transitionSound, 0);
	Mix_Volume(transitionChannel, MIX_MAX_VOLUME);
	isTransitioning = true;
	data->musicIsPlaying = false;
}
void MusicManager::changeBeat(int index)
{
	const int transitionVolume = currentBeatIndex == 0 ? 50 : MIX_MAX_VOLUME;
	if (index == -1)
		currentBeatIndex = ++currentBeatIndex % 3; // toggle
	else
		currentBeatIndex = index;
	Mix_FadeOutMusic(2000);
	isChangingBeat = true;
	data->quarterNoteLength = 60.f / beats[currentBeatIndex].BPM;
	data->halfNoteLength = data->quarterNoteLength * 2;
	data->wholeNoteLength = data->quarterNoteLength * currentBeat.timeSignature;

	// play transition sfx to mask the transition
	if (Mix_PlayingMusic() && !Mix_Playing(transitionChannel))
	{
		Mix_PlayChannel(transitionChannel, transitionSound, 0);

		Mix_Volume(transitionChannel, transitionVolume);
		isTransitioning = true;
	}
}

// Checks if the time that has passed since the last beat is within acceptable timing. 
// Allows for a slightly longer offset after the beat, compared to before the beat
bool MusicManager::getBeatActiveState(float timeSinceLastBeat, float noteLength) const
{
	return timeSinceLastBeat < acceptedOffset * 1.25f || timeSinceLastBeat > noteLength - acceptedOffset * 0.75f;
}

bool MusicManager::update(float deltaTime)
{
	if (isLoading)
	{
		// Lets the music play muted for a time and then sets the position to the start.
		// I'm unsure about the possible latency of starting music, but I assume that 
		// changing position of a song that's already playing is faster
		const double currentTime = Mix_GetMusicPosition(currentBeat.music) * 1000;
		if (currentTime * 0.001 > loadTime)
		{
			isLoading = false;
			// adjust for audio delay - this is not precise but will do for now
			data->timeSinceLastQuarterNote = -0.015f;
			data->timeSinceLastWholeNote = -0.015f;
			data->timeSinceLastHalfNote = -0.015f;
			data->currentQuarterNote = 1;
			Mix_SetMusicPosition(0);
		}
		return false;
	}
	if (isChangingBeat)
	{
		if (Mix_GetMusicVolume(currentBeat.music) == 0)
		{
			isTransitioning = false;
			currentBeat = beats[currentBeatIndex];
			isChangingBeat = false;
			isLoading = true;
			startPlaying();
		}
	}
	if (!Mix_PlayingMusic()) return false;

	// update timers
	data->timeSinceLastWholeNote += deltaTime;
	data->timeSinceLastHalfNote += deltaTime;
	data->timeSinceLastQuarterNote += deltaTime;

	// quarter note has passed, which is true when all other note lengths passes
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

	// update active state of each note length - decides when certain input actions are acceptable
	data->wholeNoteActive = getBeatActiveState(data->timeSinceLastWholeNote, data->wholeNoteLength);
	data->halfNoteActive = getBeatActiveState(data->timeSinceLastHalfNote, data->halfNoteLength);
	data->quarterNoteActive = getBeatActiveState(data->timeSinceLastQuarterNote, data->quarterNoteLength);

	// update the progress of each note length - from 0 (beat just happened) to 1 (note length just ended)
	data->quarterNoteProgress = data->timeSinceLastQuarterNote / data->quarterNoteLength;
	data->wholeNoteProgress = data->timeSinceLastWholeNote / data->wholeNoteLength;
	data->halfNoteProgress = data->timeSinceLastHalfNote / data->halfNoteLength;

	if (data->halfNoteProgress < 0.5f)
		data->pulseMultiplier = data->halfNoteProgress * 2.f;
	else
		data->pulseMultiplier = 1 - (data->halfNoteProgress - 0.5f) * 2.f;

	return true;
}

void MusicManager::printStats() const
{
	const SHORT x = 80;
	const char* trueString = "TRUE -       ";
	const char* falseString = "     - FALSE";

	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)0});
	std::cout << "-------------------------------------- ";
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)1});
	std::cout << "|                 MUSIC";

	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)2});
	std::cout << "| Current Quarter note: " << data->currentQuarterNote;
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)3});
	std::cout << "| Whole note synched:   " << (data->wholeNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)4});
	std::cout << "| Half note synched:    " << (data->halfNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)5});
	std::cout << "| Quarter note synched: " << (data->quarterNoteActive ? trueString : falseString);
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)6});
	std::cout << "| Time since whole note: " << data->timeSinceLastWholeNote << "\n";
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)7});
	std::cout << "| Time since half note : " << data->timeSinceLastHalfNote << "\n";
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)8});
	std::cout << "| Is transitioning:     " << (isTransitioning ? trueString : falseString) << "\n";
	SetConsoleCursorPosition(hOut, COORD{x, (SHORT)9});

	std::cout << "-------------------------------------- ";

	for (int i = 0; i < 8; i++)
	{
		SetConsoleCursorPosition(hOut, {(SHORT)(x + 37), (SHORT)(1 + i)});
		std::cout << "| ";
	}

	SetConsoleCursorPosition(hOut, COORD{(SHORT)0, (SHORT)10});
	std::cout.flush();
}

void MusicManager::unload() const
{
	for (size_t i = 0; i < std::size(beats); i++)
	{
		if (beats[i].music != nullptr)
			Mix_FreeMusic(beats[i].music);
	}
	for (size_t i = 0; i < std::size(laserSounds); i++)
	{
		if (laserSounds[i] != nullptr)
			Mix_FreeChunk(laserSounds[i]);
	}
	if (transitionSound != nullptr)
		Mix_FreeChunk(transitionSound);

	if (transitionSound != nullptr)
		Mix_FreeChunk(glitchSound);
	delete data;
	Mix_Quit();
}

void MusicManager::playLaserSound(bool successfulLaser) const
{
	// atm only the first laser sound is being played
	// but later I will make slightly different laser sounds in rotation
	
	//int index = data->currentQuarterNote - 1; 

	const int channel = Mix_PlayChannel(-1, successfulLaser ? laserSounds[0] : badLaserSound, 0);
	Mix_Volume(channel, successfulLaser ? 50 : MIX_MAX_VOLUME);
}

void  MusicManager::playGlitchSound() const
{
	Mix_PlayChannel(glitchChannel, glitchSound, 0);
	const int volume = rand() % 28 + 70;
	Mix_Volume(glitchChannel, volume);
}

Beat::Beat(int bpm, int signature, const char* path) : BPM(bpm), timeSignature(signature), path(path)
{
}
