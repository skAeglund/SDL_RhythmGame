#include "engine.h"
#include <algorithm>
#include <windows.h>
#include <iomanip>
#include <cmath>
#include "collision.h"
#include "vector2.h"
#include "musicManager.h"
#include "game.h"
#include <SDL_image.h>

#define _WIN32_WINNT 0x0500
using namespace Vector2D;
using namespace Collision;

namespace Engine
{
	SDL_Window* window;
	SDL_Renderer* render;
	SDL_Rect windowRect;

	Uint64 previousTicks;
	double deltaTime = 0;
	double elapsedTime = 0;
	int framerate;

	std::vector<Position> positions;
	std::vector<Velocity> velocities;
	std::vector<Rotation> rotations;
	std::vector<Appearance> appearances;
	std::vector<Tag> tags;
	int objectCount;
	int playerIndex;

	std::vector<Laser> lineList;
	std::vector<ObjectPendingDeletion> objectsToDelete;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* backgroundTexture;

	bool keys[SDL_NUM_SCANCODES] = { false };
	int collisionChecksPerFrame;

#pragma region
	// initializes SDL, window, renderer, loads textures and sets console settings
	bool initializeEngine(const char* textureArr[], int textureCount)
	{
		// Initialize SDL
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
			return false;
		}

		window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
		if (!window) {
			std::cout << "Error creating window: " << SDL_GetError() << std::endl;
			return false;
		}

		render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!render)
		{
			std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
			return false;
		}
		SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_BLEND);
		windowRect = SDL_Rect{ 0,0, WIDTH, HEIGHT };
		previousTicks = SDL_GetPerformanceCounter();

		// Hide cursor in console
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(out, &cursorInfo);
		cursorInfo.bVisible = false;
		SetConsoleCursorInfo(out, &cursorInfo);

		// lock size of console
		HWND consoleWindow = GetConsoleWindow();
		SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

		if (sizeof(textureArr) > 0)
		{
			backgroundTexture = IMG_LoadTexture(render, textureArr[0]);
			SDL_SetTextureBlendMode(backgroundTexture, SDL_BlendMode::SDL_BLENDMODE_BLEND);
		}

		for (size_t i = 1; i < textureCount; i++)
		{
			SDL_Texture* texture = IMG_LoadTexture(render, textureArr[i]);
			textures.push_back(texture);
			SDL_SetTextureAlphaMod(texture, 200);
		}

		// makes rand() actually random
		srand(time(NULL));
		return true;
	}

#pragma endregion INITIALIZATION

