#include "state.h"
#include "menustate.h"
#include "playstate.h"
#include "endstate.h"
#include "game.h"
#include "utils.h"
#include "world.h"
#include "ui.h"
#include "image.h"
#include "input.h"

#include <cmath>

EndState* EndState::instance = NULL;

int fadeEnd = 255;

EndState::EndState()
{

}

// EndState::~EndState() {}

void EndState::init()
{

}

void EndState::onEnter()
{
	std::cout << "$ Entering menu state" << std::endl;
	this->init();
}

void EndState::render(Image * framebuffer)
{
	Game * game = Game::instance;

	if (game->GAME_END)
	{
		framebuffer->drawImage(*Image::Get("data/ui/end.tga"), 0, 0);
	}
	else
	{
		framebuffer->drawImage(*Image::Get("data/ui/over.tga"), 0, 0);
	}

	if (fadeEnd > 0) {
		framebuffer->drawRectangle(0, 0, game->bufferSize, game->bufferSize, Color(0x000, fadeEnd), true);
		fadeEnd -= 2;
	}
}

void EndState::update(double seconds_elapsed)
{
	Game * game = Game::instance;

	if (Input::wasKeyPressed(SDL_SCANCODE_Z))
	{
		game->must_exit = true;
	}
}

void EndState::onLeave(int fut_state)
{

}
