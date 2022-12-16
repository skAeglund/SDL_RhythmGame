#pragma once
#include <SDL.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "sprite.h"
#include "collision.h"
#include "player.h"

#define MIN_SIZE_WHOLENOTE 45
#define MIN_SIZE_HALFNOTE 20
#define MIN_SIZE_TRIPPLESPLIT 32
#define WIDTH 1920
#define HEIGHT 1080
#define HEALTH_LINE_HEIGHT HEIGHT - 100

bool initializeEngine(int* currentHealth);
void quit();
SDL_Renderer* getRenderer();

void drawCircles(float x, float y, float radius);
void drawBeatCircles();
void drawLines(SDL_Color color);
void drawHealthLine();
void drawObjects();
void drawColliders();
void sortObjects();
void moveObjects();
void rotateObjects();
void updatePlayerVelocity(float x, float y);
int getObjectCount();

void renderPresent();
void renderClear();

float updateTicks();
void delayNextFrame();
void printTimeStats();

bool getKeyDown(int index);
void updateKey(int index, bool value);
void resetKeys();

struct Position { float x; float y; float radius = 50; };
struct Velocity { float xVelocity; float yVelocity; };
struct Rotation { float force; float angle; };
struct Appearance { SDL_Texture* texture; float scaleOffset = 0; SDL_Color tint = SDL_Color(255, 255, 255, 0); };

struct ObjectPendingDeletion { Position position; Appearance appearance; float elapsedFadeOutTime; float angle; };

enum class Tag { Asteroid, Unsplittable, Player, none };


struct Line { float x1; float y1; float x2; float y2; float elapsedTime = 0.f; SDL_Color color = SDL_Color(0, 255, 255, 255); };

Position getPlayerPosition();
void createObject(Position pos, Rotation rot, Velocity vel, float scaleOffset = 0, SDL_Texture* texture = nullptr, Tag tag = Tag::Asteroid);
void createObject(Position position, Rotation rotation, Velocity velocity, float scaleOffset, const char* texturePath, Tag tag = Tag::Asteroid);
void removeObject(int index);
void addLine(Line line);

struct Color { int r; int g; int b; };



