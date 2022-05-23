#include "../../picoDoom/src/spritesheet.cpp"

namespace drawsys {
	const int TILE_WIDTH = 64;
	const int TILE_HEIGHT = 64;
	const int TILE_SHEET_WIDTH = _spritesheet->h / TILE_WIDTH;

	const int PALETTE_X = 256;
	const int PALETTE_Y = 191;
	const int SCENE_TEXTURE_X = 256;
	const int SCENE_TEXTURE_Y = 128;
	const int SCENE_WALL_SCALE = 90;

	struct DrawSlice {
		int startY;
		int endY;
		uint16_t sourceX;
		uint16_t sourceY;
		int sourceHeight;
	};

	DrawSlice columnSlices[240][4];
	float columnDepth[240];
	uint8_t columnCount[240];

	DrawSlice createSlice(float dist, int topZ, int bottomZ, int sourceX, int sourceY, int sourceHeight) {
		DrawSlice slice;
		slice.startY = 120 - topZ / dist;
		slice.endY = 120 - bottomZ / dist;
		slice.sourceX = sourceX;
		slice.sourceY = sourceY;
		slice.sourceHeight = sourceHeight;
		return slice;
	}

	void appendSlice(int x, float depth, DrawSlice slice) {
		int count = columnCount[x];
		if (count >= 4) return;
		if (depth > columnDepth[x]) return;
		columnSlices[x][count] = slice;
		columnCount[x]++;
	}

	void appendWall(int x, float depth, DrawSlice slice) {
		columnCount[x] = 0; // clear column
		columnDepth[x] = depth;
		appendSlice(x, depth, slice);
	}

	float columnToTheta(int x) {
		float centerDist = (119.5 - float(x));
		return centerDist * (abs(centerDist) * -0.00001 + 0.007);
	}

	int thetaToColumn(float theta) {
		if (theta > 1.225) return 420;
		if (theta < -1.225) return -180;
		float discriminant = 0.000049 - 0.00004 * abs(theta);
		float absColumn = (0.007 - sqrt(discriminant)) / 0.00002;
		return int((absColumn * sign(theta)) + 119.5);
	}
}