#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "framework.h"
#include "utils.h"
#include "state.h"
#include <iostream>

class Image;

class PlayState : public State {
protected:

public:

	static PlayState * instance;

	PlayState();

	// ~PlayState();

	void init();
	void render(Image * framebuffer);
	void update(double elapsed_time);

	void onEnter();
	void onLeave(int fut_state);

	void checkInputs();
	int stateID() { return 1; };

	float playTime;
};

#endif