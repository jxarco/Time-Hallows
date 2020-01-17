/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#include "includes.h"
#include "image.h"
#include "utils.h"
#include "synth.h"
#include "state.h"

class Game
{
public:
	static Game* instance;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	int bufferSize;
	bool maskActive;
	bool maskView;
	bool alphaMask;
	bool debug;

	bool pendingAttack = false;
	int redScreenTimer = 0;

	std::vector<Vector4> raindrops;
	std::vector<std::string> textLines;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;

	double gameTime; // in seconds
	double totalTimeElapsed = 0; // in seconds
	
	bool must_exit;
	bool must_start = false;

	bool START = false;
	bool GAME_END = false;
	bool GAME_OVER = false;

	bool raining = false;
	int time_selected = 0;
	int character_selected = 0;

	// prompts
	bool userInput = false;

	//audio
	Synth synth;

	//ctor
	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );
	void reset();

	void showFramebuffer(Image* img);

	// gui
	void prompt(const char * text, const char * a, const char * b);
	void sendText( std::string text );
	void sendTextPause();

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseMove(SDL_MouseMotionEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onResize(int width, int height);

	//audio stuff
	void enableAudio(); //opens audio channel to play sound
	void onAudio(float* buffer, unsigned int len, double time, SDL_AudioSpec &audio_spec); //called constantly to fill the audio buffer
};


#endif 