#include "picosystem.hpp"

using namespace picosystem;

void rotateVector2(float &x, float &y, float angle) {
	float newX = x * cos(angle) - y * sin(angle);
	y = x * sin(angle) + y * cos(angle);
	x = newX;
}

class AbstractEntity {
public:
	float x;
	float y;
	float angle;
	float size;
	uint16_t collisionType;
	bool alive;

public:
	explicit AbstractEntity(float _x, float _y, float _size) {
		x = _x;
		y = _y;
		angle = 0.0;
		size = _size;
	}

	virtual void update() = 0;
	virtual void draw() = 0;
};

class AbstractDynamicEntity : public AbstractEntity {
public:
	float vx;
	float vy;

public:
	explicit AbstractDynamicEntity(float _x, float _y, float _size)
		: AbstractEntity(_x, _y, _size) {
		vx = 0.0;
		vy = 0.0;
	}

	void updateDrag(float amount) {
		vx *= amount;
		vy *= amount;
	}

	void updateKinematics() {
		x += vx;
		y += vy;
	}
};

class AbstractAnimateEntity : public AbstractDynamicEntity {
private:
	int health;

public:
	explicit AbstractAnimateEntity(float _x, float _y, float _size, int _health)
		: AbstractDynamicEntity(_x, _y, _size) {
		health = _health;
	}

	void deltaHealth(int deltaHealth) {
		health += deltaHealth;
		if (health <= 0) {
			alive = false;
			health = 0;
		}
	}
};

class PlayerEntity : public AbstractAnimateEntity {
private:
	float angularVelocity;
public:
	explicit PlayerEntity(float _x, float _y)
		: AbstractAnimateEntity(_x, _y, 0.3, 100) {
		angularVelocity = 0.0;
	}

	void update() {
		if (button(LEFT)) {
			angularVelocity = std::max(-0.18, abs(angularVelocity - 0.008) * -1.7);
		}
		if (button(RIGHT)) {
			angularVelocity = std::min(0.18, abs(angularVelocity + 0.008) * 1.7);
		}
		
		angularVelocity *= 0.6;
		angle += angularVelocity;
		
		if (button(UP)) {
			vx += sin(angle) * 0.018;
			vy += cos(angle) * 0.018;
		}
		if (button(DOWN)) {
			vx -= sin(angle) * 0.006;
			vy -= cos(angle) * 0.006;
		}

		rotateVector2(vx, vy, angle);
		vx *= 0.6;
		vy *= 0.8;
		rotateVector2(vx, vy, -angle);
		updateKinematics();
	}

	void draw() {
		
	}
};

std::vector<AbstractEntity*> entities;

void addEntity(AbstractEntity* entity) {
	entities.push_back(entity);
}

void updateEntities() {
	for (int i = 0; i < entities.size(); i++) {
		entities[i]->update();
	}
}