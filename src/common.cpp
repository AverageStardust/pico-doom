#include "picosystem.hpp"
#include <math.h>

using namespace picosystem;

void rotateVector2(float& x, float& y, float angle) {
	float newX = x * cos(angle) - y * sin(angle);
	y = x * sin(angle) + y * cos(angle);
	x = newX;
}

int sign(float val) {
	return val < 0.0f ? -1.0f : 1.0f;
}