#include "player.h"
#include <algorithm>
#include "assets.h"
#include "easingFunctions.h"
#include "input.h"
#include "engine.h"

#define MAXVELOCITY 400.f
#define FRICTION 2.f

using namespace Engine;

Player::Player(MusicManager* musicManager, SDL_Renderer* renderer) : remainingHealth(maxHealth), musicManager(musicManager)
{
	const Position pos(Position( WIDTH / 2 - playerRadius / 2, HEIGHT * 0.8f, playerRadius));
	Engine::createObject(pos, Rotation(10, 0), Velocity(), 20, Assets::playerTexturePath, Tag::Player);

	overlay = Sprite(200, 200);
	overlay.load(Assets::sphericalLightPath, renderer);
	SDL_SetTextureBlendMode(overlay.texture, SDL_BLENDMODE_ADD);
}

void Player::update(float deltaTime, float pulseMultiplier)
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

	const Vector2 friction = velocity * deltaTime * FRICTION;
	velocity -= friction;
	Engine::updatePlayerVelocity(velocity.x, velocity.y);

	//timeSinceLastFail += deltaTime;
	//timeSinceLastSuccess += deltaTime;

	constexpr float fadeOutTime = 2.f;

	// draw laser shot overlay
	if (timeSinceLastSuccess < fadeOutTime || timeSinceLastFail < fadeOutTime)
	{
		timeSinceLastFail += deltaTime;
		timeSinceLastSuccess += deltaTime;
		// after failed shot
		if (timeSinceLastFail < timeSinceLastSuccess)
		{
			// tint player texture red for a duration after failed shot
			const int c = std::lerp(0, 255, Ease::Out(std::clamp(timeSinceLastFail / fadeOutTime, 0.f, 1.f), 2));
			const Color tint(255, c, c, 255);
			Engine::updatePlayerTextureMod(tint);

			const float progress = timeSinceLastFail / fadeOutTime;
			const int size = 150;
			const Position playerPos = Engine::getPlayerPos();
			overlay.updateOpacity(1 - Ease::Out(progress, 3));
			overlay.draw(playerPos.x - size / 2, playerPos.y - size / 2, size, size);
		}
		else // after successful shot
		{
			const float progress = timeSinceLastSuccess / fadeOutTime;
			const int size = static_cast<int>(std::lerp(150, 100, Ease::Out(progress, 2)));
			const Position playerPos = Engine::getPlayerPos();
			const float opacityMultiplier = max(1 - Ease::Out(progress, 5), Ease::InOutSine(pulseMultiplier) * 0.2f + 0.3f);
			overlay.updateOpacity(opacityMultiplier);
			overlay.draw(playerPos.x - size / 2, playerPos.y - size / 2, size, size);

			Engine::updatePlayerTextureMod(Color(255 * Ease::Out(progress + 0.1f, 5), 255.f, 255.f, 255.f));
		}
		if (timeSinceLastSuccess > fadeOutTime && timeSinceLastFail > fadeOutTime)
		{
			SDL_SetTextureColorMod(overlay.texture, 255, 255, 255);
			SDL_SetTextureBlendMode(overlay.texture, SDL_BLENDMODE_BLEND);
		}
	}
	else
	{
		const int size = 100;
		const Position playerPos = Engine::getPlayerPos();
		overlay.updateOpacity(Ease::InOutSine(pulseMultiplier) * 0.2f + 0.3f);
		overlay.draw(playerPos.x - size / 2, playerPos.y - size / 2, size, size);
	}
		//Engine::updatePlayerTextureMod(Color(255, 255, 255, 255).multiplied(1 - (Ease::InOutSine(1 - pulseMultiplier) * 0.2f)));

}

void Player::shootLaser(int mouseX, int mouseY, MusicData* musicData, bool& wasShotSuccessful)
{
	const Vector2 mousePosition{ static_cast<float>(mouseX), static_cast<float>(mouseY) };
	Position playerPos = Engine::getPlayerPos();
	Vector2 centerPos = Vector2(playerPos.x, playerPos.y);
	Vector2 direction = mousePosition - centerPos;
	direction.normalize();
	centerPos = centerPos + direction * (radius);
	playerPos = Position{ centerPos.x, centerPos.y };

	const Laser newLaser{ {LASER_DEFAULT_LIFETIME}, playerPos.x, playerPos.y, mousePosition.x, mousePosition.y };
	wasShotSuccessful = Engine::addLaser(newLaser, musicData);

	if (!wasShotSuccessful)
	{
		SDL_SetTextureColorMod(overlay.texture, 255, 0, 0);
		SDL_SetTextureBlendMode(overlay.texture, SDL_BLENDMODE_BLEND);
		timeSinceLastFail = 0;
	}
	else
	{
		SDL_SetTextureColorMod(overlay.texture, 255, 255, 255);
		SDL_SetTextureBlendMode(overlay.texture, SDL_BLENDMODE_ADD);
		timeSinceLastSuccess = 0;
	}
		
}

void Player::reset()
{
	remainingHealth = maxHealth;
	velocity = { 0.f, 0.f };
}

void Player::takeDamage(int damage)
{
	musicManager->playGlitchSound();
	if (remainingHealth > 0)
		remainingHealth -= damage;

	if(remainingHealth <= 0)
	{
		musicManager->stopPlaying();
		Engine::clearObjects();
		musicManager->playGlitchSound();
	}
}

void Player::destroy()
{
	overlay.destroy();
}
