#include "../../picoDoom/src/common.cpp"
#include "../../picoDoom/src/leveldata.cpp"
#include "../../picoDoom/src/drawsys.cpp"

namespace entity {
	class AbstractEntity {
	public:
		float x;
		float y;
		float z;
		float angle;
		float size;
		uint16_t collisionType;
		bool alive;

	public:
		explicit AbstractEntity(float _x, float _y, float _size) {
			x = _x;
			y = _y;
			z = 0;
			angle = 0.0;
			size = _size;
		}

		void drawSurface(AbstractEntity* player, float angle, float radius, float height, int sourceX, int sourceY, int sourceWidth, int sourceHeight) {
			float
				px = player->x,
				py = player->y,
				pz = player->z,
				pa = player->angle;
			float
				x1 = x + sin(angle) * radius,
				y1 = y + cos(angle) * radius,
				x2 = x - sin(angle) * radius,
				y2 = y - cos(angle) * radius;
			float
				angle1 = wrapAngle(pa - atan2f(x1 - px, y1 - py)),
				angle2 = wrapAngle(pa - atan2f(x2 - px, y2 - py));
			int
				column1 = drawsys::thetaToColumn(angle1),
				column2 = drawsys::thetaToColumn(angle2);
			float
				dist1 = hypotf(x1 - px, y1 - py) * cos(angle1),
				dist2 = hypotf(x2 - px, y2 - py) * cos(angle2);
			int
				topZ = int(height * float(drawsys::SCENE_WALL_SCALE)) - drawsys::SCENE_WALL_SCALE - pz,
				bottomZ = -drawsys::SCENE_WALL_SCALE - pz;

			float columnRatio = 1.0 / abs(column1 - column2);
			if (column1 > column2) {
				std::swap(column1, column2);
				std::swap(dist1, dist2);
			}
			int startColumn = std::max(0, column1);
			int endColumn = std::min(239, column2);
			for (int i = startColumn; i <= endColumn; i++) {
				float surfaceX = float(i - column1) * columnRatio;
				float dist = dist1 + (dist2 - dist1) * surfaceX;
				if (dist < 0.2) continue;

				drawsys::DrawSlice slice = drawsys::createSlice(
					dist,
					topZ,
					bottomZ,
					sourceX + int(float(sourceWidth) * surfaceX),
					sourceY,
					sourceHeight
				);
				drawsys::appendWall(i, dist, slice);
			}
		}

		virtual void update() = 0;
		virtual void draw(AbstractEntity* player) = 0;
	};

	class DoorEntity : public AbstractEntity {
	public:
		explicit DoorEntity(float _x, float _y, float _angle)
			: AbstractEntity(_x, _y, 0.5) {
			angle = _angle;
		}

		void update() {}
		void draw(AbstractEntity* player) {
			drawSurface(player, angle, 0.5, 2.0, 320, 0, 64, 64);
		}
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

		void collideLevelPoint(float x, float y, std::vector<std::pair<float, float>>& solutions) {
			int ix = int(x), iy = int(y);
			if (!_tiledata[ix + iy * TILEDATA_WIDTH]) return;

			if (x - ix < 0.5) {
				if (!_tiledata[ix - 1 + iy * TILEDATA_WIDTH]) {
					solutions.push_back(std::make_pair(ix - x, 0.0));
				}
			}
			else {
				if (!_tiledata[ix + 1 + iy * TILEDATA_WIDTH]) {
					solutions.push_back(std::make_pair((ix + 1) - x, 0.0));
				}
			}

			if (y - iy < 0.5) {
				if (!_tiledata[ix + (iy - 1) * TILEDATA_WIDTH]) {
					solutions.push_back(std::make_pair(0.0, iy - y));
				}
			}
			else {
				if (!_tiledata[ix + (iy + 1) * TILEDATA_WIDTH]) {
					solutions.push_back(std::make_pair(0.0, (iy + 1) - y));
				}
			}
		}

		static bool compareLevelCollisionPairs(std::pair<float, float> a, std::pair<float, float> b) {
			return abs(a.first) + abs(a.second) < abs(b.first) + abs(b.second);
		}

		void collideLevel(int tries = 4) {
			if (!tries) return;

			std::vector<std::pair<float, float>> solutions;
			collideLevelPoint(x - size, y - size, solutions);
			collideLevelPoint(x + size, y - size, solutions);
			collideLevelPoint(x - size, y + size, solutions);
			collideLevelPoint(x + size, y + size, solutions);

			if (!solutions.size()) return;

			auto [solutionX, solutionY] =
				*std::min_element(solutions.begin(), solutions.end(), compareLevelCollisionPairs);

			if (solutionX != 0.0) {
				x += solutionX * 1.05;
				vx = 0;
			}
			if (solutionY != 0.0) {
				y += solutionY * 1.05;
				vy = 0;
			}

			collideLevel(tries - 1);
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
			: AbstractAnimateEntity(_x, _y, 0.2, 100) {
			angularVelocity = 0.0;
		}

		void update() {
			if (button(LEFT)) {
				angularVelocity = std::min(0.18, abs(angularVelocity + 0.008) * 1.7);
			}
			if (button(RIGHT)) {
				angularVelocity = std::max(-0.18, abs(angularVelocity - 0.008) * -1.7);
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
			collideLevel();

			float speed = hypotf(vx, vy);
			if (speed > 0.03) {
				z = (abs(sin(time() * 0.0055)) - 0.5) * speed * 150.0;
			}
			else {
				z = 0;
			}
		}

		void draw(AbstractEntity* player) {

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

	void drawEntities(AbstractEntity* player) {
		for (int i = 0; i < entities.size(); i++) {
			entities[i]->draw(player);
		}
	}
}