#include "utils.h"
#include "ui.h"
#include "camera.h"
#include "world.h"
#include "character.h"
#include "particle.h"
#include "synth.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

World* World::instance = NULL;

std::vector<Vector2> mortifagos;
std::vector<Vector2> survivors;

void renderMap(World* world, Image * framebuffer);
void renderAlphaMap(World* world, Image * framebuffer);
bool compare(Character *c1, Character *c2);

void printCharacters(std::vector<Character*> v)
{
	for (auto x : v)
		std::cout << x->name << "\n";
}

World::World(int dayTime, int dayCondition)
{
	this->camera = camera;
	
	this->map = *Image::Get("data/maps/map.tga");
	this->map_night = *Image::Get("data/maps/map_night.tga");
	this->map_alpha = *Image::Get("data/maps/map_alpha.tga");
	this->map_alpha_night = *Image::Get("data/maps/map_alpha_night.tga");
	this->map_mask = *Image::Get("data/maps/map_mask.tga");

	this->dayTime = dayTime;
	this->dayCondition = dayCondition;

	ttl_death = 80;
	timeDeath = ttl_death;
	invoked = false;
}

World::~World()
{

}

void World::begin()
{
	Game* game = Game::instance;
	std::string ch_name = "data/characters/med.tga";

	if (game->character_selected == 1)
		ch_name = "data/characters/little.tga";
	if (game->character_selected == 2)
		ch_name = "data/characters/big.tga";

	mainCharacter = new Character(ch_name.c_str(), 32, 32, 4, 3, 2, 1);
	mainCharacter->setPosition(20, 800);
	mainCharacter->setTileOrder(3, 0, 2, 1);
	mainCharacter->setTransition(5);
	this->characters.push_back(mainCharacter);

	mortifagos.push_back(Vector2(860, 860)); mortifagos.push_back(Vector2(600, 826));
	mortifagos.push_back(Vector2(274, 1277)); mortifagos.push_back(Vector2(1355, 756));
	mortifagos.push_back(Vector2(785, 269)); mortifagos.push_back(Vector2(1452, 206));
	mortifagos.push_back(Vector2(944, 104)); mortifagos.push_back(Vector2(872, 800));
	mortifagos.push_back(Vector2(660, 525)); mortifagos.push_back(Vector2(1450, 327));
	
	// los que rodean
	mortifagos.push_back(Vector2(1355, 756)); mortifagos.push_back(Vector2(1360, 800));
	mortifagos.push_back(Vector2(1400, 830)); mortifagos.push_back(Vector2(1450, 815));
	mortifagos.push_back(Vector2(1438, 768)); mortifagos.push_back(Vector2(1400, 756));

	survivors.push_back(Vector2(1400, 1200)); // mala -> arbol
	survivors.push_back(Vector2(800, 1215)); // se salva con el barco
	survivors.push_back(Vector2(90, 1422)); // barco
	survivors.push_back(Vector2(150, 170)); // padre castillo -> te da la capa
	survivors.push_back(Vector2(1407, 785)); // rodeado -> llevarlo a casa

											 // create inquis
	for (int i = 0; i < mortifagos.size(); i++)
	{
		Vector2 pos = mortifagos.at(i);
		std::string name = "data/characters/mortis.tga";
		Character * newInqui = new Character(name.c_str(), 32, 32, 4, 3, 0, 1);
		newInqui->setPosition(pos.x, pos.y);
		newInqui->setTileOrder(3, 0, 2, 1);
		newInqui->setTransition(5);
		newInqui->setState(IA);
		this->characters.push_back(newInqui);
	}

	// create survivors
	for (int i = 0; i < survivors.size(); i++)
	{
		Vector2 pos = survivors.at(i);
		std::string index = std::to_string(i + 1);
		std::string name = "data/characters/survivor" + index + ".tga";
		Character *survivor = new Character(name.c_str(), 32, 32, 4, 3, 0, 1);
		survivor->setPosition(pos.x, pos.y);
		survivor->setTileOrder(3, 0, 2, 1);
		survivor->setTransition(10);
		survivor->setState(NPC);
		this->characters.push_back(survivor);
		survivors_only.push_back(survivor);
	}

	// create camera
	camera = new Camera(0, 0);
	camera->setCharacter(mainCharacter);

	Synth::SamplePlayback* rain_sample = Game::instance->synth.playSample("data/music/rain.wav", 1, true);
	rain_sample->stop();
	
	dayTime = game->time_selected;
	dayCondition = game->raining ? RAINY : CLEAR;

	this->rainSystem = new RainParticleSystem(10000, 1600);
	
	camera->lookAt(mainCharacter->x, mainCharacter->y);

	if (this->dayCondition == RAINY)
		game->synth.samples_play["data/music/rain.wav"]->play();

	if (1)
	{
		std::string lines = std::string(
			"October 31st. I've been in a deep sleep\n") +
			"for almost 1 month, and the worst part,\n" +
			"I don't remember what happened.\n" +
			"\n" +
			"First thing I did after my late\n" +
			"awakening was reading the newspaper.\n" +
			"It helped:\n" +
			"\n" +
			"\"Uncontrolled expansion of deadly\n" +
			"illness strucking terror into civilians.\n" +
			"People under 20 are the most affected.\n" +
			"More than 126 youngsters have already\n" +
			"dead and this number is expected to\n" +
			"increase notably the next days.\n" +
			"Synthoms can differ depending on the\n" +
			"subject.\n" +
			"...\n" +
			"\n" +
			"\"There is no cure at this moment. It is\n" +
			"time to panic?\"\n" +
			"Numerous individuals have taken to the\n" +
			"street using violence against everyone.\n" +
			"Some people think they are not even from\n" +
			"this planet.\n" +
			"Others rely on the possibility of\n" +
			"shameless people using this situation to\n" +
			"get things worse.\n" +
			"\n" +
			"Finally, the freak ones link this\n" +
			"situation with an old legend.\n" +

			"They swear the strange people, called\n" +
			"\"HEARTLESS\", are here protecting\n" +
			"magical objects named...\n" +
			"\"The Time Hallows\".\n" +
			"\n" +
			"\n" +
			"[Explore the town looking for anything\n" +
			"can help. Good luck.]\n";

		game->sendText(lines);
		game->must_start = true;
	}
}

