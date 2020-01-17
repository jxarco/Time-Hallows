#ifndef PARTICLE_H
#define PARTICLE_H

#include "utils.h"

class Image;

class RainParticleSystem {

private:

	typedef struct {

		Vector2 position;

		float ttl;
		float lifeTime;

	} Particle;

public:

	std::vector<Particle> particles;

	RainParticleSystem( int length, int w );
	~RainParticleSystem();

	void render(Image * framebuffer);
	void update(double seconds_elapsed);
};

#endif // !PARTICLE_H

