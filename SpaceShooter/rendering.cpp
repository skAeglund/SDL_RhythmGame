#include "rendering.h"
#include <algorithm>
#include <Windows.h>
#include "easingFunctions.h"
#include "SDL.h"
#include "engine.h"
#include "player.h"
#include "vector2.h"

using namespace Vector2D;

namespace Rendering
{
	SDL_Renderer* render;

	void setRenderer(SDL_Renderer* renderer)
	{
		render = renderer;
	}
	SDL_Renderer* getRenderer()
	{
		return render;
	}
	// Draws multiple circles, mainly used for debugging colliders
	void drawCircles(float x, float y, float radius)
	{
		const int resolution = 48;
		const float step = (2 * PI) / resolution;
		float radiusMultiplier = 1.f;
		UINT8 r, g, b, a;
		SDL_GetRenderDrawColor(render, &r, &g, &b, &a);
		UINT8 currentRed = r, currentGreen = g, currentBlue = b;

		for (int j = 0; j < 3; j++, radiusMultiplier -= 0.33f)
		{
			const float currentRadius = radius * radiusMultiplier;
			for (int i = 0; i < resolution; ++i)
			{
				const float angle = step * i;
				const float x1 = cos(angle);
				const float y1 = sin(angle);

				const float next_angle = step * (i + 1);
				const float x2 = cos(next_angle);
				const float y2 = sin(next_angle);

				SDL_RenderDrawLineF(
					render,
					x1 * currentRadius + x,
					y1 * currentRadius + y,
					x2 * currentRadius + x,
					y2 * currentRadius + y
				);
			}
			currentRed = static_cast<UINT8>(currentRed / 1.5);
			currentGreen = static_cast<UINT8>(currentGreen / 1.5);
			currentBlue = static_cast<UINT8>(currentBlue / 1.5);
			SDL_SetRenderDrawColor(render, currentRed, currentGreen, currentBlue, a);
		}

		SDL_SetRenderDrawColor(render, r, g, b, a); // reset to start color
	}
	// Overloading with color
	void drawCircles(Color color, float x, float y, float radius)
	{
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
		drawCircles(x, y, radius);
	}
	// Draws a simple hexagon with a single render call
	void drawHexagon(float x, float y, float radius)
	{
		const float step = (2 * PI) / 6;
		SDL_FPoint points[7];
		for (int i = 0; i < 7; ++i)
		{
			const float angle = step * i;
			const float x1 = cos(angle);
			const float y1 = sin(angle);

			points[i] = SDL_FPoint(x1 * radius + x, y1 * radius + y);
		}
		// to minimize start/end overlap
		const Vector2 dir = unitDirection(points[5].x, points[5].y, points[6].x, points[6].y);
		points[6].x -= dir.x * 1.2f;
		points[6].y -= dir.y * 1.2f;
		SDL_RenderDrawLinesF(render, points, 7);
	}

	// Draws a hexagon with adjustable offset of an additional line
	void drawHexagon(float x, float y, float radius, float offset)
	{
		const float step = (2 * PI) / 6;
		SDL_FPoint points[7];
		SDL_FPoint points2[7];
		for (int i = 0; i < 7; ++i)
		{
			const float angle = step * i;
			const float x1 = cos(angle);
			const float y1 = sin(angle);

			points[i] = SDL_FPoint(x1 * radius + x, y1 * radius + y);
			points2[i] = SDL_FPoint(x1 * (radius + offset) + x, y1 * (radius + offset) + y);
		}
		// to minimize start/end overlap
		const Vector2 dir = unitDirection(points[5].x, points[5].y, points[6].x, points[6].y);
		points[6].x -= dir.x * 1.2f;
		points[6].y -= dir.y * 1.2f;
		points2[6].x -= dir.x * 1.2f;
		points2[6].y -= dir.y * 1.2f;
		SDL_RenderDrawLinesF(render, points, 7);
		SDL_RenderDrawLinesF(render, points2, 7);
	}

