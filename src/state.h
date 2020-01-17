#ifndef STATE_H
#define STATE_H

#include "framework.h"
#include "utils.h"
#include <iostream>

class StateManager;
class Image;

class State {

public:

	State();
	//virtual ~State();

	virtual void init();
	virtual void render(Image * framebuffer);
	virtual void update(double t);

	virtual int stateID();

	virtual void onEnter();
	virtual void onLeave(int fut_state);
};

// *********************************************************************************************************
// *********************************************************************************************************

// Manager pattern

class StateManager : State {
private:
	State * current_state;

public:
	
	static StateManager * instance;

	StateManager();
	//virtual ~StateManager();

	// changing states
	void setInitialState(State* state);
	void changeCurrentState(State* new_state);
	//

	State* getCurrentState() {
		return current_state;
	}

	void init();
	void render(Image * framebuffer);
	void update(double time_elapsed);
	
	int stateID();
};

#endif
