#include "picosystem.hpp"
#include <math.h>

using namespace picosystem;

const float PI = 3.14159265358;
const float TAU = 6.28318530718;

void rotateVector2(float& x, float& y, float angle) {
	float newX = x * cos(angle) - y * sin(angle);
	y = x * sin(angle) + y * cos(angle);
	x = newX;
}

int sign(float val) {
	return val < 0.0f ? -1.0f : 1.0f;
}

float wrapAngle(float angle) {
	return angle - TAU * floor((angle + PI) / TAU);
}