#pragma region

	SDL_Renderer* getRenderer()
	{
		return render;
	}

	// draws multiple circles, mainly used for debugging colliders
	void drawCircles(float x, float y, float radius)
	{
		int resolution = 48;
		float step = (2 * PI) / resolution;
		float radiusMultiplier = 1.f;
		UINT8 r, g, b, a;
		SDL_GetRenderDrawColor(render, &r, &g, &b, &a);
		UINT8 currentRed = r, currentGreen = g, currentBlue = b;

		for (int j = 0; j < 3; j++, radiusMultiplier -= 0.33f)
		{
			float currentRadius = radius * radiusMultiplier;
			for (int i = 0; i < resolution; ++i)
			{
				float angle = step * i;
				float x1 = cos(angle);
				float y1 = sin(angle);

				float next_angle = step * (i + 1);
				float x2 = cos(next_angle);
				float y2 = sin(next_angle);

				SDL_RenderDrawLine(
					render,
					x1 * currentRadius + x,
					y1 * currentRadius + y,
					x2 * currentRadius + x,
					y2 * currentRadius + y
				);
			}
			currentRed /= 1.5;
			currentGreen /= 1.5;
			currentBlue /= 1.5;
			SDL_SetRenderDrawColor(render, currentRed, currentGreen, currentBlue, a);
		}

		SDL_SetRenderDrawColor(render, r, g, b, a); // reset to start color
	}

	// draws a simple hexagon with a single render call
	void drawHexagon(float x, float y, float radius)
	{
		// A simplified version of the circle draw
		// where a single render call is used
		float step = (2 * PI) / 6;
		SDL_FPoint points[7];
		float nextAngle;
		for (int i = 0; i < 7; ++i)
		{
			float angle = step * i;
			float x1 = cos(angle);
			float y1 = sin(angle);

			points[i] = SDL_FPoint(x1 * radius + x, y1 * radius + y);
		}
		// to minimize start/end overlap
		Vector2 dir = unitDirection(points[5].x, points[5].y, points[6].x, points[6].y);
		points[6].x -= dir.x * 1.2f;
		points[6].y -= dir.y * 1.2f;
		SDL_RenderDrawLinesF(render, points, 7);
	}

	// draws a hexagon with adjustable offset of an additional line
	void drawHexagon(float x, float y, float radius, float offset)
	{
		// A simplified version of the circle draw
		// where a single render call is used
		float step = (2 * PI) / 6;
		SDL_FPoint points[7];
		SDL_FPoint points2[7];
		float nextAngle;
		for (int i = 0; i < 7; ++i)
		{
			float angle = step * i;
			float x1 = cos(angle);
			float y1 = sin(angle);

			points[i] = SDL_FPoint(x1 * radius + x, y1 * radius + y);
			points2[i] = SDL_FPoint(x1 * (radius + 0.5f) + x, y1 * (radius + 0.5f) + y); //todo: make thickness a variable
		}
		// to minimize start/end overlap
		Vector2 dir = unitDirection(points[5].x, points[5].y, points[6].x, points[6].y);
		points[6].x -= dir.x * 1.2f;
		points[6].y -= dir.y * 1.2f;
		points2[6].x -= dir.x * 1.2f;
		points2[6].y -= dir.y * 1.2f;
		SDL_RenderDrawLinesF(render, points, 7);
		SDL_RenderDrawLinesF(render, points2, 7);
	}

	// draws objects in process of being deleted
	void drawObjectsFadingOut()
	{
		float fadeOutTime = 1.5f;
		for (int i = objectsToDelete.size() - 1; i >= 0; i--)
		{
			objectsToDelete[i].elapsedFadeOutTime += deltaTime;
			if (objectsToDelete[i].elapsedFadeOutTime > fadeOutTime)
			{
				objectsToDelete.erase(objectsToDelete.begin() + i);
				continue;
			}
			float progress = objectsToDelete[i].elapsedFadeOutTime / fadeOutTime;
			float sizeMultiplier = 1 - pow(progress, 2);

			Position pos = objectsToDelete[i].position;
			SDL_Rect destination{
				pos.x - (pos.radius * sizeMultiplier) - (objectsToDelete[i].appearance.scaleOffset / 2),
				pos.y - (pos.radius * sizeMultiplier) - (objectsToDelete[i].appearance.scaleOffset / 2),
				pos.radius * 2 * sizeMultiplier + objectsToDelete[i].appearance.scaleOffset,
				pos.radius * 2 * sizeMultiplier + objectsToDelete[i].appearance.scaleOffset
			};

			UINT8 prevAlpha;
			SDL_GetTextureAlphaMod(objectsToDelete[i].appearance.texture, &prevAlpha);

			int a = std::lerp(255, 0, progress);
			SDL_SetTextureAlphaMod(objectsToDelete[i].appearance.texture, a);
			SDL_SetTextureColorMod(objectsToDelete[i].appearance.texture, 0, 225, 255);
			SDL_RenderCopyEx(render, objectsToDelete[i].appearance.texture, NULL, &destination, objectsToDelete[i].angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
			SDL_SetTextureAlphaMod(objectsToDelete[i].appearance.texture, prevAlpha);

			int fadeCount = 12;
			
			for (int j = 0; j < fadeCount; j++)
			{
				float multiplier = 1 - ((float)j / (float)fadeCount);
				multiplier = pow(multiplier, 2);
				SDL_SetRenderDrawColor(render, 0, 225 * multiplier, 255 * multiplier, 150 * multiplier * (1 - progress));
				drawHexagon(pos.x, pos.y, (pos.radius + j + 7) * sizeMultiplier, 0.5f);
				drawHexagon(pos.x, pos.y, (pos.radius - j * 0.5f) * sizeMultiplier);
			
			}
			SDL_SetRenderDrawColor(render, 0, 225, 255, 255 * (1 - progress));
			drawHexagon(pos.x, pos.y, (pos.radius + 1) * sizeMultiplier, 1.f);
			drawHexagon(pos.x, pos.y, (pos.radius + 7) * sizeMultiplier);
		}

	}

	// draws all movable objects available in the game
	void drawObjects()
	{
		for (int i = 0; i < objectCount; i++)
		{
			SDL_Rect destination{
				positions[i].x - positions[i].radius - (appearances[i].scaleOffset / 2),
				positions[i].y - positions[i].radius - (appearances[i].scaleOffset / 2),
				positions[i].radius * 2 + appearances[i].scaleOffset,
				positions[i].radius * 2 + appearances[i].scaleOffset
			};
			if (appearances[i].tint.a > 0)
				SDL_SetTextureColorMod(appearances[i].texture, appearances[i].tint.r, appearances[i].tint.g, appearances[i].tint.b);
			SDL_RenderCopyEx(render, appearances[i].texture, NULL, &destination, rotations[i].angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
		}

		if (objectsToDelete.size() > 0)
			drawObjectsFadingOut();

	}

	// draws the "protection laser" which symbolizes the player's health
	void drawHealthLine(MusicData* music)
	{
		// this is just a dirty solution for this deadline 
		// if I had more time I'd simply create a spritesheet of a beam

		Laser line(0, HEALTH_LINE_HEIGHT, WIDTH, HEALTH_LINE_HEIGHT, 0, SDL_Color(0, 225, 255, 255));
		
		int thickness = Player::remainingHealth;
		Vector2 direction = unitDirection(line.x1, line.y1, line.x2, line.y2);
		Vector2 offsetVector = direction.perpendicularVectorClockwise();

		float easeInOutBeat = -(cos(PI * music->pulseMultiplier) - 1) / 2;

		float beatMultiplier = (easeInOutBeat / 3) + 0.6667f - (music->quarterNoteProgress / 10); // make the multiplier go from 1 -> 0.66
		// draw base line
		for (int i = 1; i < thickness; i++)
		{
			float fade = (float)i / thickness;
			fade = sin((fade * PI) / 2);

			SDL_SetRenderDrawColor(render, line.color.r * fade, line.color.g * fade, line.color.b * fade, 255 * fade * beatMultiplier);
			SDL_RenderDrawLineF(render, line.x1 + offsetVector.x * i, line.y1 + offsetVector.y * i, line.x2 + offsetVector.x * i, line.y2 + offsetVector.y * i);
			SDL_RenderDrawLineF(render, line.x1 - offsetVector.x * i, line.y1 - offsetVector.y * i, line.x2 - offsetVector.x * i, line.y2 - offsetVector.y * i);
		}

		// increase size of fadeout with the beat, the numbers are experimental
		thickness *= (pow(beatMultiplier / 4, 3)) + 0.9f;
		// draw fade out
		for (int i = 0; i < thickness * 3; i++)
		{
			float fade = 1 - ((float)i / (thickness * 3));
			fade *= 0.66f;
			float o = thickness + i;

			SDL_SetRenderDrawColor(render, line.color.r * fade, line.color.g * fade, line.color.b * fade, 255 * fade * beatMultiplier);
			SDL_RenderDrawLineF(render, line.x1 + offsetVector.x * o, line.y1 + offsetVector.y * o, line.x2 + offsetVector.x * o, line.y2 + offsetVector.y * o);
			SDL_RenderDrawLineF(render, line.x1 - offsetVector.x * o, line.y1 - offsetVector.y * o, line.x2 - offsetVector.x * o, line.y2 - offsetVector.y * o);
		}
	}

	void drawDoubleLines(Position startPosition, Position endPosition, Vector2 offsetDirection, float startOffset, float endOffset)
	{
		// renders two lines from startPosition to endPosition with mirrored offset
		SDL_RenderDrawLineF(render, startPosition.x + offsetDirection.x * startOffset, startPosition.y + offsetDirection.y * startOffset,
			endPosition.x + offsetDirection.x * endOffset, endPosition.y + offsetDirection.y * endOffset);

		SDL_RenderDrawLineF(render, startPosition.x - offsetDirection.x * startOffset, startPosition.y - offsetDirection.y * startOffset,
			endPosition.x - offsetDirection.x * endOffset, endPosition.y - offsetDirection.y * endOffset);
	}

	void drawLasers(SDL_Color color)
	{
		if (lineList.empty()) return;

		float lifeTime = 0.7f;
		for (int i = lineList.size() - 1; i >= 0; i--)
		{
			color = lineList[i].color;
			lineList[i].elapsedTime += deltaTime;
			if (lineList[i].elapsedTime > lifeTime)
			{
				lineList.erase(lineList.begin() + i);
				continue;
			}

			Vector2 playerToEndDirection = unitDirection(positions[playerIndex].x, positions[playerIndex].y, lineList[i].x2, lineList[i].y2);
			Vector2 playerEdgePosition = Vector2(positions[playerIndex].x, positions[playerIndex].y) + playerToEndDirection * positions[playerIndex].radius;

			// the line's lifetime eased with different easing functions
			float progress1 = 1 - cos(((lineList[i].elapsedTime / lifeTime) * 3.1415) / 2); // ease in out sine
			float progress2 = 1 - pow(1 - progress1, 4);										// ease out quart on already eased sine
			float progress3 = 1 - pow(1 - lineList[i].elapsedTime / lifeTime, 5);			// ease out quint
			float progress4 = 1 - pow(1 - progress1, 3);									// ease out cubic on already eased sine

			color.a *= 1 - progress2; // fadeout opacity over lifetime

			Position endPosition(lineList[i].x2, lineList[i].y2);
			Position startPosition(std::lerp(lineList[i].x1, endPosition.x, progress1), std::lerp(lineList[i].y1, endPosition.y, progress1));

			Vector2 direction = unitDirection(Vector2(lineList[i].x1, lineList[i].y1), Vector2(lineList[i].x2, lineList[i].y2));
			Vector2 offsetVector = direction.perpendicularVectorClockwise();


			// only render certain lines during specific parts of the line's lifetime
			if (progress2 < 0.7f)
			{
				SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
				drawDoubleLines(startPosition, endPosition, offsetVector, (1 - progress3), 1);

				if (progress2 < 0.6f)
				{
					SDL_SetRenderDrawColor(render, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f, color.a);
					drawDoubleLines(startPosition, endPosition, offsetVector, (2 * (1 - progress3)), 2);

					if (progress2 < 0.5f)
					{
						SDL_SetRenderDrawColor(render, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, color.a);
						drawDoubleLines(startPosition, endPosition, offsetVector, 3 * (1 - progress3), 3);

						// draw a circle at the end of the line to make the edge smoother
						SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a * 0.25f);
						drawCircles(endPosition.x, endPosition.y, 3);

						if (progress2 < 0.4f)
						{
							//draw outline
							SDL_SetRenderDrawColor(render, color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, color.a);
							drawDoubleLines(startPosition, endPosition, offsetVector, 4 * (1 - progress3), 4);
							if (progress2 < 0.2f)
							{
								// draw "burst" lines, offset to look like a cone
								for (int j = 0; j < 6; j++)
								{
									for (int k = 0; k < 3; k++)
									{
										float alphaMultiplier = 1 - j * 0.15f;
										SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a * (1 - lineList[i].elapsedTime / 0.2f) * alphaMultiplier);
										float length = (float)(rand() % 30 + 10) * (0.5f + (lineList[i].elapsedTime / 0.2f));
										Position startPos(playerEdgePosition.x - direction.x * 20, playerEdgePosition.y - direction.y * 20);
										Position endPosition(lineList[i].x1 + direction.x * length, lineList[i].y1 + direction.y * length);
										drawDoubleLines(startPos, endPosition, offsetVector, 0, (3 + j * 4));
									}

								}
								if (progress2 < 0.1)
								{
									// draw darker lines outside the center line
									SDL_SetRenderDrawColor(render, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, color.a * (1 - lineList[i].elapsedTime / 0.1f));
									for (int j = 1; j < 8; j += 2)
									{
										drawDoubleLines(Position(lineList[i].x1, lineList[i].y1), endPosition, offsetVector, j, 5);
									}

								}
							}
						}

					}
				}
			}

			//draw static transparent trail
			SDL_SetRenderDrawColor(render, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 40 * (1 - progress2));
			SDL_RenderDrawLineF(render, lineList[i].x1, lineList[i].y1, endPosition.x, endPosition.y);
			SDL_SetRenderDrawColor(render, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, 20 * (1 - progress2));
			drawDoubleLines(Position(lineList[i].x1, lineList[i].y1), endPosition, offsetVector, 1, 1);

			if (progress2 < 0.8f && progress2 > 0.05f)
			{
				//draw moving transparent trail
				SDL_SetRenderDrawColor(render, color.r, color.g, color.b, 255 * (1 - progress2 / 0.8f));
				SDL_RenderDrawLineF(render, startPosition.x - direction.x * 100.f, startPosition.y - direction.y * 100.f, endPosition.x, endPosition.y);
			}

			//draw highlight
			SDL_SetRenderDrawColor(render, 255, 255, 255, color.a);
			SDL_RenderDrawLineF(render, startPosition.x, startPosition.y, endPosition.x, endPosition.y);
		}
	}

	void drawSimpleCircle(float x, float y, float radius, int resolution = 6)
	{
		float step = (2 * PI) / resolution;
		for (int i = 0; i < resolution; ++i)
		{
			float angle = step * i;
			float x1 = cos(angle);
			float y1 = sin(angle);

			float nextAngle = step * (i + 1);
			float x2 = cos(nextAngle);
			float y2 = sin(nextAngle);

			SDL_RenderDrawLine(
				render, x1 * radius + x, y1 * radius + y, x2 * radius + x, y2 * radius + y
			);


		}
	}

	void drawBeatCircles(MusicData* music)
	{
		//MusicData* music = getMusicData();
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		float synchDuration = music->quarterNoteLength / 4;

		float easedWholeNoteProgress = pow(music->wholeNoteProgress, 4);
		float easedHalfNoteProgress = pow(music->halfNoteProgress, 4);
		float easedQuarterNoteProgress = pow(music->quarterNoteProgress, 4);

		SDL_Color wholeNoteColor{
			music->timeSinceLastWholeNote < synchDuration || music->wholeNoteProgress > 0.98f ? 115 : std::lerp(255, 115, easedWholeNoteProgress), 0,
			music->timeSinceLastWholeNote < synchDuration || music->wholeNoteProgress > 0.98f ? 250 : 230 * music->wholeNoteProgress,
			music->timeSinceLastWholeNote < synchDuration || music->wholeNoteProgress > 0.98f ? 255 : 200 * music->wholeNoteProgress
		};

		SDL_Color quarterNoteColor{
			music->timeSinceLastQuarterNote < synchDuration || music->quarterNoteProgress > 0.98f ? 115 : std::lerp(255, 115, easedQuarterNoteProgress), 0,
			music->timeSinceLastQuarterNote < synchDuration || music->quarterNoteProgress > 0.98f ? 230 : 230 * music->quarterNoteProgress,
			music->timeSinceLastQuarterNote < synchDuration || music->quarterNoteProgress > 0.98f ? 190 : 100 * music->quarterNoteProgress
		};

		SDL_Color halfNoteColor{
			music->timeSinceLastHalfNote < synchDuration || music->halfNoteProgress > 0.98f ? 115 : std::lerp(255, 115, easedHalfNoteProgress), 0,
			music->timeSinceLastHalfNote < synchDuration || music->halfNoteProgress > 0.98f ? 250 : 230 * music->halfNoteProgress,
			music->timeSinceLastHalfNote < synchDuration || music->halfNoteProgress > 0.98f ? 255 : 200 * music->halfNoteProgress
		};


		for (int i = 0; i < objectCount; i++)
		{
			if (i == playerIndex) continue;

			if (tags[i] == Tag::Unsplittable) // draw red pentagon if the object is disabled
			{
				SDL_SetRenderDrawColor(render, 150, 0, 0, 175);
				drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 1, 1.f);
				drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 7);
				continue;
			}
			int resolution = 6;
			float radius = positions[i].radius * 2.5f;
			float distanceToMouse = distance(mouseX, mouseY, positions[i].x, positions[i].y);
			float maxDistance = 2000;
			float distanceMultiplier = 1 - std::clamp((distanceToMouse - (positions[i].radius * 1.5f) - 50) / maxDistance, 0.f, 1.f);
			distanceMultiplier = pow(distanceMultiplier, 3);

			if (positions[i].radius > MIN_SIZE_WHOLENOTE)
			{
				if (music->timeSinceLastWholeNote < synchDuration)
				{
					radius = positions[i].radius;
				}
				else
				{
					radius = std::lerp(positions[i].radius * 2.5f, positions[i].radius, music->wholeNoteProgress - 0.02f);
					if (music->wholeNoteProgress < 0.5f)
					{
						float radius2 = std::lerp(radius, positions[i].radius, music->halfNoteProgress);
						radius2 = std::clamp(radius2, positions[i].radius, radius);
						SDL_SetRenderDrawColor(render, wholeNoteColor.r, 0, wholeNoteColor.b, wholeNoteColor.a * 0.66f * distanceMultiplier);
						drawHexagon(positions[i].x, positions[i].y, radius2);
					}
					float halfNoteMultiplier = 1 + (1 - music->halfNoteProgress);
					SDL_SetRenderDrawColor(render, 100, 0, 200, 100 * distanceMultiplier * halfNoteMultiplier);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 1);
				}
				SDL_SetRenderDrawColor(render, wholeNoteColor.r, 0, wholeNoteColor.b, wholeNoteColor.a * distanceMultiplier);
			}
			else if (positions[i].radius > MIN_SIZE_HALFNOTE)
			{
				if (music->timeSinceLastHalfNote < synchDuration)
				{
					radius = positions[i].radius;
				}
				else
				{

					radius = std::lerp(positions[i].radius * 2.5f, positions[i].radius, music->halfNoteProgress - 0.02f);
					if (music->halfNoteProgress < 0.5f)
					{
						float radius2 = std::lerp(radius, positions[i].radius, music->quarterNoteProgress);
						SDL_SetRenderDrawColor(render, halfNoteColor.r, halfNoteColor.g, halfNoteColor.b, halfNoteColor.a * 0.66f * distanceMultiplier);
						drawHexagon(positions[i].x, positions[i].y, radius2);
					}
					float quarterNoteMultiplier = 1 + (1 - music->quarterNoteProgress);
					SDL_SetRenderDrawColor(render, 100, 0, 200, 100 * distanceMultiplier * quarterNoteMultiplier);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 1);
				}

				SDL_SetRenderDrawColor(render, halfNoteColor.r, 0, halfNoteColor.b, halfNoteColor.a * distanceMultiplier);
			}
			else
			{
				if (music->timeSinceLastQuarterNote < synchDuration)
				{
					radius = positions[i].radius;
				}
				else
				{
					radius = std::lerp(positions[i].radius * 2.5f, positions[i].radius, music->quarterNoteProgress);
					SDL_SetRenderDrawColor(render, 115, 0, 230, 100 * distanceMultiplier);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius);
				}
				SDL_SetRenderDrawColor(render, quarterNoteColor.r, 0, quarterNoteColor.b, quarterNoteColor.a * distanceMultiplier);
			}

			drawHexagon(positions[i].x, positions[i].y, radius, 1.f);
			UINT8 r, g, b, a;
			SDL_GetRenderDrawColor(render, &r, &g, &b, &a);
			SDL_SetRenderDrawColor(render, r, g, b, a * 0.66f);
			drawHexagon(positions[i].x, positions[i].y, radius + 5, 0.5f);
		}

		float radius = music->timeSinceLastQuarterNote < 0.1f ? 12 : 25 - (13 * music->quarterNoteProgress);
		float alpha = music->timeSinceLastQuarterNote < 0.1f ? 120 : quarterNoteColor.a * 0.8f;
		float green = music->timeSinceLastQuarterNote < 0.1f ? 200 : 50;
		float blue = music->timeSinceLastQuarterNote < 0.1f ? 200 : 100;
		SDL_SetRenderDrawColor(render, 0, green, blue, alpha);
		drawSimpleCircle(mouseX + 0.5f, mouseY + 0.5f, 12, 48);
		drawSimpleCircle(mouseX + 0.5f, mouseY + 0.5f, 13.2f, 48);
	}

	void drawColliders()
	{
		SDL_SetRenderDrawColor(render, 255, 255, 0, 255);
		for (int i = 0; i < objectCount; i++)
		{
			drawCircles(positions[i].x, positions[i].y, positions[i].radius);
		}
	}

	void drawBackground()
	{
		SDL_Rect destination{ 0, 0, WIDTH, HEIGHT };
		int result = SDL_RenderCopy(render, backgroundTexture, NULL, &destination);
		if (result != 0)
		{
			std::cout << "Fail to render background\n";
		}
	}

	void renderPresent()
	{
		SDL_RenderPresent(render);
	}

	void renderClear()
	{
		SDL_RenderClear(render);
	}

	void unloadTextures()
	{
		for (int i = textures.size(); i >= 0; i--)
		{
			SDL_DestroyTexture(textures[i]);
			textures.pop_back();
		}
		SDL_DestroyTexture(appearances[playerIndex].texture);
		SDL_DestroyTexture(backgroundTexture);
	}

