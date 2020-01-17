#ifndef UI_H
#define UI

#include "includes.h"
#include "image.h"
#include "camera.h"
#include "character.h"
#include "particle.h"
#include "utils.h"
#include "game.h"
#include "world.h"

class GUI {

private:

	bool show;

	int textIndex;
	int boxIndex;

public:

	static GUI* instance;

	World * world;

	bool writing = false;

	int cinematic_height;

	GUI();
	~GUI();
	
	void render( Image &framebuffer );
	void renderText( Image &framebuffer );
	void renderPrompt(Image &framebuffer);

	void updateText();
	inline void toggle() { show = !show; };

	void startCinema();
	void stopCinema();
};

#endif // !UI_H

