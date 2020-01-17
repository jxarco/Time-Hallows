#include "utils.h"
#include "camera.h"
#include "input.h"
#include "game.h"
#include "world.h"

#include <cmath>
#include <algorithm>
#include <vector>

Camera::Camera()
{
	smoothing = true;
	// shake_elapsed = 0;
	shake_length = 0;
	shake_amount = 0;
}

Camera::~Camera()
{

}

Camera::Camera(int x, int y)
{
	this->x = x;
	this->y = y;
	smoothing = true;
	// shake_elapsed = 0;
	shake_length = 0;
	shake_amount = 0;
}

void Camera::update(double seconds_elapsed)
{
	this->follow(this->character);

	if (shake_length > 0)
	{
		int shakeX = random() * shake_amount * 2 - shake_amount;
		int shakeY = random() * shake_amount * 2 - shake_amount;

		/*int x = this->x + shakeX;
		int y = this->y + shakeY;*/
		
		this->x += shakeX;
		this->y += shakeY;

		shake_length--;

		/*int gameMapSize = World::instance->map.width;
		int bufferSize = Game::instance->bufferSize;
		int margin = bufferSize / 2;

		this->x = clamp(this->x, margin, gameMapSize - margin) - bufferSize / 2;
		this->y = clamp(this->y, margin, gameMapSize - margin) - bufferSize / 2;*/
	}
}

void Camera::follow(Character * chtr)
{
	int gameMapSize = World::instance->map.width;
	int bufferSize = Game::instance->bufferSize;

	int margin = bufferSize / 2;
	int targetx = clamp(chtr->x, margin, gameMapSize - margin) - bufferSize/2;
	int targety = clamp(chtr->y, margin, gameMapSize - margin) - bufferSize/2;

	this->x = smoothing ? lerp(targetx, this->x, 0.9) : targetx;
	this->y = smoothing ? lerp(targety, this->y, 0.9) : targety;
}

void Camera::lookAt(int x, int y, bool smooth)
{
	int gameMapSize = World::instance->map.width;
	int bufferSize = Game::instance->bufferSize;

	int margin = bufferSize / 2;
	int targetx = clamp(x, margin, gameMapSize - margin) - bufferSize / 2;
	int targety = clamp(y, margin, gameMapSize - margin) - bufferSize / 2;

	this->x = smooth ? lerp(targetx, this->x, 0.9) : targetx;
	this->y = smooth ? lerp(targety, this->y, 0.9) : targety;
}

void Camera::shake(int amount, int length)
{
	shake_amount = amount;
	shake_length = length;
}
