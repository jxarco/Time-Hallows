#include "utils.h"
#include "ui.h"

#include <cmath>
#include <algorithm>
#include <vector>

GUI* GUI::instance = NULL;

bool _prompt_available = false;

GUI::GUI()
{
	world = World::instance;
	show = false;
}

GUI::~GUI()
{

}

void GUI::render(Image &framebuffer)
{
	int bufferSize = Game::instance->bufferSize;
	World* world = World::instance;
	Game * game = Game::instance;
	Camera * camera = world->camera;

	if (!game->START)
	{
		cinematic_height = bufferSize / 4;
		framebuffer.drawRectangle(0, 0, bufferSize, cinematic_height, Color::BLACK, true);
		framebuffer.drawRectangle(0, bufferSize - cinematic_height, bufferSize, cinematic_height, Color::BLACK, true);
		this->renderText(framebuffer);
		return;
	}

	// render again cinematic but remove slowly
	cinematic_height = lerp(0, cinematic_height, 0.99);
	framebuffer.drawRectangle(0, 0, bufferSize, cinematic_height, Color::BLACK, true);
	framebuffer.drawRectangle(0, bufferSize - cinematic_height, bufferSize, cinematic_height, Color::BLACK, true);

	float map_size = 40; // don't change
	float player_size = 4;

	// pintar mapa
	framebuffer.drawImage(world->map, 1, bufferSize - map_size - 1, map_size, map_size);

	float x = float(World::instance->mainCharacter->x) * map_size / float(World::instance->map.width);
	float y = float(World::instance->mainCharacter->y ) * map_size / float(World::instance->map.height);
	framebuffer.drawRectangle(1 + x - player_size/2, bufferSize - map_size - player_size/2 + y - 1, player_size, player_size, Color::RED, true);

	float life_x = float(bufferSize) / 30;
	float life_y = map_size;
	float life = World::instance->mainCharacter->lifePoints;
	float max = World::instance->mainCharacter->maxLifePoints;

	Color c = Color::WHITE;

	if (life / max < 0.75)
		c = Color(0xffff00);
	if (life / max < 0.5)
		c = Color(0xff6600);
	if (life / max < 0.25)
		c = Color::RED;
	
	life *= life_y / max;

	framebuffer.drawRectangle(2 + map_size, bufferSize - life_y - 1, life_x, life_y, Color(0x444, 180), true);
	framebuffer.drawRectangle(3 + map_size, bufferSize - life, life_x - 2, life - 2, c, true);

	// HOUR
	Image font = *Image::Get("data/fonts/bitmap-font-white.tga");
	
	/*double factor = Game::instance->gameTime / 60.0;
	int totalMinutes = factor * 1440;
	int minutes = totalMinutes % 60;
	int hours = float(totalMinutes) / 60;*/

	std::string sss = std::to_string((int)(900 - Game::instance->totalTimeElapsed)/60) + std::string(" min");
	framebuffer.drawText(sss.c_str(), bufferSize - map_size - 20, bufferSize - 1 - 9, font);

	// pintar la tierra -> tiempo de vida
	float timeLeft = 1.0 - (Game::instance->totalTimeElapsed / 900);
	map_size = 25;

	if (timeLeft < 0 && !game->GAME_OVER)
	{
		this->startCinema();
		game->sendText("\"It's over.\"");
		game->sendText("");
		game->sendText("\"Almost the 98% of the world");
		game->sendText("population has already dead\"");
		game->sendText("\"No one could save us.\"");
		game->GAME_OVER = true;
	}
		

	framebuffer.drawImage(*Image::Get("data/ui/earth.tga"), *Image::Get("data/ui/earth_red.tga"), bufferSize - map_size - 1, bufferSize - map_size - 1, timeLeft);

	if (!this->show) {
		this->renderText(framebuffer);
		return;
	}

	Character* chtr = world->mainCharacter;
	int chtrx = (chtr->x - chtr->w / 2) - camera->x;
	int chtry = (chtr->y - chtr->h / 2) - camera->y - 13;

	if(World::instance->hasLifeStone)
		framebuffer.drawImage(*Image::Get("data/hallows/lifestone.tga"), chtrx - 2, chtry, 12, 10);
	if (World::instance->hasPowerWand)
		framebuffer.drawImage(*Image::Get("data/hallows/wand.tga"), chtrx + 10, chtry, 12, 10);
	if (World::instance->hasInviCloak)
		framebuffer.drawImage(*Image::Get("data/hallows/cloak.tga"), chtrx + 22, chtry, 12, 10);
	
	int text_x = 10;
	int box_x = 3;

	int gui_x = 0;
	int gui_y = 0;
	int gui_width = bufferSize - gui_x * 2;
	int gui_height = 10 * 4; 
	
	textIndex = 2;
	boxIndex = 2;

	// GUI -> last thing to render
	framebuffer.drawRectangle(gui_x, gui_y, gui_width, gui_height, Color(0x444, 180), true);
	framebuffer.drawText("DEBUG [1]", text_x, textIndex, font);
	framebuffer.drawRectangle(box_x, 3, 4, 4, game->debug ? Color::GREEN : Color::RED, true);
	framebuffer.drawText("RAIN [2]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 11, 4, 4, World::instance->dayCondition == RAINY ? Color::GREEN : Color::RED, true);
	framebuffer.drawText("TIME [3]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 19, 4, 4, world->dayTime == NIGHT ? Color::BLUE : world->dayTime == DAY ? Color::YELLOW : Color:: GREEN, true);

	framebuffer.drawText("FPS: " + std::to_string(game->fps), box_x, textIndex += 8, font);

	text_x += 70;
	box_x += 70;
	textIndex = 2;

	framebuffer.drawText("MASK [4]", text_x, textIndex, font);
	framebuffer.drawRectangle(box_x, 3, 4, 4, game->maskActive ? Color::GREEN : Color::RED, true); 
	framebuffer.drawText("SEE MASK [5]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 11, 4, 4, game->maskView ? Color::GREEN : Color::RED, true);
	framebuffer.drawText("ALPHA [6]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 19, 4, 4, game->alphaMask ? Color::GREEN : Color::RED, true);

	text_x += 85;
	box_x += 85;
	textIndex = 2;

	framebuffer.drawText("HAS WAND [7]", text_x, textIndex, font);
	framebuffer.drawRectangle(box_x, 3, 4, 4, world->hasPowerWand ? Color::GREEN : Color::RED, true);
	framebuffer.drawText("HAS STONE [8]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 11, 4, 4, world->hasLifeStone ? Color::GREEN : Color::RED, true);
	framebuffer.drawText("HAS CLOAK [9]", text_x, textIndex += 8, font);
	framebuffer.drawRectangle(box_x, 19, 4, 4, world->hasInviCloak ? Color::GREEN : Color::RED, true);
	
	

	this->renderText(framebuffer);

	if (Game::instance->userInput)
		this->renderPrompt(framebuffer);
}

void GUI::updateText()
{
	Game * game = Game::instance;

	if (game->textLines.size() > 1) {
		game->textLines.erase(game->textLines.begin(), game->textLines.begin() + 2);
	}
	else if (game->textLines.size() == 1) {
		game->textLines.erase(game->textLines.begin());
	}

	if (!game->textLines.size())
		GUI::instance->writing = false;
}

void GUI::renderText(Image &framebuffer)
{
	if (Game::instance->textLines.size() == 0)
		return;

	int bufferSize = Game::instance->bufferSize;

	int bg_height = bufferSize / 4;
	framebuffer.drawRectangle(0, bufferSize - bg_height, bufferSize, bg_height, Color(0x000, 220), true);

	// hacer un for y renderizar todo el texto q haya en el array
	int index = 3;
	for (int i = 0; i < min(2, Game::instance->textLines.size()); i++)
	{
		std::string line = Game::instance->textLines.at(i);
		int y = bufferSize - bg_height + 3 + index;
		framebuffer.drawText(line, 8, y, *Image::Get("data/fonts/bitmap-font-white.tga"));
		index += 10;
	}

	framebuffer.drawText("Press z", bufferSize - 46, bufferSize - 12, *Image::Get("data/fonts/bitmap-font-white.tga"));
}

void GUI::startCinema()
{
	Game::instance->START = false;
}

void GUI::stopCinema()
{
	Game::instance->START = true;
}

void GUI::renderPrompt(Image &framebuffer)
{
	
}