	// Draws objects in process of being deleted - fading out over time
	void drawObjectsFadingOut(std::vector<ObjectPendingDeletion> objects)
	{
		for (size_t i =0; i < objects.size(); i++)
		{
			const float progress = objects[i].elapsedLifeTime / objects[i].totalLifeTime;
			const float sizeMultiplier = 1.f - powf(progress, 2);

			const Position pos = objects[i].position;
			SDL_Rect destination{
				static_cast<int>(pos.x - (pos.radius * sizeMultiplier) - (objects[i].appearance.scaleOffset / 2)),
				static_cast<int>(pos.y - (pos.radius * sizeMultiplier) - (objects[i].appearance.scaleOffset / 2)),
				static_cast<int>(pos.radius * 2 * sizeMultiplier + objects[i].appearance.scaleOffset),
				static_cast<int>(pos.radius * 2 * sizeMultiplier + objects[i].appearance.scaleOffset)
			};

			UINT8 prevAlpha;
			SDL_GetTextureAlphaMod(objects[i].appearance.texture, &prevAlpha);

			// red tint if destroyed by health line
			Color color = pos.y + pos.radius > HEALTH_LINE_HEIGHT - 5 ? Color(255, 0, 0, 150) : Color(0, 225, 255, 150);

			const auto alpha = static_cast<UINT8>(std::lerp(255, 0, progress));
			SDL_SetTextureAlphaMod(objects[i].appearance.texture, alpha);
			SDL_SetTextureColorMod(objects[i].appearance.texture, color.r, color.g, color.b);
			SDL_RenderCopyEx(render, objects[i].appearance.texture, NULL, &destination, objects[i].angle, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
			SDL_SetTextureAlphaMod(objects[i].appearance.texture, prevAlpha);

			constexpr int fadeCount = 12;
			

			for (size_t j = 0; j < fadeCount; j++)
			{
				float multiplier = 1 - (static_cast<float>(j) / static_cast<float>(fadeCount));
				multiplier = powf(multiplier, 2);
				const auto r = static_cast<UINT8>(color.r * multiplier);
				const auto g = static_cast<UINT8>(color.g * multiplier);
				const auto b = static_cast<UINT8>(color.b * multiplier);
				const auto a = static_cast<UINT8>(color.a * multiplier * (1 - progress));
				SDL_SetRenderDrawColor(render, r, g, b, a);
				drawHexagon(pos.x, pos.y, (pos.radius + j + 7.f) * sizeMultiplier, 0.5f);
				drawHexagon(pos.x, pos.y, (pos.radius - j * 0.5f) * sizeMultiplier);
			}
			SDL_SetRenderDrawColor(render, 0, 225, 255, 255 * (1 - progress));
			drawHexagon(pos.x, pos.y, (pos.radius + 1) * sizeMultiplier, 1.f);
			drawHexagon(pos.x, pos.y, (pos.radius + 7) * sizeMultiplier);
		}

	}

	// Draws all movable objects available in the game
	void drawObjects(std::vector<Position> positions, std::vector<Appearance> appearances, std::vector<Rotation> rotations)
	{
		for (int i = 0; i < positions.size(); i++)
		{
			SDL_Rect destination{
				static_cast<int>(positions[i].x - positions[i].radius - (appearances[i].scaleOffset / 2)),
				static_cast<int>(positions[i].y - positions[i].radius - (appearances[i].scaleOffset / 2)),
				static_cast<int>(positions[i].radius * 2 + appearances[i].scaleOffset),
				static_cast<int>(positions[i].radius * 2 + appearances[i].scaleOffset)
			};
			if (appearances[i].tint.a > 0)
				SDL_SetTextureColorMod(appearances[i].texture, appearances[i].tint.r, appearances[i].tint.g, appearances[i].tint.b);
			SDL_RenderCopyEx(render, appearances[i].texture, nullptr, &destination, rotations[i].angle, nullptr, SDL_FLIP_NONE);
		}
	}

	// Renders two lines from startPosition to endPosition with offset
	void drawDoubleLines(Position startPosition, Position endPosition, Vector2 offsetDirection, float startOffset, float endOffset)
	{
		SDL_RenderDrawLineF(render, startPosition.x + offsetDirection.x * startOffset, startPosition.y + offsetDirection.y * startOffset,
			endPosition.x + offsetDirection.x * endOffset, endPosition.y + offsetDirection.y * endOffset);

		SDL_RenderDrawLineF(render, startPosition.x - offsetDirection.x * startOffset, startPosition.y - offsetDirection.y * startOffset,
			endPosition.x - offsetDirection.x * endOffset, endPosition.y - offsetDirection.y * endOffset);
	}

	// Overloaded: Renders two lines from startPosition to endPosition with mirrored offset - with new color 
	void drawDoubleLines(Color color, Position startPosition, Position endPosition, Vector2 offsetDirection, float progress, float offset)
	{
		const float startOffset = progress > 0.f ? offset * (1 - progress) : 0.f;

		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
		drawDoubleLines(startPosition, endPosition, offsetDirection, startOffset, offset);
	}
	// Overloaded: Renders two lines with equal offset in each direction - with new color
	void drawDoubleLines(Color color, Position startPosition, Position endPosition, Vector2 offsetDirection, float offset)
	{
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
		drawDoubleLines(startPosition, endPosition, offsetDirection, offset, offset);
	}
	// Draws a single line with with a specific color
	void drawSingleLine(Color color, Position start, Position end)
	{
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
		SDL_RenderDrawLineF(render, start.x, start.y, end.x, end.y);
	}

	// Draws the "protection laser" which symbolizes the player's health
	void drawHealthLine(MusicData* music, int playerHealth)
	{
		// this is just a dirty solution for this deadline 
		// if I had more time I'd simply create a spritesheet of a beam

		constexpr Position start({ 0, HEALTH_LINE_HEIGHT });
		constexpr Position end({ WIDTH, HEALTH_LINE_HEIGHT });
		Color color(0, 225, 255, 255);
		int thickness = playerHealth;

		const Vector2 offsetDir = unitDirection(start.x, start.y, end.x, end.y).perpendicularVector();
		const float easeInOutBeat = -(cos(PI * music->pulseMultiplier) - 1) / 2;
		const float beatMultiplier = (easeInOutBeat / 3) + 0.6667f - (music->halfNoteProgress / 10); // makes the multiplier go from 1 -> 0.66

		// draw base line
		for (int i = 1; i < thickness; i++)
		{
			const float fade = sin(static_cast<float>(i) / static_cast<float>(thickness) * PI * 0.5f);

			drawDoubleLines(color.multiplied(fade, fade * beatMultiplier), start, end, offsetDir, i);
		}

		// increase size of fadeout with the beat, the numbers are experimental
		thickness *= (pow(beatMultiplier / 4, 3)) + 0.75f;
		// draw fade out
		for (int i = 0; i < thickness * 4; i++)
		{
			const float fade = (1 - Ease::Out(static_cast<float>(i) / static_cast<float>((thickness * 4)), 2)) * 0.8f;
			const auto offset = static_cast<float>(thickness + i);

			drawDoubleLines(color.multiplied(fade, fade * beatMultiplier ), start, end, offsetDir, offset);
		}
	}

	void drawLasers(std::vector<Laser> lasers, std::vector<Position> positions, Position playerPos)
	{
		if (lasers.empty()) return;


		for (size_t i = 0; i < lasers.size(); i++)
		{
			Color color = lasers[i].color;

			Vector2 playerToEndDirection = unitDirection(playerPos.x, playerPos.y, lasers[i].x2, lasers[i].y2);
			const Vector2 playerEdgePosition = Vector2(playerPos.x, playerPos.y) + playerToEndDirection * playerPos.radius;

			// the line's lifetime eased with different easing functions
			const float progress = lasers[i].elapsedLifeTime / lasers[i].totalLifeTime;
			const float progress1 = Ease::InOutSine(progress);
			const float progress2 = Ease::Out(progress1, 4);
			const float progress3 = Ease::Out(progress, 5);

			color.a *= 1 - progress2; // fadeout opacity over lifetime

			const Position basePosition = Position(lasers[i].x1, lasers[i].y1); // start position without offset
			const Position endPosition(lasers[i].x2, lasers[i].y2);
			const Position startPosition(std::lerp(lasers[i].x1, endPosition.x, progress1), std::lerp(lasers[i].y1, endPosition.y, progress1));

			Vector2 direction = unitDirection(Vector2(lasers[i].x1, lasers[i].y1), Vector2(lasers[i].x2, lasers[i].y2));
			const Vector2 offsetVector = direction.perpendicularVector();

			// decrease the amount of layers toward the end of the laser's lifetime
			const int layerCount =
				progress2 < 0.4f ? 4 :
				progress2 < 0.5f ? 3 :
				progress2 < 0.6f ? 2 :
				progress2 < 0.7f ? 1 : 0;

			// draw main layers
			for (int j = 0; j < layerCount; j++)
			{
				drawDoubleLines(color.multiplied(1 - (j*0.15f)), startPosition, endPosition, offsetVector, progress3, j + 1);
			}
			
			if(progress2 < 0.4f)
			{
				// draw circle at the end to make the edge more smooth
				drawCircles(color.multiplied(1.f, 0.25f), endPosition.x, endPosition.y, 3);
				if (progress2 < 0.2f)
				{
					// draw "burst" lines, offset to look like a cone
					for (int j = 0; j < 6; j++)
					{
						for (int k = 0; k < 3; k++)
						{
							const float alphaMultiplier = 1 - j * 0.15f;
							Color col = color.multiplied(1.f, (1 - lasers[i].elapsedLifeTime / 0.2f) * alphaMultiplier);

							const float length = static_cast<float>(rand() % 30 + 10) * (0.5f + (lasers[i].elapsedLifeTime / 0.2f));
							const Position startPos(playerEdgePosition.x - direction.x * 20, playerEdgePosition.y - direction.y * 20);
							const Position endPosition(lasers[i].x1 + direction.x * length, lasers[i].y1 + direction.y * length);
							drawDoubleLines(col, startPos, endPosition, offsetVector, 0.f, (3.f + j * 4.f));
						}

					}
					if (progress2 < 0.1f)
					{
						// draw darker lines outside the center line
						SDL_SetRenderDrawColor(render, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f, color.a * (1 - lasers[i].elapsedLifeTime / 0.1f));
						for (int j = 1; j < 8; j += 2)
						{
							drawDoubleLines(basePosition, endPosition, offsetVector, j, 5);
						}

					}
				}
			}

			// draw static transparent trail
			drawSingleLine(color.multiplied(0.8f, (1 - progress2) * 0.5f), basePosition, endPosition);
			drawDoubleLines(color.multiplied(0.8f, (1 - progress2) * 0.25f), basePosition, endPosition, offsetVector, 1);

			// draw moving transparent trail
			SDL_SetRenderDrawColor(render, color.r, color.g, color.b, 255 * (1 - progress2 / 0.8f));
			if (progress2 < 0.8f && progress2 > 0.05f)
			{
				SDL_RenderDrawLineF(render, startPosition.x - direction.x * 50.f, startPosition.y - direction.y * 50.f, endPosition.x, endPosition.y);
			}

			//draw highlight
			SDL_SetRenderDrawColor(render, 255, 255, 255, color.a);
			SDL_RenderDrawLineF(render, startPosition.x, startPosition.y, endPosition.x, endPosition.y);
		}
	}

	void drawSimpleCircle(float x, float y, float radius, int resolution = 6)
	{
		const float step = (2 * PI) / resolution;
		for (int i = 0; i < resolution; ++i)
		{
			const float angle = step * i;
			const float x1 = cos(angle);
			const float y1 = sin(angle);

			const float nextAngle = step * (i + 1);
			const float x2 = cos(nextAngle);
			const float y2 = sin(nextAngle);

			SDL_RenderDrawLine(
				render, x1 * radius + x, y1 * radius + y, x2 * radius + x, y2 * radius + y
			);
		}
	}

	// Draws a star made out of two triangles that are based on the same square, 
	// offset to make them look like a star
	void drawStar(Star star, float size, float elapsedTime, int thickness = 3)
	{
		// triangle pointing downward \/
		SDL_FPoint downwardTriangle[4];
		downwardTriangle[0] = SDL_FPoint(star.x - size / 2, star.y - size / 2);
		downwardTriangle[1] = SDL_FPoint(downwardTriangle[0].x + size, downwardTriangle[0].y);
		downwardTriangle[2] = SDL_FPoint(star.x, star.y + size / 2);
		downwardTriangle[3] = downwardTriangle[0];

		// triangle pointing upward /\ 
		SDL_FPoint upwardTriangle[4];
		upwardTriangle[0] = SDL_FPoint(star.x - size / 2, star.y + size / 2);
		upwardTriangle[1] = SDL_FPoint(upwardTriangle[0].x + size, upwardTriangle[0].y);
		upwardTriangle[2] = SDL_FPoint(star.x, star.y - size / 2);
		upwardTriangle[3] = upwardTriangle[0];

		for (int p = 0; p < 4; p++)
		{
			// offset y to centralize both triangles to make them look like a star together
			downwardTriangle[p].y += size / 8;
			upwardTriangle[p].y -= size / 8;
			// rotate all points of the triangles, to make the star spin
			rotatePoint(downwardTriangle[p].x, downwardTriangle[p].y, star.x, star.y, elapsedTime * 50);
			rotatePoint(upwardTriangle[p].x, upwardTriangle[p].y, star.x, star.y, elapsedTime * 50);
		}

		SDL_RenderDrawLinesF(render, downwardTriangle, 4);
		SDL_RenderDrawLinesF(render, upwardTriangle, 4);
	}


	void drawStars(MusicData* music, std::vector<Star> starList, float elapsedTime)
	{
		for (int i = starList.size() - 1; i > 0; i--)
		{
			const int lifeTime = static_cast<int>(std::round(starList[i].totalLifeTime));

			const float beatMultiplier =
				lifeTime == 1 ? music->quarterNoteProgress * 0.5f + 0.5f :
				lifeTime == 2 ? 1 - (music->quarterNoteProgress * 0.5f + 0.5f) :
				lifeTime == 3 ? Ease::Out(music->pulseMultiplier, 3) :
				lifeTime == 4 ? 1 - Ease::Out(music->pulseMultiplier, 3) : 1.f;
			;

			const float normalizedLife = starList[i].elapsedLifeTime / (music->wholeNoteLength * starList[i].totalLifeTime);
			const float multiplier = 1 - Ease::Out(normalizedLife, 2);
			const Color color = starList[i].color;
			SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a * multiplier * beatMultiplier);
			drawStar(starList[i], std::clamp(starList[i].maxSize * multiplier, 1.f, starList[i].maxSize), elapsedTime);
		}
	}

