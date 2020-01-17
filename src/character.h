#ifndef CHARACTER_H
#define CHARACTER_H

#include "includes.h"
#include "utils.h"
#include "sprite.h"

enum {PLAYER = 0, IA, NPC};

typedef struct {
	Color color = Color::WHITE;
	bool colision = false;
	bool interaction = false;
} sColision;

class Character : public Sprite {

protected:

	sColision checkColision(int cx, int cy);
	
public:

	// IA
	int IAttl;
	int IAtimeSpell;

	bool gotWandCode = false;

	bool gotWand = false;
	bool gotStone = false;
	bool gotCloak = false;

	bool mustBeInCave = false;

	bool visible = true;
	bool travelling = false;
	bool isAtCave = false;
	bool isAtIsland = false;

	int up;
	int down;
	int right;
	int left;

	bool inTalk = false;
	bool canInteract = false;
	Color interactionColor = Color::WHITE;
	bool isMakingDamage = false;

	float lifePoints;
	float maxLifePoints = 100.f;

	int state;
	std::string name;

	int attackTimer = 0;
	int maxAttackTimer = 60;
	int waitTime = 0;

	Character();
	Character(const char* filename, int w, int h, int rows, int cols, int beginr, int beginc);
	~Character();

	void setTileOrder(int up, int down, int right, int left);
	void setState(int state);

	void render(Image& frameBuffer);
	void renderEffects(Image& frameBuffer);
	void update(double seconds_elapsed);

	bool moveLeft(bool rotate = true);
	bool moveRight(bool rotate = true);
	bool moveUp(bool rotate = true);
	bool moveDown(bool rotate = true);

	bool attack();
	void interact();
	void damage();

	bool collides(int direction);
};


#endif 