#pragma endregion RENDERING

#pragma region

	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, SDL_Texture* texture, Tag tag)
	{
		// randomize texture if a texture wasn't included
		if (texture == nullptr)
			texture = textures[rand() % (textures.size())];

		// randomize color tint for each instance
		float r = rand() % 25 + 200, g = rand() % 20 + 220, b = rand() % 15 + 240;
		Appearance appearance(texture, scaleOffset, SDL_Color(r, g, b, 255));


		positions.push_back(position);
		velocities.push_back(velocity);
		rotations.push_back(rotation);
		appearances.push_back(appearance);
		tags.push_back(tag);
		objectCount++;
	}

	void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, const char* texturePath, Tag tag)
	{
		SDL_Texture* texture = IMG_LoadTexture(render, texturePath);
		Appearance appearance(texture, scaleOffset);

		int result = SDL_QueryTexture(texture, NULL, NULL, NULL, NULL);
		if (result != 0)
		{
			std::cout << "Failed to load image at: " << texturePath << std::endl;
			return;
		}
		positions.push_back(position);
		velocities.push_back(velocity);
		rotations.push_back(rotation);
		appearances.push_back(appearance);
		tags.push_back(tag);
		objectCount++;
	}

	void removeObject(int index)
	{
		if (index < 0 || index > objectCount - 1 || index == playerIndex) return;

		// not needed since they all share the same textures
		//SDL_DestroyTexture(textures[index].texture); 

		positions.erase(positions.begin() + index);
		velocities.erase(velocities.begin() + index);
		rotations.erase(rotations.begin() + index);
		appearances.erase(appearances.begin() + index);
		tags.erase(tags.begin() + index);
		objectCount--;

		if (index < playerIndex) playerIndex--;
	}

	void splitObject(int index, Vector2 splitDirection)
	{
		int splits = 1;
		float previousRadius = positions[index].radius;
		if (previousRadius > MIN_SIZE_WHOLENOTE)
		{
			positions[index].radius *= 0.3f;
			splits = 6;
		}
		else if (previousRadius > MIN_SIZE_TRIPPLESPLIT)
		{
			positions[index].radius *= 0.4f;
			splits = 2;
		}
		else
			positions[index].radius *= 0.5f;
		if (positions[index].radius < MIN_RADIUS)
		{
			removeObject(index);
			return;
		}
		float force = (rand() % 50 + 100) * 0.2f;
		Vector2 velocityDirection = splitDirection.perpendicularVectorClockwise();
		velocities[index] = Velocity(velocityDirection.x * force, velocityDirection.y * force);
		positions[index].x += velocityDirection.x * previousRadius;
		positions[index].y += velocityDirection.y * previousRadius;
		rotations[index].angle = (float)(rand() % 360);

		for (int i = 0; i < splits; i++)
		{
			float angleToRotate = 360 / (splits + 1);
			rotateVector(velocityDirection.x, velocityDirection.y, angleToRotate);

			force = (rand() % 50 + 100) * 0.2f;
			createObject(positions[index], rotations[index], velocities[index], 0, appearances[index].texture);
			velocities[objectCount - 1] = Velocity(velocityDirection.x * force, velocityDirection.y * force);
			positions[objectCount - 1].x += velocityDirection.x * previousRadius;
			positions[objectCount - 1].y += velocityDirection.y * previousRadius;
			rotations[objectCount - 1].angle = (float)(rand() % 360);
		}

	}

	Position getPlayerPos()
	{
		return positions[playerIndex];
	}

	int getObjectCount()
	{
		return objectCount;
	}
	
	void addLaser(Laser line, MusicData* music)
	{
		line.color = music->quarterNoteActive ? SDL_Color(0, 200, 255, 255) : SDL_Color(255, 0, 0, 255);

		for (int i = 0; i < objectCount; i++)
		{
			if (i == playerIndex) continue;
			Circle circ = Circle(positions[i].x, positions[i].y, positions[i].radius);
			if (pointCircleIntersect(line.x2, line.y2, circ))
			{
				bool success = false;
				if (positions[i].radius > MIN_SIZE_WHOLENOTE && music->wholeNoteActive && appearances[i].tint.b > 0)
				{
					success = true;
				}
				else if (positions[i].radius > MIN_SIZE_HALFNOTE && music->halfNoteActive && appearances[i].tint.b > 0)
				{
					success = true;
				}
				else if (positions[i].radius < MIN_SIZE_HALFNOTE && music->quarterNoteActive && appearances[i].tint.b > 0)
				{
					success = true;
				}

				if (success)
				{
					Vector2 splitDirection = Vector2(line.x1, line.y1) - Vector2(line.x2, line.y2);
					splitDirection.normalize();
					positions[i].x = line.x2;
					positions[i].y = line.y2;
					line.color = SDL_Color(0, 200, 255, 255);
					splitObject(i, splitDirection);
				}
				else
				{
					line.color = SDL_Color(255, 0, 0, 255);
					if (appearances[i].tint.b > 0)
					{
						appearances[i].tint = SDL_Color(150, 0, 0, 255);
						tags[i] = Tag::Unsplittable;
					}
				}
				break;
			}
		}
		lineList.push_back(line);
	}

	void sortObjects()
	{
		if (objectCount <= 1) return;
		for (int i = 1; i < objectCount; i++)
		{
			int j = i;
			while (j > 0 && positions[j].x - positions[j].radius < positions[j - 1].x - positions[j - 1].radius)
			{
				std::swap(positions[j], positions[j - 1]);
				std::swap(velocities[j], velocities[j - 1]);
				std::swap(appearances[j], appearances[j - 1]);
				std::swap(rotations[j], rotations[j - 1]);
				std::swap(tags[j], tags[j - 1]);

				if (j == playerIndex) playerIndex--;
				else if (j - 1 == playerIndex) playerIndex++;

				j--;
			}
		}
	}

	void clearObjects()
	{
		for (int i = objectCount - 1; i >= 0; i--)
		{
			removeObject(i);
		}
	}

	void moveObjects()
	{
		float depenetrateX, depenetrateY;
		int count = 0;
		for (int i = 0; i < objectCount; i++)
		{
			positions[i].x += velocities[i].xVelocity * deltaTime;
			positions[i].y += velocities[i].yVelocity * deltaTime;

			for (int other = i + 1; other < objectCount; other++) // check collisions to the right
			{
				count++;
				// stop checking if the 'other' collider's left x coordinate is bigger than the original's right x coordinate
				if (positions[other].x - positions[other].radius > positions[i].x + positions[i].radius)
					break;
				if (circleIntersect(positions[i].x, positions[i].y, positions[i].radius,
					positions[other].x, positions[other].y, positions[other].radius, depenetrateX, depenetrateY))
				{
					// unsplittable objects can't be affected by other objects
					if (tags[i] == Tag::Unsplittable)
					{
						velocities[other] = velocities[i];
						positions[other].x += depenetrateX;
						positions[other].y += depenetrateY;
					}
					else if (tags[other] == Tag::Unsplittable)
					{
						velocities[i] = velocities[other];
						positions[i].x -= depenetrateX;
						positions[i].y -= depenetrateY;
					}
					// keep the velocity of the bigger collider
					else if (positions[i].radius > positions[other].radius)
					{
						velocities[other] = velocities[i];
						positions[other].x += depenetrateX;
						positions[other].y += depenetrateY;
					}
					else
					{
						velocities[i] = velocities[other];
						positions[i].x -= depenetrateX;
						positions[i].y -= depenetrateY;
					}
				}
			}
			if (tags[i] == Tag::Unsplittable)
				velocities[i].yVelocity += deltaTime * std::lerp(200.f, 5.f, velocities[i].yVelocity / 500);
			else
				velocities[i].yVelocity += deltaTime * std::lerp(25.f, 5.f, velocities[i].yVelocity / 50);
		}
		collisionChecksPerFrame = count;
	}

	// checks for object collision with health line/laser
	// and also out of bounds positions
	// returns player damage
	int checkForObjectDestruction(MusicManager& musicManager)
	{
		int playerDamage = 0;
		for (int i = 0; i < objectCount; i++)
		{
			if (positions[i].y + positions[i].radius > HEALTH_LINE_HEIGHT && i != playerIndex)
			{

				objectsToDelete.push_back(ObjectPendingDeletion(positions[i], appearances[i], 0.f, rotations[i].angle));
				removeObject(i);
				playerDamage = 2;
				continue;
			}
			if (positions[i].x + positions[i].radius <  0 || positions[i].x - positions[i].radius > WIDTH ||
				positions[i].y + positions[i].radius < -100 || positions[i].y - positions[i].radius > HEIGHT)
			{
				removeObject(i);
			}
		}
		return playerDamage;
	}

	void rotateObjects()
	{
		for (int i = 0; i < objectCount; i++)
		{
			if (rotations[i].force == 0) continue;
			rotations[i].angle += rotations[i].force * deltaTime;

			if (rotations[i].angle > 360)
				rotations[i].angle = 0;
			if (rotations[i].angle < 0)
				rotations[i].angle = 360;
		}
	}

	void updatePlayerVelocity(float x, float y)
	{
		velocities[playerIndex].xVelocity = x;
		velocities[playerIndex].yVelocity = y;
	}

