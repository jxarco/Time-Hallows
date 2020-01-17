#include "utils.h"
#include "character.h"
#include "image.h"
#include "game.h"
#include "world.h"
#include "ui.h"
#include "input.h"

#include <cmath>
#include <algorithm>
#include <vector>

enum { FREE = 0, BLOCK, INTERACTION };
enum { UP = 0, DOWN, LEFT, RIGHT };

int ups = 0;
int defeated = 0;

// important locations to go
#define HOME Vector2(48, 790)
#define PORT Vector2(590, 1386)
#define BOAT Vector2(670, 1395)
#define ISLAND Vector2(850, 1300)
#define ISLAND_BOAT Vector2(840, 1340)
#define CAVE Vector2(320, 770)
#define OUT_CAVE Vector2(960, 125)

#define IA_ATTACK_WAIT_TIME 30

void dialogS1();
void dialogS2();
void dialogS3();
bool dialogS4(); // depending on the possesion of the lifestone
void dialogS5();

void updateIA(Character* chtr, double seconds_elapsed);
void updatePlayer(Character* chtr, double seconds_elapsed);
void updateNPC(Character* chtr, double seconds_elapsed);

// PLAYER
int ttl = 40;
int timeSpell = ttl;
bool attacking = false;

int ttl_heal = 30;
int timeHeal = ttl;

bool canInvoke = false;
bool hasInvoked = false;

Character::Character()
{

}

Character::~Character()
{

}

Character::Character(const char* filename, int w, int h, int rows, int cols, int beginr, int beginc)
{
	this->w = w;
	this->h = h;
	
	this->canInteract = false;
	this->hasToMove = false;
	this->isAtCave = false;

	this->rows = rows;
	this->cols = cols;
	this->currentRow = beginr;
	this->currentCol = beginc;
	this->transition = 5;

	this->timer = 0;
	this->waitTime = IA_ATTACK_WAIT_TIME;

	this->lifePoints = 100;
	this->state = PLAYER;

	this->image = *Image::Get(filename);
	this->font = *Image::Get("data/fonts/bitmap-font-white.tga");
	this->name = std::string(filename);
}

void Character::setTileOrder(int up, int down, int right, int left)
{
	this->up = up;
	this->down = down;
	this->right = right;
	this->left = left;
}

void Character::setState(int state)
{
	this->state = state;

	if (state == IA) {
		IAttl = 30;
		IAtimeSpell = IAttl;
		timer = random(200);
	}
}

void Character::render(Image& frameBuffer)
{
	if (!this->visible)
		return;

	Game * game = Game::instance;
	World * world = World::instance;

	// do not render ia and npcs if main is at the cave
	if (this->state == IA || this->state == NPC)
	{
		if (world->mainCharacter->isAtCave && !this->isAtCave)
			return;
		else if (!world->mainCharacter->isAtCave && this->isAtCave)
			return;
	}

	int w = this->w;
	int h = this->h;
	int x0 = this->currentCol * w;
	int y0 = this->currentRow * h;

	Color debug_rect = this->interactionColor;

	int x = (this->x - this->w / 2) - world->camera->x;
	int y = (this->y - this->h / 2) - world->camera->y;
	int corner_margin = this->w / 4;

	if (game->debug) {

		if (isMakingDamage)
			debug_rect = Color::RED;

		frameBuffer.drawRectangle(x, y, w, h, debug_rect, false);

		frameBuffer.drawLine(x, y + corner_margin, x + corner_margin, y, debug_rect);
		frameBuffer.drawLine(x + this->h - corner_margin - 1, y, x + this->w - 1, y + corner_margin, debug_rect);

		if (y > frameBuffer.height/2) {
			frameBuffer.drawText(std::string("x ") + std::to_string(this->x), x + 6, y - 24, this->font);
			frameBuffer.drawText(std::string("y ") + std::to_string(this->y), x + 6, y - 16, this->font);
		}
		else
		{
			frameBuffer.drawText(std::string("x ") + std::to_string(this->x), x+ 6, y + this->h + 2, this->font);
			frameBuffer.drawText(std::string("y ") + std::to_string(this->y), x+ 6, y + this->h + 10, this->font);
		}
	}
	
	std::string name = std::string(this->name);
	std::string name_night = "data/characters/" + name.substr(16, name.size() - 20) + "_night.tga";

	if (world->dayTime == AUTO && !world->mainCharacter->isAtCave)
	{
		float factor = Game::instance->gameTime / 60.0;
		if (factor > 0.5)
			factor = 1.0 - factor;
		factor /= 0.5;
		frameBuffer.drawImage(this->image, *Image::Get(name_night.c_str()), x, y, Area(x0, y0, w, h), factor);
	}
	else if(world->dayTime == NIGHT && !world->mainCharacter->isAtCave)
		frameBuffer.drawImage(*Image::Get(name_night.c_str()), x, y, Area(x0, y0, w, h));
	else
		frameBuffer.drawImage(this->image, x, y, Area(x0, y0, w, h));
}

