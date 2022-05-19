#include "picosystem.hpp"

#include "../../picoDoom/src/spritesheet.cpp"
#include "../../picoDoom/src/leveldata.cpp"
#include "../../picoDoom/src/drawsystem.cpp"

using namespace picosystem;

PlayerEntity player* = entity::createPlayer(2.5, 2.5);

void init() {
}

void _update(uint32_t tick) {
	player.angle += 0.01;
}

void update(uint32_t tick) {
	if (stats.fps < 40) {
		// overan frame, update twice
		_update(tick);
	}
	_update(tick);
}

void draw(uint32_t tick) {
	drawsys::render(&player);

	pen(15, 0, 0);
	text(std::to_string(stats.fps), 0, 0);
}