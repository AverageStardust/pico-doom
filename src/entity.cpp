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

	virtual void update();
	virtual void draw();
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
public:
	explicit PlayerEntity(float _x, float _y)
		: AbstractAnimateEntity(_x, _y, 0.3, 100) {
	}

	void update() {
		updateDrag(0.99);
		updateKinematics();
	}

	void draw() {

	}
};