void World::render(Image * framebuffer)
{
	Game * game = Game::instance;
	int bufferSize = game->bufferSize;

	renderMap(this, framebuffer);

	// render boat before pj
	if (this->canUseBoat)
		this->renderBoat(framebuffer);

	if (game->maskView)
		framebuffer->drawImage(this->map_mask, -this->camera->x, -this->camera->y);

	// draw characters
	// sort first
	std::sort(this->characters.begin(), this->characters.end(), compare);

	for (int i = 0; i < this->characters.size(); i++)
		this->characters.at(i)->render(*framebuffer);

	// draw alpha map -> after character to simulate 3d
	if (game->alphaMask && !game->maskView)
		renderAlphaMap(this, framebuffer);

	for (int i = 0; i < this->characters.size(); i++)
		this->characters.at(i)->renderEffects(*framebuffer);

	// render cave dark mask 
	if (this->mainCharacter->isAtCave)
		framebuffer->drawImage(*Image::Get("data/maps/dark.tga"), this->mainCharacter->x - this->camera->x - 400, this->mainCharacter->y - this->camera->y - 400);

	// draw rain -> after alpha map
	if(this->dayCondition == RAINY && !this->mainCharacter->isAtCave)
		this->rainSystem->render(framebuffer);
}

void World::update(double seconds_elapsed)
{
	Game * game = Game::instance;

	if (this->s5saved && !GUI::instance->writing)
	{
		this->s5saved = false;
		this->survivors_only.at(4)->visible = false;
	}

	if (this->s2saved && !GUI::instance->writing)
	{
		this->s2saved = false;
		this->survivors_only.at(1)->visible = false;
	}

	if(!GUI::instance->writing)
		GUI::instance->stopCinema();

	for (int i = 0; i < this->characters.size(); i++)
		this->characters.at(i)->update(seconds_elapsed);

	camera->update(seconds_elapsed);

	bool rescued = false;

	for (int i = 0; i < 5; i++)
		rescued = this->survivors_rescued[i];

	// ALL CHARACTERS RESCUED
	if (!this->castleHasShaked && rescued &&
		(this->hasInviCloak && this->hasLifeStone && this->hasPowerWand))
	{
		this->castleHasShaked = true;
		game->synth.playSample("data/music/tremor.wav", 1, false);
		this->camera->shake(12, 200);

		game->sendText("????? WHAT IS HAPPENING?");
		game->sendText("");

		// OJO SI ESTAS YA EN LA CUEVA
		if (this->mainCharacter->isAtCave)
		{
			game->sendText("It's impossible!! It's happening");
			game->sendText("right here, at the CAVE!");
			game->sendText("... I should leave as soon");
			game->sendText("as possible.");
		}
		else {
			game->sendText("I think it came from the CAVE");
			game->sendText("");
			game->sendText("It's a good thing I'm not there.");
			game->sendText("");
		}

		game->sendTextPause();
		game->sendText("I need answers. I'm sure the");
		game->sendText("old man at the tent knows more.");
		game->sendText("I need to go there.");
		game->sendText("");

		int old_size = mortifagos.size();

		// add new bichus in cave
		mortifagos.push_back(Vector2(340, 152)); mortifagos.push_back(Vector2(375, 152));
		mortifagos.push_back(Vector2(424, 152)); mortifagos.push_back(Vector2(450, 152));
		mortifagos.push_back(Vector2(412, 186)); mortifagos.push_back(Vector2(364, 180)); 
		mortifagos.push_back(Vector2(308, 178));

		mortifagos.push_back(Vector2(388, 212)); mortifagos.push_back(Vector2(436, 216));
		mortifagos.push_back(Vector2(268, 204)); mortifagos.push_back(Vector2(322, 218));
		mortifagos.push_back(Vector2(368, 244));

		for (int i = old_size; i < mortifagos.size(); i++)
		{
			Vector2 pos = mortifagos.at(i);
			std::string name = "data/characters/mortis.tga";
			Character * newInqui = new Character(name.c_str(), 32, 32, 4, 3, 0, 1);
			newInqui->setPosition(pos.x, pos.y);
			newInqui->setTileOrder(3, 0, 2, 1);
			newInqui->setTransition(5);
			newInqui->setState(IA);
			newInqui->isAtCave = true;
			this->characters.push_back(newInqui);
		}

		// go to tent only if not in cave
		if(!this->mainCharacter->isAtCave)
			World::instance->mainCharacter->setPosition(608, 1078, true);
	}
}

