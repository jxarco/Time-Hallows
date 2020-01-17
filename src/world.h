#ifndef WORLD_H
#define WORLD_H

#include "includes.h"
#include "image.h"
#include "camera.h"
#include "character.h"
#include "particle.h"
#include "utils.h"

enum { CLEAR, RAINY, SNOWY };

#define AUTO 0
#define DAY 1
#define NIGHT 2

class World {

public:
	
	static World* instance;

	Camera* camera;

	std::vector<Character*> characters;
	std::vector<Character*> survivors_only;

	bool survivors_rescued[5] = {false,false, false, false, false};
	
	Character * mainCharacter;

	bool allInquisDead = false;
	bool allSurvivorsSaved = false;

	bool hasLifeStone = false;
	bool hasPowerWand = false;
	bool hasInviCloak = false;
	bool hasLight = false;

	bool canUseBoat = false;
	bool castleHasShaked = false;
	bool awareOfGroup = false;

	bool s2saved = false;
	bool s5saved = false;

	// INVOKE DEATH
	int ttl_death;
	int timeDeath;
	bool invoked;

	Image map;
	Image map_night;
	Image map_alpha;
	Image map_alpha_night;
	Image map_mask;

	Image * current_map = &map;
	Image * current_map_alpha = &map_alpha;

	// day conditions
	int dayTime;
	int dayCondition;

	RainParticleSystem * rainSystem;

	World(int dayTime = AUTO, int dayCondition = CLEAR);
	~World();

	void render(Image * framebuffer);
	void update(double seconds_elapsed);

	void begin();

	void renderBoat(Image * framebuffer);

	void toggleRain();
	void toggleNight();
};



#endif // WORLD_H

