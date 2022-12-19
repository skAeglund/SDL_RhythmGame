#include "player.h"
#include "input.h"
#include <iostream>
#include <algorithm>

#define MAXVELOCITY 400.f
#define FRICTION 2.f

using namespace Engine;

Player::Player()
{
	Player::remainingHealth = maxHealth;
}

void Player::update(float deltaTime)
{
	Vector2 inputDirection;
	if (Engine::getKeyDown(InputKey::moveLeft))
		inputDirection.x -= 1;
	if (Engine::getKeyDown(InputKey::moveRight))
		inputDirection.x += 1;
	if (Engine::getKeyDown(InputKey::moveUp))
		inputDirection.y -= 1;
	if (Engine::getKeyDown(InputKey::moveDown))
		inputDirection.y += 1;

	inputDirection.normalize();
	
	velocity += inputDirection * 500.f *deltaTime;
	velocity.clamp(0, MAXVELOCITY);

	Vector2 friction = velocity * deltaTime * FRICTION;
	velocity -= friction;
	Engine::updatePlayerVelocity(velocity.x, velocity.y);

}

// this is inactivated for now
void Player::shootBall(int mouseX, int mouseY)
{
	Vector2 mousePosition{ (float)mouseX, (float)mouseY };
	Position position = Engine::getPlayerPos();
	Vector2 centerPosition{ (position.x), (position.y) };
	Vector2 direction = mousePosition - centerPosition;
	direction.normalize();
	centerPosition = centerPosition + direction * (radius +22);
	Velocity projectileVelocity(direction.x * 700, direction.y* 700);
	Position pos{ centerPosition.x , centerPosition.y , 20.f};
	Engine::createObject(pos, Rotation(0,0), projectileVelocity, 10.f/*, forceBallTexture*/);
}

void Player::shootLaser(int mouseX, int mouseY, MusicData* musicData, bool& wasShotSuccessful)
{
	Vector2 mousePosition{ (float)mouseX, (float)mouseY };
	Position playerPos = Engine::getPlayerPos();
	Vector2 centerPos = Vector2(playerPos.x, playerPos.y);
	Vector2 direction = mousePosition - centerPos;
	direction.normalize();
	centerPos = centerPos + direction * (radius);
	playerPos = Position(centerPos.x, centerPos.y);
	wasShotSuccessful = Engine::addLaser(Laser(playerPos.x, playerPos.y, (float)mouseX, (float)mouseY), musicData);
}

void Player::reset()
{
	remainingHealth = maxHealth;
	velocity = { 0.f, 0.f };
}
