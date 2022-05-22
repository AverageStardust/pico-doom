#include "../../picoDoom/src/render.cpp"

entity::PlayerEntity* player = new entity::PlayerEntity(2.5, 2.5);

void init() {
	addEntity(player);
}

void _update(uint32_t tick) {
	entity::updateEntities();
}

void update(uint32_t tick) {
	if (stats.fps < 40) {
		// overan frame, update twice
		_update(tick);
	}
	_update(tick);
}

void draw(uint32_t tick) {
	drawsys::render(player);

	pen(15, 0, 0);
	text(std::to_string(stats.fps), 0, 0);
}