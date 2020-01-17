#include "state.h"
#include "playstate.h"
#include "endstate.h"
#include "game.h"
#include "utils.h"
#include "world.h"
#include "ui.h"
#include "image.h"
#include "input.h"

PlayState* PlayState::instance = NULL;

bool counter_reseted = false;

PlayState::PlayState() : State()
{
	
}

/*PlayState::~PlayState() {

}*/

void PlayState::init()
{
	std::cout << "$ init play state" << std::endl;

	World * world = World::instance;
	world->begin();
}

void PlayState::onEnter()
{
	std::cout << "$ Entering play state" << std::endl;
	this->init();
}

void PlayState::render(Image * framebuffer)
{
	Game * game = Game::instance;

	World::instance->render(framebuffer);
	GUI::instance->render(*framebuffer);

	// the world renders but it's not been updated
	if (!game->START)
		return;

	// show attacks
	if (game->pendingAttack)
		framebuffer->drawImage(*Image::Get("data/ui/red.tga"), 0, 0, game->bufferSize, game->bufferSize);
}

void PlayState::update(double seconds_elapsed)
{
	World * world = World::instance;
	Character * chtr = world->mainCharacter;
	Game * game = Game::instance;
	GUI * gui = GUI::instance;

	if (Input::wasKeyPressed(SDL_SCANCODE_Z))
	{
		if (game->textLines.size() == 0)
			chtr->interact();
		else
			gui->updateText();
	}

	if (!game->START && game->must_start && !GUI::instance->writing) {

		if (!counter_reseted)
		{
			game->totalTimeElapsed = 0;
			counter_reseted = true;
		}
		
		game->START = true;
	}
	
	world->timeDeath++;

	// rain is always updating
	if (world->dayCondition == RAINY && !world->mainCharacter->isAtCave)
		world->rainSystem->update(seconds_elapsed);

	if (!game->START)
		return;

	world->update(seconds_elapsed);

	if (game->pendingAttack)
		game->redScreenTimer++;

	if (game->redScreenTimer > 30) {
		game->pendingAttack = false;
		game->redScreenTimer = 0;
	}

	// do not move if pending text
	if (game->textLines.size() > 0)
		return;

	// check for player movement
	checkInputs(); 

	if (game->GAME_END || game->GAME_OVER)
		StateManager::instance->changeCurrentState(EndState::instance);
}

void PlayState::checkInputs()
{
	Character * chtr = World::instance->mainCharacter;

	if (Input::isKeyPressed(SDL_SCANCODE_DOWN))
		chtr->moveDown();
	else if (Input::isKeyPressed(SDL_SCANCODE_RIGHT))
		chtr->moveRight();
	else if (Input::isKeyPressed(SDL_SCANCODE_LEFT))
		chtr->moveLeft();
	else if (Input::isKeyPressed(SDL_SCANCODE_UP))
		chtr->moveUp();
	
	if (Input::isKeyPressed(SDL_SCANCODE_W))
	{
		chtr->y -= 10;
		chtr->moveUp();
	}
	if (Input::isKeyPressed(SDL_SCANCODE_A))
	{
		chtr->x -= 10;
		chtr->moveLeft();
	}
	if (Input::isKeyPressed(SDL_SCANCODE_S))
	{
		chtr->y += 10;
		chtr->moveDown();
	}
	if (Input::isKeyPressed(SDL_SCANCODE_D))
	{
		chtr->x += 10;
		chtr->moveRight();
	}
}

void PlayState::onLeave(int fut_state)
{

}