void Character::renderEffects(Image& frameBuffer)
{
	Game * game = Game::instance;
	World * world = World::instance;

	if (state == NPC)
		return;

	if (state == IA)
	{
		if (IAtimeSpell < IAttl)
		{
			float f = float(IAtimeSpell) / IAttl;
			unsigned frame = f * 25.0;
			int tx = frame % 5;
			int ty = frame / 5.0;

			int xdark = (this->x) - world->camera->x - 24;
			int ydark = (this->y) - world->camera->y - 24;

			frameBuffer.drawImage(*Image::Get("data/effects/darkness.tga"), xdark, ydark, Area(tx * 48, ty * 48, 48, 48));
		}

		return;
	}

	if (timeSpell < ttl)
	{
		float f = float(timeSpell) / ttl;
		unsigned frame = f * 25.0;
		int tx = frame % 5;
		int ty = frame / 5.0;

		int xspell = (this->x) - world->camera->x - 64;
		int yspell = (this->y) - world->camera->y - 64;

		attacking = true;
		frameBuffer.drawImage(*Image::Get("data/effects/magic.tga"), xspell, yspell, Area(tx * 128, ty * 128, 128, 128));
	}
	else
		attacking = false;


	if (timeHeal < ttl_heal)
	{
		float f = float(timeHeal) / ttl_heal;
		unsigned frame = f * 25.0;
		int tx = frame % 5;
		int ty = frame / 5.0;

		int xheal = (this->x) - world->camera->x - 24;
		int yheal = (this->y) - world->camera->y - 24;

		frameBuffer.drawImage(*Image::Get("data/effects/heal.tga"), xheal, yheal, Area(tx * 48, ty * 48, 48, 48));
	}

	if (world->timeDeath < world->ttl_death)
	{
		float f = float(world->timeDeath) / world->ttl_death;
		unsigned frame = f * 25.0;
		int tx = frame % 5;
		int ty = frame / 5.0;

		int xdark = (this->x) - world->camera->x - 128;
		int ydark = (this->y) - world->camera->y - 128;

		frameBuffer.drawImage(*Image::Get("data/effects/invoke.tga"), xdark, ydark, Area(tx * 256, ty * 256, 256, 256));
	}

	// render also the triangle interaction
	if (this->canInteract)
	{
		int margin = 3;
		int xT = (this->x - 5) - world->camera->x;
		int yT = (this->y - this->h / 2 - 10 - margin) - world->camera->y;
		frameBuffer.drawImage(*Image::Get("data/ui/interaction.tga"), xT, yT, Area(0, 0, 10, 10));
	}

}

void Character::update(double seconds_elapsed)
{
	// update position in case it has to move
	if (!GUI::instance->writing && hasToMove)
	{
		setPosition(x1, y1);
		hasToMove = false;
	}

	// do not update ia and npcs if main is at the cave
	if (this->state == IA || this->state == NPC)
	{
		if (World::instance->mainCharacter->isAtCave && !this->isAtCave)
			return;
		else if (!World::instance->mainCharacter->isAtCave && this->isAtCave)
			return;
	}


	switch (state)
	{
	case PLAYER:
		updatePlayer(this, seconds_elapsed);
		break;
	case IA:
		updateIA(this, seconds_elapsed);
		break;
	case NPC:
		updateNPC(this, seconds_elapsed);
		break;
	}
}

bool Character::attack()
{
	if (this->state == IA)
	{
		if (waitTime > 0)
			return false;

		attackTimer++;

		if (attackTimer > maxAttackTimer) {
			attackTimer = 0;
			waitTime = IA_ATTACK_WAIT_TIME;
			Game::instance->pendingAttack = true;
			return true;
		}

		return false;
	}

	return false;
} 

