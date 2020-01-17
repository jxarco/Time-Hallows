#ifndef SPRITE_H
#define SPRITE_H

#include "includes.h"
#include "utils.h"
#include "image.h"

class Sprite
{
protected:

	Image font;

public:

	Image image;
	int rows;
	int cols;

	int w;
	int h;
	int x;
	int y;

	int x0;
	int y0;

	// future position
	bool hasToMove;
	int x1;
	int y1;

	// For changing the current sprite
	int currentRow;
	int currentCol;
	int transition;
	int timer;

	Sprite();
	Sprite(const char* filename, int w, int h);
	Sprite(const char* filename, int w, int h, int rows, int cols, int beginr, int beginc);
	~Sprite();

	inline int getW() { return this->w; };
	inline int getH() { return this->h; };

	void setPosition(int x, int y, bool postpose = false);
	void setDistribution(int rows, int cols, int beginr, int beginc);
	void setTransition(int transition);
	
	void render(Image& frameBuffer);
	void render(Image& frameBuffer, bool debug);
};


#endif 