void World::renderBoat(Image * framebuffer)
{
	if (this->mainCharacter->isAtCave)
		return;

	int x = (610) - World::instance->camera->x;
	int y = (1375) - World::instance->camera->y;

	if (this->mainCharacter->isAtIsland) {
		x = (840 - 60) - World::instance->camera->x;
		y = (1339 - 25) - World::instance->camera->y;
	}
	
	if (this->mainCharacter->travelling)
	{
		x = (this->mainCharacter->x - 60) - World::instance->camera->x;
		y = (this->mainCharacter->y - 25) - World::instance->camera->y;
	}

	if (this->dayTime == AUTO && !this->mainCharacter->isAtCave)
	{
		float factor = Game::instance->gameTime / 60.0;
		if (factor > 0.5)
			factor = 1.0 - factor;
		factor /= 0.5;
		framebuffer->drawImage(*Image::Get("data/vehicles/boat.tga"), *Image::Get("data/vehicles/boat_night.tga"), x, y, Area(0, 0, 128, 60), factor);
	}
	else if (this->dayTime == NIGHT && !this->mainCharacter->isAtCave)
		framebuffer->drawImage(*Image::Get("data/vehicles/boat_night.tga"), x, y, Area(0, 0, 128, 60));
	else
		framebuffer->drawImage(*Image::Get("data/vehicles/boat.tga"), x, y, Area(0, 0, 128, 60));
}

void World::toggleNight()
{
	dayTime++;
	if (dayTime > 2)
		dayTime = 0;

	current_map = dayTime == DAY ? &map : &map_night;
	current_map_alpha = dayTime == DAY ? &map_alpha : &map_alpha_night;	
}

void World::toggleRain()
{
	dayCondition = (dayCondition == CLEAR) ? RAINY : CLEAR;

	if (dayCondition != RAINY)
		Game::instance->synth.samples_play["data/music/rain.wav"]->stop();
	else
		Game::instance->synth.samples_play["data/music/rain.wav"]->play();
}

bool compare(Character *c1, Character *c2)
{
	Character a = *c1;
	Character b = *c2;

	return (a.y < b.y);
}

void renderMap(World* world, Image * framebuffer)
{
	Game * game = Game::instance;

	if (world->dayTime == AUTO)
	{
		// draw map getting gameTime factor
		float f = game->gameTime / 60.0;
		if (f > 0.5)
			f = 1.0 - f;
		f /= 0.5;

		framebuffer->drawImage(world->map, world->map_night, -world->camera->x, -world->camera->y, f);
	}
	else
		framebuffer->drawImage(*world->current_map, -world->camera->x, -world->camera->y);

}

void renderAlphaMap(World* world, Image * framebuffer)
{
	Game * game = Game::instance;

	if (world->dayTime == AUTO)
	{
		// draw map getting gameTime factor
		float f = game->gameTime / 60.0;
		if (f > 0.5)
			f = 1.0 - f;
		f /= 0.5;

		framebuffer->drawImage(world->map_alpha, world->map_alpha_night, -world->camera->x, -world->camera->y, f);
	}
	else
		framebuffer->drawImage(*world->current_map_alpha, -world->camera->x, -world->camera->y);

}