#include "game.h"
#include "utils.h"
#include "input.h"
#include "image.h"
#include "camera.h"
#include "character.h"
#include "ui.h"
#include "menustate.h"
#include "playstate.h"
#include "endstate.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <regex>

Game* Game::instance = NULL;

double findMod(double a, double b);
Color bgcolor(130, 80, 100);

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	
	must_exit = false;
	must_start = false;
	debug = false;
	maskActive = true;
	alphaMask = true;
	maskView = false;

	bufferSize = 256;
	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;

	World::instance = new World();
	GUI::instance = new GUI();

	enableAudio();
	synth.playSample("data/music/ambient.wav", 0.6, true);

	StateManager::instance = new StateManager();
	PlayState::instance = new PlayState();
	MenuState::instance = new MenuState();
	EndState::instance = new EndState();
	StateManager::instance->setInitialState(MenuState::instance);
}

void Game::reset()
{
	debug = false;
	maskActive = true;
	alphaMask = true;

	time = 0.0f;

	World::instance->begin();
	GUI::instance = new GUI();
}

//what to do when the image has to be draw
void Game::render(void)
{
	Image framebuffer(this->bufferSize, this->bufferSize);
	StateManager::instance->render(&framebuffer);
	showFramebuffer(&framebuffer);
}

void Game::update(double seconds_elapsed)
{
	World * world = World::instance;

	gameTime = findMod(double(time), double(60));
	StateManager::instance->update(seconds_elapsed);

	if(Input::wasKeyPressed(SDL_SCANCODE_ESCAPE))
		must_exit = true;
	if (Input::wasKeyPressed(SDL_SCANCODE_0))
		GUI::instance->toggle();
	if (Input::wasKeyPressed(SDL_SCANCODE_1))
		this->debug = !this->debug;
	if (Input::wasKeyPressed(SDL_SCANCODE_2))
		World::instance->toggleRain();
	if (Input::wasKeyPressed(SDL_SCANCODE_3))
		World::instance->toggleNight();
	if (Input::wasKeyPressed(SDL_SCANCODE_4))
		this->maskActive = !this->maskActive;
	if (Input::wasKeyPressed(SDL_SCANCODE_5))
		this->maskView = !this->maskView;
	if (Input::wasKeyPressed(SDL_SCANCODE_6)) {
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT))
			world->camera->shake(10, 80);
		else
			this->alphaMask = !this->alphaMask;
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_7))
		world->hasPowerWand = !world->hasPowerWand;
	if (Input::wasKeyPressed(SDL_SCANCODE_8))
		world->hasLifeStone = !world->hasLifeStone;
	if (Input::wasKeyPressed(SDL_SCANCODE_9))
		world->hasInviCloak = !world->hasInviCloak;
}

void Game::sendText(std::string text)
{
	GUI::instance->writing = true;
	std::string s = text;
	std::regex e("\n");
	std::regex_token_iterator<std::string::iterator> i(s.begin(), s.end(), e, -1);
	std::regex_token_iterator<std::string::iterator> end;
	while (i != end) {
		std::string ss = *i++ ;
		this->textLines.push_back(ss);
	}
}

void Game::sendTextPause()
{
	this->textLines.push_back("");
	this->textLines.push_back("");
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	window_width = width;
	window_height = height;
}

//sends the image to the framebuffer of the GPU
void Game::showFramebuffer(Image* img)
{
	static Image finalframe;

	if (window_width < img->width * 4 || window_height < img->height * 4)
	{
		finalframe = *img;
		finalframe.scale( window_width, window_height );
	}
	else
	{
		if (finalframe.width != window_width || finalframe.height != window_height)
		{
			finalframe.resize(window_width, window_height);
			finalframe.fill(Color::BLACK);
		}
		finalframe.drawImage(*img, (window_width - img->width * 4) * 0.5, (window_height - img->height * 4) * 0.5, img->width * 4, img->height * 4);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (1) //flip
	{
		glRasterPos2f(-1, 1);
		glPixelZoom(1, -1);
	}

	glDrawPixels(finalframe.width, finalframe.height, GL_RGBA, GL_UNSIGNED_BYTE, finalframe.pixels);
}

//AUDIO STUFF ********************

SDL_AudioSpec audio_spec;

void AudioCallback(void*  userdata,
	Uint8* stream,
	int    len)
{
	static double audio_time = 0;

	memset(stream, 0, len);//clear
	if (!Game::instance)
		return;

	Game::instance->onAudio((float*)stream, len / sizeof(float), audio_time, audio_spec);
	audio_time += len / (double)audio_spec.freq;
}

void Game::enableAudio()
{
	SDL_memset(&audio_spec, 0, sizeof(audio_spec)); /* or SDL_zero(want) */
	audio_spec.freq = 48000;
	audio_spec.format = AUDIO_F32;
	audio_spec.channels = 1;
	audio_spec.samples = 1024;
	audio_spec.callback = AudioCallback; /* you wrote this function elsewhere. */
	if (SDL_OpenAudio(&audio_spec, &audio_spec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	SDL_PauseAudio(0);
}

void Game::onAudio(float *buffer, unsigned int len, double time, SDL_AudioSpec& audio_spec)
{
	//fill the audio buffer using our custom retro synth
	synth.generateAudio(buffer, len, audio_spec);
}

//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onMouseMove(SDL_MouseMotionEvent event)
{
	// std::cout << event.x << "\n";
}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
}

double findMod(double a, double b)
{
	// Handling negative values 
	if (a < 0)
		a = -a;
	if (b < 0)
		b = -b;

	// Finding mod by repeated subtraction 
	double mod = a;
	while (mod >= b)
		mod = mod - b;

	// Sign of result typically depends 
	// on sign of a. 
	if (a < 0)
		return -mod;

	return mod;
}