	// Used for all note lengths, during active beat
	void drawActiveBeatCircle(Position pos, Color color, float distanceMultiplier = 1.f)
	{
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a * distanceMultiplier * distanceMultiplier);

		if (pos.radius > MIN_SIZE_HALFNOTE)
			drawHexagon(pos.x, pos.y, pos.radius + 6);

		drawHexagon(pos.x, pos.y, pos.radius + 1, 1.f);
	}

	// Used for whole notes and half notes, during off beat timing
	void drawOffBeatCircle(Position pos, Color color, float beatProgress, float secondBeatProgress, float radius, float distanceMultiplier = 1.f)
	{
		if (beatProgress < 0.5f)
		{
			float radius2 = std::lerp(radius, pos.radius, secondBeatProgress);
			radius2 = std::clamp(radius2, pos.radius, radius);
			SDL_SetRenderDrawColor(render, color.r, 0, color.b, color.a * 0.66f * distanceMultiplier);
			drawHexagon(pos.x, pos.y, radius2);
		}
		const float halfNoteMultiplier = 1 + (1 - secondBeatProgress);
		SDL_SetRenderDrawColor(render, 100, 0, 200, 100 * distanceMultiplier * halfNoteMultiplier);
		drawHexagon(pos.x, pos.y, pos.radius);
		drawHexagon(pos.x, pos.y, pos.radius + 1);
		SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a * distanceMultiplier);
	}
	Color getBeatCircleColor(bool isBeatActive, float beatProgress, float fadeInProgress, int add = 0)
	{
		fadeInProgress = std::clamp(fadeInProgress, 0.f, 1.f);
		return Color{
			isBeatActive ? 60.f : std::lerp(100.f, 80.f, Ease::In(beatProgress, 4)), 0.f,
			isBeatActive ? 225.f : (200 + add) * beatProgress,
			isBeatActive ? 255.f : (200 + add) * fadeInProgress
		};
	}
	float getBeatCircleRadius(float prevRadius, float timeElapsed, float beatLength, float synchDuration)
	{
		return std::lerp(prevRadius * 2.5f, prevRadius,
						(timeElapsed - synchDuration) / (beatLength - synchDuration));
	}
	void drawBeatCircles(MusicData* music, std::vector<Position> positions, std::vector<Tag> tags, size_t playerIndex)
	{
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);

		const float synchDuration = music->quarterNoteLength / 4;

		const bool quarterNoteActive = music->timeSinceLastQuarterNote < synchDuration;
		const bool wholeNoteActive   = music->timeSinceLastWholeNote   < synchDuration;
		const bool halfNoteActive    = music->timeSinceLastHalfNote    < synchDuration;
		
		const float quarterFadeInProgress = (music->timeSinceLastQuarterNote - synchDuration) / 0.1f;
		const float wholeFadeInProgress   = (music->timeSinceLastWholeNote   - synchDuration) / 0.5f;
		const float halfFadeInProgress    = (music->timeSinceLastHalfNote    - synchDuration) / 0.2f;

		const Color quarterNoteColor = getBeatCircleColor(quarterNoteActive, music->quarterNoteProgress, quarterFadeInProgress);
		const Color wholeNoteColor   = getBeatCircleColor(wholeNoteActive,   music->wholeNoteProgress, wholeFadeInProgress, 20);
		const Color halfNoteColor    = getBeatCircleColor(halfNoteActive,    music->halfNoteProgress, halfFadeInProgress, 15);

		for (size_t i = 0; i < positions.size(); i++)
		{
			if (i == playerIndex) continue;

			if (tags[i] == Tag::Unsplittable) // draw red pentagon if the object is disabled
			{
				SDL_SetRenderDrawColor(render, 150, 0, 0, 175);
				drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 1, 1.f);
				drawHexagon(positions[i].x, positions[i].y, positions[i].radius + 7);
				continue;
			}
			float radius = positions[i].radius;
			const float distanceToMouse = distance(mouseX, mouseY, positions[i].x, positions[i].y);
			constexpr float maxDistance = 1300;
			float distanceMultiplier = 1 - (distanceToMouse - (positions[i].radius * 1.5f) - 50) / maxDistance;

			distanceMultiplier = powf(std::clamp(distanceMultiplier,0.f,1.f), 3);

			if (positions[i].radius > MIN_SIZE_WHOLENOTE)
			{
				if (wholeNoteActive)
					drawActiveBeatCircle(positions[i], wholeNoteColor, distanceMultiplier);
				else
				{
					radius = getBeatCircleRadius(radius, music->timeSinceLastWholeNote, music->wholeNoteLength, synchDuration);

					drawOffBeatCircle(positions[i], wholeNoteColor, music->wholeNoteProgress, 
								      music->halfNoteProgress, radius, distanceMultiplier);
				}
			}
			else if (positions[i].radius > MIN_SIZE_HALFNOTE)
			{
				if (halfNoteActive)
					drawActiveBeatCircle(positions[i], halfNoteColor, distanceMultiplier);
				else
				{
					radius = getBeatCircleRadius(radius, music->timeSinceLastHalfNote, music->halfNoteLength, synchDuration);

					drawOffBeatCircle(positions[i], halfNoteColor, music->halfNoteProgress, 
					                  music->quarterNoteProgress, radius, distanceMultiplier);
				}
			}
			else // quarter notes
			{
				if (quarterNoteActive)
					drawActiveBeatCircle(positions[i], quarterNoteColor, distanceMultiplier);
				else
				{
					radius = getBeatCircleRadius(radius, music->timeSinceLastQuarterNote, music->quarterNoteLength, synchDuration);

					SDL_SetRenderDrawColor(render, 115, 0, 230, 100 * distanceMultiplier);
					drawHexagon(positions[i].x, positions[i].y, positions[i].radius);

					SDL_SetRenderDrawColor(render, quarterNoteColor.r, quarterNoteColor.g, quarterNoteColor.b,
					                       quarterNoteColor.a * distanceMultiplier);
				}
			}

			drawHexagon(positions[i].x, positions[i].y, radius, 1.f);
			UINT8 r, g, b, a;
			SDL_GetRenderDrawColor(render, &r, &g, &b, &a);
			SDL_SetRenderDrawColor(render, r, g, b, a * 0.9f);
			drawHexagon(positions[i].x, positions[i].y, radius + 5, 0.5f);
		}
	}

	void drawBackground()
	{
		const SDL_Rect rect(0, 0, WIDTH, HEIGHT);
		SDL_SetRenderDrawColor(render, 0, 1, 2, 255);
		SDL_RenderFillRect(render, &rect);
	}

	void renderPresent()
	{
		SDL_RenderPresent(render);
	}

	void renderClear()
	{
		SDL_RenderClear(render);
	}
}