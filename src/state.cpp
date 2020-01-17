#include "game.h"
#include "utils.h"
#include "state.h"
#include "image.h"



State::State() {}
//State::~State() {}

void State::init() {}
void State::render(Image * framebuffer) {}
void State::update(double t) {}

int State::stateID() { return -1; }

void State::onEnter() {}
void State::onLeave(int fut_state) {}

// *********************************************************************************************************
// *********************************************************************************************************

StateManager* StateManager::instance = NULL;

StateManager::StateManager() {

	current_state = NULL;
}
//StateManager::~StateManager() {}

void StateManager::setInitialState(State* state) {
	current_state = state;
	current_state->onEnter();
}

void StateManager::changeCurrentState(State* new_state) {

	if (current_state) current_state->onLeave(new_state->stateID());
	current_state = new_state;
	current_state->onEnter();
}

void StateManager::init() {
	current_state->init();
}

void StateManager::render(Image  * framebuffer) {
	current_state->render(framebuffer);
}

void StateManager::update(double time_elapsed) {
	current_state->update(time_elapsed);
}

int StateManager::stateID() {
	return current_state->stateID();
}