void Character::interact()
{
	Color pixel = this->interactionColor;
	World * world = World::instance;
	Game * game = Game::instance;

	Game::instance->synth.playSample("data/music/action.wav", 1, false);

	if (!this->canInteract)
	{
		ups++;
		game->sendText("There's nothing to do here");
		return;
	}

	GUI::instance->startCinema();

	if (pixel.r == 127 && pixel.g == 0 && pixel.b == 127)
	{
		
		for (int i = 0; i < world->survivors_only.size(); i++)
		{
			Character * chtr = world->survivors_only.at(i);

			if (!chtr->visible)
				continue;

			if (chtr->inTalk)
			{
				bool rescued = true;

				switch (i)
				{
				case 0:
					dialogS1();
					break;
				case 1:
					dialogS2();
					this->gotCloak = true;
					break;
				case 2:
					dialogS3();
					break;
				case 3:
					rescued = dialogS4(); // do i have the lifestone?
					break;
				case 4:
					dialogS5();
					break;
				}

				world->survivors_rescued[i] = rescued;
				// end here -> only 1 in talk
				return;
			}
				
		}

	}
	if (pixel.r == 0 && pixel.g == 255 && pixel.b == 0)
	{
		if (world->hasLight)
		{
			game->sendText("One torch will be enough.");
		}
		else {
			game->sendText("Maybe I can use this fire to light");
			game->sendText("up a torch.");
			game->sendText("Don't burn yourself...");
			game->sendText("");
			game->sendText("[Torch achieved]");
			world->hasLight = true;
		}
	}
	else if (pixel.r == 0 && pixel.g == 0 && pixel.b == 255)
	{
		if (this->isAtCave)
		{
			this->setPosition(OUT_CAVE.x, OUT_CAVE.y);
			this->mustBeInCave = false;
			this->isAtCave = false;

			world->map = *Image::Get("data/maps/map.tga");
			world->map_night = *Image::Get("data/maps/map_night.tga");
			world->map_alpha = *Image::Get("data/maps/map_alpha.tga");
			world->map_alpha_night = *Image::Get("data/maps/map_alpha_night.tga");
			world->map_mask = *Image::Get("data/maps/map_mask.tga");
		}
		else {
			if (world->hasLight)
			{
				game->sendText("(uses torch)");
				game->sendText("");
				this->mustBeInCave = true;

			}
			else {
				game->sendText("Uhh... Too dark inside,");
				game->sendText("I need something to illuminate.");
			}
		}
		
	}
	else if (pixel.r == 255 && pixel.g == 127 && pixel.b == 255)
	{
		GUI::instance->stopCinema(); // no cinema here
		game->sendText("Good. I needed it.");
		game->sendText("");
		game->sendText("[Life points restored]");
		timeHeal = 0;
		game->synth.playSample("data/music/spell-1.wav", 1, false);
		this->lifePoints = this->maxLifePoints;
	}
	else if (pixel.r == 0 && pixel.g == 255 && pixel.b == 255)
	{
		// cementerio
		if (this->y < 1000)
		{
			if (World::instance->hasLifeStone)
			{
				game->sendText("Imagine finding another of");
				game->sendText("this hallows here... ");
			}
			else {
				game->sendText("(looking behind the tomb)");
				game->sendText("");
				game->sendText("...What's this?");
				game->sendText("");
				game->sendText("It seems to be a rare precious");
				game->sendText("stone. Reminds me of a famous one");
				game->sendText("which used to appear in old tales.");
				game->sendText("");
				game->sendText("This can be one of the things they");
				game->sendText("are protecting.");
				game->sendText("What they are?");
				game->sendText("");
				game->sendText("[Lifestone achieved]");
				this->gotStone = true;
			}
		}
		// estatua
		else {
			game->sendText("What a strange figure... I do");
			game->sendText("not remember seeing it before");
		}
	}
	else if (pixel.r == 255 && pixel.g == 127 && pixel.b == 0)
	{
		World* w = World::instance;

		// stats house
		if (this->y > 500)
		{
			game->sendText("Stats:");
			game->sendText("");
			game->sendText("# Deaths:    0");
			game->sendText("# Defeated:  0");
			game->sendText("# Ups:       " + std::to_string(ups));
			game->sendText("Playtime:    " + std::to_string(int(game->time / 60)) + " min");
		}
		// is the fountain: wand code
		else {
			game->sendText("Wait...");
			game->sendText("There is something engraved here.");
			game->sendText("\"temptation\"");
			game->sendText("");
			game->sendText("What it means?");
			game->sendText("I should discover it.");
			this->gotWandCode = true;
		}
	}
	else if (pixel.r == 255 && pixel.g == 0 && pixel.b == 255)
	{
		// is the wand interaction inside cave
		if (this->isAtCave)
		{
			GUI::instance->startCinema();
			game->sendText("I think something is hidden here.");
			game->sendText("");
			game->sendText("Looks like guys at the entry were");
			game->sendText("protecting it.");

			if (this->gotWandCode)
			{
				game->sendText("[introduces password]");
				game->sendText("");
				game->sendText("[mechanism opens]");
				game->sendText("");
				game->sendText("Cool. Let's see what's inside");
				game->sendText("this thing.");
				game->sendTextPause();
				game->sendText("A wand. It's a wand. Maybe");
				game->sendText("the old legend was true.");
				game->sendText("If I could find it, I'm sure");
				game->sendText("what this guys mission is.");
				game->sendText("I have to be careful from now");
				game->sendText("on.");
				game->sendText("[Power wand achieved]");
				game->sendText("[Press X to use it]");
				this->gotWand = true;
			}
			else {
				game->sendText("[tries to open the mechanism]");
				game->sendText("");
				game->sendText("Damn. It's sealed. Maybe I can");
				game->sendText("find the password somewhere.");
			}
		}
		// is the guy inside the tend
		else {

			if (world->castleHasShaked)
			{
				game->sendText("Sorry... Hello??");
				game->sendText("");
				game->sendTextPause();
				game->sendText("\"You are late... Be sure...");
				game->sendText("to listen carefully.");
				game->sendText("All this situation... the heartless,");
				game->sendText("the illness...is caused..");
				game->sendText("By me...");
				game->sendText("");
				game->sendText("And also... by you.");
				game->sendText("");
				game->sendText("(breathes and continues)");
				game->sendText("Yes... ME.. and YOU...");
				game->sendText("We are the sa.. ..erson");
				game->sendText("(coughs)");
				game->sendText("I'm from the... f.. from the");
				game->sendText("future. I'm 46 years older");
				game->sendText("than you.\"");
				game->sendText("");
				game->sendTextPause();
				game->sendText("I should be dreaming.");
				game->sendText("");
				game->sendTextPause();
				game->sendText("\"SHUT UP! (coughs again)");
				game->sendText("");
				game->sendText("Every..ody has to die.. one day,");
				game->sendText("but this ...llness, has bee.. create..");
				game->sendText("by us, in a period of t.. time");
				game->sendText("between you an.. me.");
				game->sendTextPause();
				game->sendText("\"You hav.. to make the ..llows d..");
				game->sendText("dissapear, to stop the lo.. loop.\"");
				game->sendTextPause();
				game->sendText("3 \"ME\"...");
				game->sendText("Ok.. How? Where?");
				game->sendTextPause();
				game->sendText("\"You .. have to create ..an");
				game->sendText("antidote from the Time Hallow..s.");
				game->sendText("I kn... you have them");
				game->sendText("I had them... also...\"");
				game->sendTextPause();
				game->sendText("(nods)");
				game->sendText("");
				game->sendTextPause();
				game->sendText("\"Inv.. invoke the pow...");
				game->sendText("power of the 3 ha..lls together");
				game->sendText("Once done, they .. will..");
				game->sendText("heal... everybody");
				game->sendText("and the hallows.. will no longer");
				game->sendText("..exist.");
				game->sendText("Go to the c... cave... and be");
				game->sendText("careful... They kno.. bout the");
				game->sendText("invi...bility cloak...\"");
				game->sendText("");
				game->sendTextPause();
				game->sendText("I'll do whatever I can.");
				game->sendText("");
				game->sendText("[press X to use hallows]");
				canInvoke = true;
			}
			else {
				game->sendText("(someone inside)");
				game->sendText("");
				game->sendText("\"The w...d is at the ...e");
				game->sendText("YOU ha.. to find it..\"");
				game->sendText("\"The ...\" [breathes]");
				game->sendText("");
				game->sendText("\"..the pas...d to unlock");
				game->sendText("is...\"");
				game->sendText("\"engraved at th.. stone... trop..");
				game->sendText("outside th.. cas..\"");
				game->sendTextPause();
				game->sendText("(other person inside)");
				game->sendText("");
				game->sendText("\"HEY!! YOU!! What are you");
				game->sendText("doing out there?");
				game->sendText("Get out inmediately!\"");
			}
		}
	}
	else if (pixel.r == 255 && pixel.g == 255 && pixel.b == 0)
	{
		game->sendText("(tries to open)");
		game->sendText("");
		game->sendText("Ok, it's closed.");
		game->sendText("");
		game->sendText("Hello!! Someone there?");
		game->sendText("");
		game->sendText("This rich people...");
	}
	else if (pixel.r == 0 && pixel.g == 127 && pixel.b == 255)
	{
		if (!world->canUseBoat) {

			game->sendText("I don't have a boat");
			game->sendText("Damn it");
			return;
		}

		if (this->x > PORT.x) { // boat guy
		
			game->sendText("(travelling to port)");
			this->travelling = true;
			this->x = ISLAND_BOAT.x;
			this->y = ISLAND_BOAT.y;
		}
		else
		{
			game->sendText("(travelling to island)");
			this->travelling = true;
			this->x = BOAT.x;
			this->y = BOAT.y;
		}
	}
}

