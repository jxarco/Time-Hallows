#include "particle.h"
#include "image.h"
#include "world.h"
#include "game.h"

#include <algorithm>
#include <vector>

RainParticleSystem::RainParticleSystem(int length, int w)
{
	for (int i = 0; i < 10000; i++)
	{
		int x = random(w);
		int y = random(w);
		int ttl = random(300, 100);
		int lifetime = 0;

		Particle p;
		p.position = Vector2(x, y);
		p.ttl = ttl;
		p.lifeTime = 0;
		
		this->particles.push_back(p);
	}
}

RainParticleSystem::~RainParticleSystem()
{

}

void RainParticleSystem::render(Image * framebuffer)
{
	for (int i = 0; i < this->particles.size(); i++)
	{
		Particle particle = this->particles.at(i);
		int px = particle.position.x;
		int py = particle.position.y;
		int ttl = particle.ttl;
		int lifeTime = particle.lifeTime;

		int x = px - World::instance->camera->x;
		int y = py - World::instance->camera->y;

		// buffersize = 256
		if (x > 256 || y > 256 || x < 0 || y < 0)
			continue;

		int alpha = 255 - (float(lifeTime) / float(ttl) * 255);
		framebuffer->drawLine(x, y, x + 2, y + 3 + random(2), Color(217, 212, 255, alpha));
	}
}

void RainParticleSystem::update(double seconds_elapsed)
{
	// update rain
	std::for_each(this->particles.begin(), this->particles.end(), [](Particle& p) {
		p.position.x += 1;
		p.position.y += 1.5 + random(1);
		p.lifeTime++; // increase lifetime

		Game* game = Game::instance;
		int map_size = World::instance->map.width;
		int ttl = p.ttl;

		if (p.position.x > map_size || p.position.y > map_size || p.lifeTime > ttl)
		{
			p.position.x = random(map_size + map_size / 2, -map_size / 2);
			p.position.y = random(map_size);
			p.ttl = random(300, 100);
			p.lifeTime = 0;
		}

	});
}