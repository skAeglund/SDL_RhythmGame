#include "player.h"
#include "input.h"
#include <iostream>
#include <algorithm>

#include <windows.h> // <- testing, remove later
#include <iomanip> // <- testing, remove later
#define MAXVELOCITY 400.f
#define FRICTION 2.f

Player::Player()
{
	forceBallTexture = IMG_LoadTexture(getRenderer(), "Content/Sprites/forceBall.png");
	Player::remainingHealth = maxHealth;
}

void Player::update(float deltaTime)
{
	Vector2 inputDirection;
	if (getKeyDown(InputKey::moveLeft))
		inputDirection.x -= 1;
	if (getKeyDown(InputKey::moveRight))
		inputDirection.x += 1;
	if (getKeyDown(InputKey::moveUp))
		inputDirection.y -= 1;
	if (getKeyDown(InputKey::moveDown))
		inputDirection.y += 1;

	inputDirection.normalize();
	
	velocity += inputDirection * 500.f *deltaTime;
	velocity.clamp(0, MAXVELOCITY);

	//position += velocity;
	Vector2 friction = velocity * deltaTime * FRICTION;
	velocity -= friction;
	updatePlayerVelocity(velocity.x, velocity.y);

}


void Player::shoot(int mouseX, int mouseY)
{
	Vector2 mousePosition{ (float)mouseX, (float)mouseY };
	Position position = getPlayerPosition();
	Vector2 centerPosition{ (position.x/* + width / 2*/), (position.y/* + height / 2*/) };
	Vector2 direction = mousePosition - centerPosition;
	direction.normalize();
	centerPosition = centerPosition + direction * (radius +22);
	Velocity projectileVelocity(direction.x * 700, direction.y* 700);
	Position pos{ centerPosition.x , centerPosition.y , 20.f};
	createObject(pos, Rotation(0,0), projectileVelocity, 10.f, forceBallTexture);
}
void Player::laser(int mouseX, int mouseY)
{
	Vector2 mousePosition{ (float)mouseX, (float)mouseY };
	Position playerPos = getPlayerPosition();
	Vector2 centerPos = Vector2(playerPos.x, playerPos.y);
	Vector2 direction = mousePosition - centerPos;
	direction.normalize();
	centerPos = centerPos + direction * (radius);
	playerPos = Position(centerPos.x, centerPos.y);
	addLine(Line(playerPos.x, playerPos.y, (float)mouseX, (float)mouseY));
}

void Player::reset()
{
	remainingHealth = maxHealth;
	velocity = { 0.f, 0.f };
}


//testing
//static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//std::cout.flush();
//COORD coord = { (SHORT)0, (SHORT)10 };
//SetConsoleCursorPosition(hOut, coord);
//std::cout << "Length: " << std::setprecision(5) << std::fixed << direction.length() << std::endl;