void Character::damage()
{
	lifePoints -= 10;

	if (lifePoints <= 0)
		Game::instance->GAME_OVER = true;
}

bool Character::moveLeft(bool rotate)
{
	if (rotate)
	this->currentRow = this->left;

	if (this->timer > this->transition) {
		this->timer = 0;
		this->currentCol++;
	}

	if (this->currentCol > this->cols-1)
		this->currentCol = 0;

	bool colision = collides(LEFT);

	if (!colision || !rotate){
		this->x -= state == IA ? 1:2;
	}

	return colision;
}

bool Character::moveDown(bool rotate)
{
	if (rotate)
	this->currentRow = this->down;

	if (this->timer > this->transition) {
		this->timer = 0;
		this->currentCol++;
	}

	if (this->currentCol > this->cols - 1)
		this->currentCol = 0;

	bool colision = collides(DOWN);

	if (!colision || !rotate) {
		this->y += state == IA ? 1 : 2;
	}

	return colision;
}

bool Character::moveRight(bool rotate)
{
	if (rotate)
	this->currentRow = this->right;

	if (this->timer > this->transition) {
		this->timer = 0;
		this->currentCol++;
		// Game::instance->synth.playSample("data/music/footsteps.wav", 1, false);
	}

	if (this->currentCol > this->cols - 1)
		this->currentCol = 0;

	bool colision = collides(RIGHT);

	if (!colision || !rotate) {
		this->x += state == IA ? 1 : 2;
	}

	return colision;
}

bool Character::moveUp(bool rotate)
{
	if(rotate)
	this->currentRow = this->up;

	if (this->timer > this->transition) {
		this->timer = 0;
		this->currentCol++;
	}

	if (this->currentCol > this->cols - 1)
		this->currentCol = 0;

	bool colision = collides(UP);

	if (!colision || !rotate) {
		this->y -= state == IA ? 1 : 2;
	}

	return colision;
}

