#include "state.h"
#include "menustate.h"
#include "playstate.h"
#include "game.h"
#include "utils.h"
#include "world.h"
#include "ui.h"
#include "image.h"
#include "input.h"

#include <cmath>

MenuState* MenuState::instance = NULL;

int selected = 0;
int o_selected = 0;
int character_selected = 0;
int chtr_timer = 0;
int chtr_tile_col = 0;

// fade transition
int fade = 0;
bool must_leave = false;

bool rain = false;

const char* time_modes[] = { "AUTO", "DAY", "NIGHT" };
int time_selected = 0;

const char* pnjs[] = {"Character 1", "Character 2", "Character 3"};
const char* pnjs_path[] = { 
	"data/characters/med.tga",
	"data/characters/little.tga",
	"data/characters/big.tga"
};

Image font; 


MenuState::MenuState()
{

}

//MenuState::~MenuState() {}

void MenuState::init()
{
	font = *Image::Get("data/fonts/bitmap-font-black.tga");
}

void MenuState::onEnter()
{
	std::cout << "$ Entering menu state" << std::endl;
	this->init();
}

void MenuState::render(Image * framebuffer)
{
	Game * game = Game::instance;
	
	int button_w = 164;
	int button_h = 16;

	if (rendering_options)
	{
		framebuffer->drawImage(*Image::Get("data/ui/options.tga"), 0, 0);

		framebuffer->drawRectangle(game->bufferSize / 2 - button_w / 2 + 1, game->bufferSize / 2 + 25, button_w, button_h, o_selected == 0 ? Color::WHITE : Color(0xe0dfd5, 117), true);
		framebuffer->drawRectangle(game->bufferSize / 2 - button_w / 2 + 8, game->bufferSize / 2 + 29, 7, 7, Color::GREEN, rain);
		framebuffer->drawText("Rain", game->bufferSize / 2 - button_w / 2 + 20, game->bufferSize / 2 + 28, font);
		
		framebuffer->drawRectangle(game->bufferSize / 2 - button_w / 2 + 1, game->bufferSize / 2 + 40, button_w, button_h, o_selected == 1 ? Color::WHITE : Color(0xe0dfd5, 117), true);
		framebuffer->drawText(time_modes[time_selected], game->bufferSize / 2 - button_w / 2 + 3, game->bufferSize / 2 + 43, font);

		framebuffer->drawRectangle(game->bufferSize / 2 - button_w / 2 + 1, game->bufferSize / 2 + 55, button_w, button_h, o_selected == 2 ? Color::WHITE : Color(0xe0dfd5, 117), true);
		framebuffer->drawText( pnjs[character_selected] , game->bufferSize / 2 - button_w / 2 + 3, game->bufferSize / 2 + 58, font);


		// draw pnj
		framebuffer->drawImage(*Image::Get(pnjs_path[character_selected]), game->bufferSize / 2 - 16, game->bufferSize / 2 + 75, Area(chtr_tile_col*32,0,32,32));
		return;
	}

	framebuffer->drawImage(*Image::Get("data/ui/menu.tga"), 0, 0);

	if(selected == 0)
		framebuffer->drawLine(81, 192, 168, 192, Color(0xff7f00, 180));
	else if (selected == 1)
		framebuffer->drawLine(46, 223, 124, 223, Color(0xff7f00, 180));
	else if (selected == 2)
		framebuffer->drawLine(134, 223, 204, 223, Color(0xff7f00, 180));

	if (must_leave)
	{
		framebuffer->drawRectangle(0, 0, game->bufferSize, game->bufferSize, Color(0x000, fade), true);
		fade += 4;
		if(fade > 255)
			StateManager::instance->changeCurrentState(PlayState::instance);
	}
}

void MenuState::update(double seconds_elapsed)
{
	Game * game = Game::instance;

	chtr_timer++;
	if (chtr_timer > 6) {
		chtr_tile_col++;
		chtr_timer = 0;
		if (chtr_tile_col > 2)
			chtr_tile_col = 0;
	}
		
	if (Input::wasKeyPressed(SDL_SCANCODE_Z))
	{
		Game::instance->synth.playSample("data/music/menu.wav", 1, false);

		if (this->rendering_options)
		{
			if (o_selected == 0)
				rain = !rain;
			return;
		}

		switch (selected)
		{
			case 0:
				must_leave = true; // fade to playstate
			break;
			case 1:
				this->rendering_options = true;
				break;
			case 2:
				Game::instance->must_exit = true;
				break;
		}
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_X))
	{
		Game::instance->synth.playSample("data/music/menu.wav", 1, false);
		this->rendering_options = false;
		Game::instance->character_selected = character_selected;
		Game::instance->time_selected = time_selected;
		Game::instance->raining = rain;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_DOWN))
	{
		if (this->rendering_options)
		{
			Game::instance->synth.playSample("data/music/menu.wav", 1, false);

			o_selected++;
			if (o_selected > 2)
				o_selected = 0;
		}
		else {
			Game::instance->synth.playSample("data/music/menu.wav", 1, false);
			selected++;
			if (selected > 2)
				selected = 0;
		}
		
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_UP))
	{
		Game::instance->synth.playSample("data/music/menu.wav", 1, false);

		if (this->rendering_options)
		{			
			o_selected--;
			if (o_selected < 0)
				o_selected = 2;
		}
		else {
			selected--;
			if (selected < 0)
				selected = 2;
		}
	}

	else if (Input::wasKeyPressed(SDL_SCANCODE_LEFT))
	{
		if (o_selected == 0)
			return;

		Game::instance->synth.playSample("data/music/menu.wav", 1, false);
		
		if (o_selected == 1)
		{
			time_selected--;
			if (time_selected < 0)
				time_selected = 2;
		}
		else {
			character_selected--;
			if (character_selected < 0)
				character_selected = 2;
		}
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_RIGHT))
	{
		if (o_selected == 0)
			return;

		Game::instance->synth.playSample("data/music/menu.wav", 1, false);

		if (o_selected == 1)
		{
			time_selected++;
			if (time_selected > 2)
				time_selected = 0;
		}
		else {
			character_selected++;
			if (character_selected > 2)
				character_selected = 0;
		}
	}

}

void MenuState::onLeave(int fut_state)
{

}