#pragma endregion OBJECT_MANAGEMENT

#pragma region

	float updateTicks()
	{
		Uint64 currentTicks = SDL_GetPerformanceCounter();
		Uint64 deltaTicks = currentTicks - previousTicks;
		deltaTime = (double)deltaTicks / SDL_GetPerformanceFrequency();
		previousTicks = currentTicks;
		elapsedTime += deltaTime;
		framerate = std::round(1 / deltaTime);

		return deltaTime;
	}

	void printTimeStats()
	{
		static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD coord = { (SHORT)0, (SHORT)0 };
		SetConsoleCursorPosition(hOut, coord);

		std::cout.flush();
		std::cout << " --------------------- \n";
		std::cout << "| Elapsed time: " << std::setprecision(1) << std::fixed << elapsedTime << "\n| \n";
		std::cout << "| FPS (capped): " << framerate << "    \n| \n";
		std::cout << "| Object count: " << objectCount << "  \n| \n";
		std::cout << "| Checks / object: " << collisionChecksPerFrame / objectCount << " \n --------------------- \n";



		for (int i = 0; i < 7; i++)
		{
			coord = { (SHORT)22, (SHORT)(1 + i) };
			SetConsoleCursorPosition(hOut, coord);
			std::cout << "| ";
		}
		std::cout << "\n\n\n";
	}

	void delayNextFrame()
	{
		// Delays the current thread to keep the fps around 120 

		Uint64 currentTicks = SDL_GetPerformanceCounter();
		deltaTime = (double)(currentTicks - previousTicks) / SDL_GetPerformanceFrequency();
		int delay = (0.00833 - deltaTime) * 1000;
		if (delay > 1)
			SDL_Delay(delay);

	}

#pragma endregion DELTATIME_RELATED

#pragma region

	bool getKeyDown(int index)
	{
		return keys[index];
	}

	void updateKey(int index, bool value)
	{
		keys[index] = value;
		//std::cout << index << " was set to " << value << std::endl;
	}

	void resetKeys()
	{
		for (bool& key : keys)
		{
			key = false;
		}
	}

#pragma endregion KEY_MANAGEMENT

	void quit()
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
}