bool Character::collides(int direction)
{
	int x = (this->x - this->w / 2);
	int y = (this->y - this->h / 2);

	int corner_margin = this->w / 8;

	sColision value;

	if (direction == DOWN)
	{
		int cy = y + this->h + 2;

		for (int i = corner_margin; i <= this->w - corner_margin; i++)
		{
			int cx = x + i;
			sColision event = this->checkColision(cx, cy);

			if (event.interaction) {

				if (value.interaction)
					continue;

				value.interaction = true;
				value.colision = true;
				value.color = event.color;
				continue;
			}

			if (event.colision) {
				value.colision = event.colision;

				if (!value.interaction)
					value.color = event.color;
			}
		}
	}

	else if (direction == UP)
	{
		int cy = y - 2;

		for (int i = corner_margin; i <= this->w - corner_margin; i++)
		{
			int cx =  x + i;
			sColision event = this->checkColision(cx, cy);

			if (event.interaction) {

				if (value.interaction)
					continue;

				value.interaction = true;
				value.colision = true;
				value.color = event.color;
				continue;
			}

			if (event.colision) {
				value.colision = event.colision;
				
				if(!value.interaction)
					value.color = event.color;
			}
			
		}
	}
	else if (direction == LEFT)
	{
		int cx =  x - 2;

		for (int i = corner_margin; i <= this->h; i++)
		{
			int cy =  y + i;
			sColision event = this->checkColision(cx, cy);

			if (event.interaction) {

				if (value.interaction)
					continue;

				value.interaction = true;
				value.colision = true;
				value.color = event.color;
				continue;
			}

			if (event.colision) {
				value.colision = event.colision;

				if (!value.interaction)
					value.color = event.color;
			}
		}
	}
	else if (direction == RIGHT)
	{
		int cx =  x + this->w + 2;

		for (int i = corner_margin; i <= this->h; i++)
		{
			int cy =  y + i;
			sColision event = this->checkColision(cx, cy);
			
			if (event.interaction) {

				if (value.interaction)
					continue;

				value.interaction = true;
				value.colision = true;
				value.color = event.color;
				continue;
			}

			if (event.colision) {
				value.colision = event.colision;

				if (!value.interaction)
					value.color = event.color;
			}
		}
	}

	this->canInteract = value.interaction;
	this->interactionColor = value.color;

	if (value.colision)
		return true;
	return false;
}

sColision Character::checkColision(int cx, int cy)
{
	Game * game = Game::instance;
	World * world = World::instance;
	Image* mask = &world->map_mask;
	
	sColision value;

	if (!Game::instance->maskActive)
		return value;
	
	if (cx > world->map.width || cy > world->map.height || cx < 0 || cy < 0) {
		value.colision = true;
		return value;
	}

	Color pixel = mask->getPixelRef(cx, cy);
	
	// red
	if (pixel.r == 255 && pixel.g == 0 && pixel.b == 0) {

		value.color = pixel;
		value.interaction = false;
		value.colision = true;
		return value;
	}
			
	if ((pixel.r == 0 && pixel.g == 0 && pixel.b == 255) || 
		(pixel.r == 255 && pixel.g == 127 && pixel.b == 255) ||
		(pixel.r == 255 && pixel.g == 255 && pixel.b == 0) ||
		(pixel.r == 255 && pixel.g == 127 && pixel.b == 0) ||
		(pixel.r == 0 && pixel.g == 255 && pixel.b == 0) ||
		(pixel.r == 255 && pixel.g == 0 && pixel.b == 255) ||
		(pixel.r == 0 && pixel.g == 255 && pixel.b == 255) ||
		(pixel.r == 0 && pixel.g == 127 && pixel.b == 255)) {
		
		value.color = pixel;
		value.interaction = true;
		value.colision = true;
		return value;
	}
	
	return value;
}

