#ifndef MENUSTATE_H
#define MENUSTATE_H

#include "framework.h"
#include "utils.h"
#include "state.h"
#include <iostream>

class Image;

class MenuState : public State {
protected:

public:

	static MenuState* instance;

	MenuState();
	bool rendering_options = false;

	//~MenuState();

	void init();
	void render(Image * framebuffer);
	void update(double elapsed_time);

	void onEnter();
	void onLeave(int fut_state);

	int stateID() { return 0; };
};

#endif