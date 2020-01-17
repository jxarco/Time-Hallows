#ifndef CAMERA_H
#define CAMERA_H

#include "includes.h"
#include "utils.h"
#include "character.h"

class Camera
{
private:

	Character* character;

	int shake_elapsed;
	int shake_length;
	int shake_amount;

public:

	int x;
	int y;

	bool smoothing;

	Camera();
	Camera(int x, int y);
	~Camera();

	inline void setCharacter(Character* c) { this->character = c; };

	void update(double seconds_elapsed);
	void lookAt(int x, int y, bool smooth = false);
	void follow(Character* chtr);
	void shake(int amount, int length);
	
};


#endif 