void updatePlayer(Character* chtr, double seconds_elapsed)
{
	World* world = World::instance;
	Game* game = Game::instance;

	timeHeal++;
	timeSpell++;

	if (!world->invoked && hasInvoked)
	{
		world->invoked = true;
		chtr->mustBeInCave = false;
		chtr->isAtCave = false;
		world->map = *Image::Get("data/maps/map.tga");
		world->map_night = *Image::Get("data/maps/map_night.tga");
		world->map_alpha = *Image::Get("data/maps/map_alpha.tga");
		world->map_alpha_night = *Image::Get("data/maps/map_alpha_night.tga");
		world->map_mask = *Image::Get("data/maps/map_mask.tga");

		// remove invisibility
		std::string name = std::string(chtr->name);
		std::string nameClean = name.substr(16, name.size() - 20);

		std::string new_name = "data/characters/" + nameClean + ".tga";
		std::cout << new_name;
		chtr->image = *Image::Get(new_name.c_str());

		World::instance->mainCharacter->setPosition(HOME.x, HOME.y);
	}

	if (world->invoked && !game->GAME_END)
	{
		std::string lines = std::string(
			"October 31st. I've been in a deep sleep\n") +
			"for almost 1 month, and the worst part,\n" +
			"I don't remember what happened.\n";

		game->sendText(lines);
		game->GAME_END = true;
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_X) && world->hasPowerWand)
	{
		Vector2 ori(chtr->x, chtr->y);
		Vector2 dest(400, 100);

		float distance = ori.distance(dest);

		// death invocation
		if (!world->invoked && canInvoke && distance < 3 && chtr->isAtCave) {

			world->hasLifeStone = false;
			world->hasInviCloak = false;
			world->hasPowerWand = false;
			world->timeDeath = 0;
			game->synth.playSample("data/music/invoke.wav", 1, false);
			GUI::instance->startCinema();

			game->sendText("(assembles the 3 hallows)");
			game->sendText("");
			game->sendTextPause();
			game->sendText("Nothing seems to be changed.");
			game->sendText("");
			game->sendText("These hallows don't exist");
			game->sendText("anymore. I hope it worked.");
			game->sendTextPause();

			game->sendText("(storyteller)");
			game->sendText("All was supposed to be a time");
			game->sendText("loop. Every time the hallows");
			game->sendText("were found, \"he\" was returning to");
			game->sendText("the past in form of corrupted");
			game->sendText("death, killing every young person");
			game->sendText("to begin the human extinction.");
			game->sendText("");
			game->sendText("With the hallows disolved to get");
			game->sendText("the antidote, the loop will not");
			game->sendText("be repeated anymore.");
			game->sendText("");
			game->sendText("Or maybe yes...");

			hasInvoked = true;
		}
		else {
			game->synth.playSample("data/music/spell-2.wav", 1.0, false);
			world->camera->shake(4, 35);
			timeSpell = 0;
		}
		
	}

	if (chtr->travelling)
	{
		Vector2 ori(chtr->x, chtr->y);
		Vector2 dest(ISLAND_BOAT.x, ISLAND_BOAT.y);

		if (chtr->isAtIsland) {
			dest = Vector2(BOAT.x, BOAT.y);
		}

		float distance = ori.distance(dest);

		if (distance > 1) {

			if (chtr->x < dest.x) chtr->moveRight(false);
			if (chtr->x > dest.x) chtr->moveLeft(false);
			if (chtr->y < dest.y) chtr->moveDown(false);
			if (chtr->y > dest.y) chtr->moveUp(false);
		}
		else {
			chtr->travelling = false;
			if (chtr->isAtIsland) {
				chtr->x = PORT.x;
				chtr->y = PORT.y;
				chtr->isAtIsland = false;
			}

			else
				chtr->isAtIsland = true;
		}

		return;
	}
	
	// update for tiles if not travelling
	chtr->timer++;

	// update hallows
	if (chtr->gotCloak && !GUI::instance->writing && !world->hasInviCloak) {

		std::string name = std::string(chtr->name);
		std::string nameClean = name.substr(16, name.size() - 20);

		std::string new_name = "data/characters/" + nameClean + "_inv.tga";
		chtr->image = *Image::Get(new_name.c_str());

		world->hasInviCloak = true;
	}
	if (chtr->gotWand && !GUI::instance->writing && !world->hasPowerWand) {
		world->hasPowerWand = true;
	}

	if (chtr->gotStone && !GUI::instance->writing && !world->hasLifeStone) {
		world->hasLifeStone = true;
	}
	if (chtr->mustBeInCave && !GUI::instance->writing && !chtr->isAtCave) {

		if (canInvoke)
		{
			world->map = *Image::Get("data/maps/cave_2.tga");
			world->map_night = *Image::Get("data/maps/cave_2.tga");
			world->map_mask = *Image::Get("data/maps/cave_mask_2.tga");
			chtr->isAtCave = true;
			chtr->setPosition(CAVE.x, CAVE.y);
		}
		else {
			world->map = *Image::Get("data/maps/cave.tga");
			world->map_night = *Image::Get("data/maps/cave.tga");
			world->map_mask = *Image::Get("data/maps/cave_mask.tga");
		}

		
		world->map_alpha = *Image::Get("data/maps/cave_alpha.tga");
		world->map_alpha_night = *Image::Get("data/maps/cave_alpha.tga");
		chtr->isAtCave = true;
		chtr->setPosition(CAVE.x, CAVE.y);
	}

	// interaction with other people in the game
	Vector2 mypos(chtr->x, chtr->y);

	for (int i = 0; i < world->survivors_only.size(); i++)
	{
		Character * survivor = world->survivors_only.at(i);

		if (!survivor->visible || (!survivor->isAtCave && chtr->isAtCave))
			continue;

		Vector2 spos(survivor->x, survivor->y);
		Vector2 c = spos - mypos;
		float distance = mypos.distance(spos);

		if (distance < 35) {
			
			chtr->interactionColor = Color(127, 0, 127);
			chtr->canInteract = true;

			survivor->inTalk = true;

			if (abs(c.x) < abs(c.y))
			{
				survivor->currentRow = c.y > 0 ? survivor->up : survivor->down;
			}
			else
				survivor->currentRow = c.x > 0 ? survivor->left : survivor->right;
		}
		else
			survivor->inTalk = false;
	}

	if ((chtr->x > 1328 && chtr->x < 1530) &&
		(chtr->y > 650 && chtr->y < 900))
	{
		Character * survivor4 = World::instance->survivors_only.at(4);
		world->camera->lookAt(survivor4->x + 25, survivor4->y - 25, true);

		if (!world->awareOfGroup) {
			if (world->hasInviCloak)
			{
				game->sendText("It's time to use the cloak.");
				game->sendText("Thank you, blind man.");
			}
			else {
				game->sendText("So many of them right there... I really");
				game->sendText("need something to rescue him.");
			}
			world->awareOfGroup = true;
		}
	}
}

