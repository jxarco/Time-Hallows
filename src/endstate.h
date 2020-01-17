#ifndef ENDSTATE_H
#define ENDSTATE_H

#include "framework.h"
#include "utils.h"
#include "state.h"
#include <iostream>

class Image;

class EndState : public State {
protected:

public:

	EndState();
	// ~EndState();

	static EndState* instance;

	void init();
	void render(Image * framebuffer);
	void update(double elapsed_time);

	void onEnter();
	void onLeave(int fut_state);

	int stateID() { return 0; };
};

#endif