#include "utils.h"
#include "sprite.h"
#include "image.h"
#include "world.h"

#include <cmath>
#include <algorithm>
#include <vector>

Sprite::Sprite()
{
	
}

Sprite::~Sprite()
{
	
}

Sprite::Sprite(const char* filename, int w, int h)
{
	this->w = w;
	this->h = h;

	this->rows = 0;
	this->cols = 0;

	this->currentRow = 0;
	this->currentCol = 0;
	this->transition = 5;
	this->timer = 0;

	this->image = *Image::Get(filename);
	this->font = *Image::Get("data/fonts/mini-font-white-4x6.tga");
}

Sprite::Sprite(const char* filename, int w, int h, int rows, int cols, int beginr, int beginc)
{
	this->w = w;
	this->h = h;

	this->rows = rows;
	this->cols = cols;
	this->currentRow = beginr;
	this->currentCol = beginc;
	this->transition = 5;
	this->timer = 0;

	this->image = *Image::Get(filename);
	this->font = *Image::Get("data/fonts/mini-font-white-4x6.tga");
}

void Sprite::setPosition(int x, int y, bool postpose)
{
	if (postpose)
	{
		x1 = x;
		y1 = y;
		hasToMove = true;
		return;
	}

	this->x = x;
	this->y = y;
	this->x0 = x;
	this->y0 = y;
}

void Sprite::setDistribution(int rows, int cols, int beginr, int beginc)
{
	this->rows = rows;
	this->cols = cols;

	this->currentRow = beginr;
	this->currentCol = beginc;
}

void Sprite::setTransition(int transition)
{
	this->transition = transition;
}

void Sprite::render(Image& frameBuffer)
{
	int w = this->w;
	int h = this->h;
	int x = this->x;
	int y = this->y;
	int x0 = this->currentCol * w;
	int y0 = this->currentRow * h;

	frameBuffer.drawImage(this->image, x, y, Area(x0, y0, w, h));
}

void Sprite::render(Image& frameBuffer, bool debug)
{
	int w = this->w;
	int h = this->h;
	int x = this->x;
	int y = this->y;
	int x0 = this->currentCol * w;
	int y0 = this->currentRow * h;

	if (debug) {

		frameBuffer.drawRectangle(x, y, w, h, Color::GREEN, false);
	}
	frameBuffer.drawImage(this->image, x, y, Area(x0, y0, w, h));
}