void updateIA(Character* chtr, double seconds_elapsed)
{
	World* world = World::instance;

	if (!chtr->visible)
		return;

	chtr->IAtimeSpell++;
	chtr->waitTime--;
	chtr->timer++;

	if (chtr->waitTime < 0)
		chtr->waitTime = 0;

	Character * mainChtr = World::instance->mainCharacter;
	Vector2 a(chtr->x, chtr->y);
	Vector2 b(mainChtr->x, mainChtr->y);
	Vector2 c = a - b;
	float distance = a.distance(b);

	if (distance < 100 && (!World::instance->hasInviCloak || canInvoke))
	{
		int x0 = mainChtr->x - 25;
		int x1 = mainChtr->x + 25;
		int y0 = mainChtr->y - 10;
		int y1 = mainChtr->y + 10;

		if (chtr->x < x0) chtr->moveRight();
		if (chtr->x > x1) chtr->moveLeft();
		if (chtr->y < y0) chtr->moveDown();
		if (chtr->y > y1) chtr->moveUp();

		if (abs(c.x) < abs(c.y))
		{
			chtr->currentRow = c.y > 0 ? chtr->up : chtr->down;
		}
		else
			chtr->currentRow = c.x > 0 ? chtr->left : chtr->right;

		if (distance < 50) {

			if (chtr->attack()) {
				chtr->IAtimeSpell = 0;
				Game::instance->synth.playSample("data/music/spell-0.wav", 1, false);
				world->camera->shake(3, 25);
				mainChtr->damage();
			}
			chtr->isMakingDamage = true;
		}
		else
			chtr->isMakingDamage = false;
	}
	else
	{
		Vector2 a(chtr->x, chtr->y);
		Vector2 b(chtr->x0, chtr->y0);
		Vector2 c = a - b;
		float distance = a.distance(b);

		if (distance > 1)
		{
			if (chtr->x < chtr->x0) chtr->moveRight();
			if (chtr->x > chtr->x0) chtr->moveLeft();
			if (chtr->y < chtr->y0) chtr->moveDown();
			if (chtr->y > chtr->y0) chtr->moveUp();

			if (abs(c.x) < abs(c.y))
			{
				chtr->currentRow = c.y > 0 ? chtr->up : chtr->down;
			}
			else
				chtr->currentRow = c.x > 0 ? chtr->left : chtr->right;
		}
		else {

			chtr->setPosition(chtr->x0, chtr->y0);

			if (chtr->timer > 180)
			{
				chtr->currentRow = floor(random(4));
				chtr->timer = 0;
			}
		}
	}

	if (attacking && distance < 50) {
		chtr->visible = false;
		chtr->IAtimeSpell = chtr->IAttl;
		defeated++;
	}
}

void updateNPC(Character* chtr, double seconds_elapsed)
{
	World* world = World::instance;

	if ((world->mainCharacter->isAtCave) ||
		(chtr->inTalk || !chtr->visible))
		return;

	chtr->timer++;

	if (chtr->timer > 180)
	{
		chtr->currentRow = floor(random(4));
		chtr->timer = 0;
	}
}

// DIALOGS

/*
Chica cerca del árbol
Iba al cementerio a rezar por su madre q murió  y ponerle flores
Te da la pista de la piedra
Le dice q vaya añ cementerio y busque en su corazón para salvar el alma antes de q sea demasiado tarde
(whispering) Tendré q descubrirlo
*/
void dialogS1()
{
	Game* game = Game::instance;
	
	if (World::instance->survivors_rescued[0])
	{
		game->sendText("\"Are you still here?");
		game->sendText("Please help otehr people!\"");
		return;
	}

	game->sendText("(she begins talking)");
	game->sendText("");
	game->sendText("\"Help! Help! Please!");
	game->sendText("They are coming for me!\"");
	game->sendTextPause();
	game->sendText("Don't panic girl, all is");
	game->sendText("going to be ok. ");
	game->sendText("How do you arrived here?");
	game->sendText("");
	game->sendTextPause();
	game->sendText("\"I saw something shining in the");
	game->sendText("graveyard. When I was going to");
	game->sendText("check for it, they came to me");
	game->sendText("and I had to run.\"");
	game->sendTextPause();
	game->sendText("Ok, there's no one close now.");
	game->sendText("Don't be scared.");
}

/*
Señor mayor perdido en la isla, necesita volver a casa
Es ciego
Te da la capa al llevarlo a casa pq el no la necesita

*/
void dialogS2()
{
	Game* game = Game::instance;
	World* world = World::instance;

	game->sendText("Hi Sir, what are you doing here?");
	game->sendText("");
	game->sendTextPause();
	game->sendText("\"... Hello? Please help");
	game->sendText("me arrive home. I'm blind.\"");
	game->sendTextPause();
	game->sendText("Sure I'll help, I've arrived");
	game->sendText("by boat, come with me.");
	game->sendTextPause();
	game->sendText("\"Boy... I'm so grateful. Please");
	game->sendText("take this.\"");
	game->sendText("(opens his bag)");
	game->sendText("");
	game->sendText("\"My son died few days ago and");
	game->sendText("there's no one better than you to");
	game->sendText("receive it.\"");
	game->sendText("");
	game->sendTextPause();
	game->sendText("It was not necessary... I feel");
	game->sendText("really sorry for your child.");
	game->sendTextPause();
	game->sendText("\"Please help every person alive.");
	game->sendText("This cloak will help.\"");
	game->sendText("[Invisibility cloak achieved]");

	world->s2saved = true;
}

/*
Chico que te deja el barco cerca del puerto
Le ayudas a arreglarlo pq quiere huir
Cuando acabais le convences para q se quede
En agradecimiento te dice q uses su barco cuando quieras
*/
void dialogS3()
{
	Game* game = Game::instance;
	World* world = World::instance;

	if (world->survivors_rescued[2])
	{
		game->sendText("\"Is the boat ok?");
		game->sendText("I'll need it when all");
		game->sendText("this nightmare ends\"");
		return;
	}

	world->canUseBoat = true;

	game->sendText("(he begins talking)");
	game->sendText("");
	game->sendText("\"I want to leave, I want");
	game->sendText("to explore the world.");
	game->sendText("If that illness comes up,");
	game->sendText("I'll be in a better place.\"");
	game->sendTextPause();
	game->sendText("¿?");
	game->sendText("");
	game->sendText("What? You've to stay here!");
	game->sendText("");
	game->sendText("Stick around to help other");
	game->sendText("people, they need us. ");
	game->sendTextPause();
	game->sendText("...");
	game->sendText("");
	game->sendText("\"Ok. I'll stay...But in exchange");
	game->sendText("please help me repair my boat.");
	game->sendTextPause();
	game->sendText("Done. Let me see.");
	game->sendText("");
	game->sendText("(tac tac)");
	game->sendText("");
	game->sendText("(trrrrr trrrrrr)");
	game->sendText("");
	game->sendText("(bsssss bsssss)");
	game->sendText("");
	game->sendText("[Boat repaired]");
	game->sendText("");
	game->sendTextPause();
	game->sendText("\"You made me feel secure here,");
	game->sendText("I'll be around.");
	game->sendText("Use this boat anytime.\"");
	game->sendText("");
	game->sendTextPause();
	game->sendText("Thank you, I appreciate it.");
}

/*
Hermano mayor de un niño con la enfermedad
Pide ayuda al castillo
Al rescatarlos se vuelve malo
*/
bool dialogS4()
{
	Game* game = Game::instance;
	World* world = World::instance;

	if (!world->hasLifeStone)
	{
		game->sendText("(he yells)");
		game->sendText("");
		game->sendText("\"HELLO!!! HELLOOOO!");
		game->sendText("OPEN THE DOOR PLEASE! I need help!\"");
		game->sendTextPause();
		game->sendText("Can I help you?");
		game->sendText("");
		game->sendTextPause();
		game->sendText("\"I don't think so. Please");
		game->sendText("get out of my way.\"");
		return false;
	}

	if (world->survivors_rescued[3])
	{
		game->sendText("\"This stone... I need to");
		game->sendText("understand how it works.");
		game->sendText("I could do huge things with that.");
		game->sendText("I wonder if there's something else.\"");
		return true;
	}

	game->sendText("(he yells)");
	game->sendText("");
	game->sendText("\"HELLO!!! HELLOOOO!");
	game->sendText("OPEN THE DOOR PLEASE! I need help!\"");
	game->sendTextPause();
	game->sendText("Can I help you?");
	game->sendText("");
	game->sendTextPause();
	game->sendText("\"At last! My little brother");
	game->sendText("has been infected.");
	game->sendText("We cannot do anything and I've");
	game->sendText("come to the castle to meet the people");
	game->sendText("living here. Maybe their money...\"");
	game->sendText("");
	game->sendTextPause();
	game->sendText("I have this rare stone. I've only read");
	game->sendText("old stories but it's worth a try.");
	game->sendText("(puts the stone closer)");
	game->sendText("");
	game->sendTextPause();
	game->sendText("...");
	game->sendText("");
	game->sendText("(the little brother open his eyes)");
	game->sendText("");
	game->sendText("\"BROTHER! It's a miracle!");
	game->sendText("");
	game->sendText("Thank you!It's unbelievable!\"");
	game->sendText("");
	game->sendTextPause();
	game->sendText("(thinking)");
	game->sendText("I didn't see that coming. This objects");
	game->sendText("are working. I'm scared that the stories");
	game->sendText("were nothing but telling the truth.");

	return true;
}

/*
Chico que está rodeado
Hay demasiados
Usas la capa para rescatarlo o los matas con la varita (consecuencias)
*/
void dialogS5()
{
	Game* game = Game::instance;
	World* world = World::instance;

	if (!world->hasInviCloak && !world->hasPowerWand)
	{
		game->sendText("(he yells)");
		game->sendText("");
		game->sendText("\"RUN! GET OUT! YOU WON'T");
		game->sendText("MAKE IT ALIVE!\"");
		return;
	}

	if (world->hasPowerWand)
	{
		game->sendText("\"Please take me home! I want");
		game->sendText("to see my mom!\"");
		game->sendTextPause();
		game->sendText("Keep calm, I'll do that.");
	}
	else if (world->hasInviCloak)
	{
		game->sendText("Pssst psst. Don't shout!");
		game->sendText("");
		game->sendTextPause();
		game->sendText("\"God, is that you? My prayers");
		game->sendText("we heard!\"");
		game->sendTextPause();
		game->sendText("...No man, but I'm no foe. I came");
		game->sendText("to save you.");
		game->sendTextPause();
		game->sendText("\"Then God brought you to me!");
		game->sendText("");
		game->sendTextPause();
		game->sendText("...");
		game->sendText("");
		game->sendText("I'll take you to a safe place");
		game->sendText("using this invisibility cloak.");
		game->sendTextPause();
		game->sendText("\"Thank God!");
		game->sendText("");
		game->sendTextPause();
		game->sendText("Enough... Let's go.");
	}

	world->s5saved = true;
}