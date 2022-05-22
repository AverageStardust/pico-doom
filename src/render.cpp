#include <algorithm>

#include "../../picoDoom/src/entity.cpp"

namespace drawsys {
	void castWall(float x, float y, int z, float theta, int i) {
		float dirX = sin(theta), dirY = cos(theta);
		int
			mapX = int(x),
			mapY = int(y),
			deltaDistX = hypotf(1.0, dirY / dirX) * 66536,
			deltaDistY = hypotf(1.0, dirX / dirY) * 66536;

		int tileStepX, tileStepY, sideDistX, sideDistY;
		if (dirX < 0) {
			tileStepX = -1;
			sideDistX = (x - mapX) * deltaDistX;
		}
		else {
			tileStepX = 1;
			sideDistX = (mapX + 1.0 - x) * deltaDistX;
		}
		if (dirY < 0) {
			tileStepY = -1;
			sideDistY = (y - mapY) * deltaDistY;
		}
		else {
			tileStepY = 1;
			sideDistY = (mapY + 1.0 - y) * deltaDistY;
		}

		int tileData;
		for (;;) {
			while (sideDistX < sideDistY) {
				sideDistX += deltaDistX;
				mapX += tileStepX;
				tileData = _tiledata[mapX + mapY * TILEDATA_WIDTH];
				if (tileData) goto hitX;
			}
			do {
				sideDistY += deltaDistY;
				mapY += tileStepY;
				tileData = _tiledata[mapX + mapY * TILEDATA_WIDTH];
				if (tileData) goto hitY;
			} while (sideDistX >= sideDistY);
		}
		return;

		float wallDist, textureX;
	hitX:
		wallDist = (mapX - x + (1 - tileStepX) / 2.0) / dirX;
		textureX = fmod(y + wallDist * dirY, 1.0);
		if (tileStepX < 0) textureX = 1.0 - textureX;
		goto hitGeneral;
	hitY:
		wallDist = (mapY - y + (1 - tileStepY) / 2.0) / dirY;
		textureX = fmod(x + wallDist * dirX, 1.0);
		if (tileStepY < 0) textureX = 1.0 - textureX;

	hitGeneral:
		int tile = (tileData % 256);
		int wallScale = (tileData >> 8) % 16;
		int textureHeight = tileData >> 12;

		DrawSlice slice = createSlice(
			wallDist,
			SCENE_WALL_SCALE * wallScale - z,
			SCENE_WALL_SCALE + z,
			(tile % TILE_SHEET_WIDTH) * TILE_WIDTH + int(textureX * TILE_WIDTH),
			(tile / TILE_SHEET_WIDTH) * TILE_HEIGHT,
			(TILE_HEIGHT >> 1) * (textureHeight + 1)
		);
		appendWall(i, wallDist, slice);
	}

	float thetaFunction(float angle, int x) {
		return angle + (119.5 - float(x)) * 0.0062;
	}

	void castWalls(AbstractDynamicEntity* player) {
		float
			x = player->x,
			y = player->y,
			angle = player->angle;

		float speed = hypot(player->vx, player->vy);
		float z = 0;
		if (speed > 0.03) {
			z = (abs(sin(time() * 0.007)) - 0.5) * speed * 130.0;
		}

		for (int i = 0; i < 240; i++) {
			float theta = thetaFunction(angle, i);
			castWall(x, y, round(z), theta, i);
		}
	}

	void renderSlices(AbstractDynamicEntity* player) {
		float angle = player->angle;

		for (int x = 0; x < 240; x += 3) {
			int startY = 120;
			if (columnCount[x]) {
				DrawSlice wallSlice = columnSlices[x][0];
				startY = wallSlice.startY;
			}
			if (columnCount[x + 1]) {
				DrawSlice wallSlice = columnSlices[x + 1][0];
				startY = std::max(startY, wallSlice.startY);
			}
			if (columnCount[x + 2]) {
				DrawSlice wallSlice = columnSlices[x + 2][0];
				startY = std::max(startY, wallSlice.startY);
			}
			int skyX = int(thetaFunction(angle, x + 1) / M_PI / 2.0 * 256.0) & 0xFF;

			int screenIndex = x;
			for (int y = 0; y < startY; y += 2) {
				uint16_t pixel = *_spritesheet->p(SCENE_TEXTURE_Y + (y >> 1), SCENE_TEXTURE_X + skyX);
				SCREEN->data[screenIndex] = pixel;
				SCREEN->data[screenIndex + 1] = pixel;
				SCREEN->data[screenIndex + 2] = pixel;
				SCREEN->data[screenIndex + 240] = pixel;
				SCREEN->data[screenIndex + 241] = pixel;
				SCREEN->data[screenIndex + 242] = pixel;
				screenIndex += 480;
			}
		}

		for (int x = 0; x < 240; x++) {
			int endY = 120;
			if (columnCount[x]) {
				DrawSlice wallSlice = columnSlices[x][0];
				endY = wallSlice.endY;
			}
			int startIndex = x + endY * 240;
			for (int i = startIndex; i < 240 * 240; i += 240) {
				SCREEN->data[i] = 0x22F3;
			}
			// draw contents of column
			for (int i = 0; i < columnCount[x]; i++) {
				DrawSlice slice = columnSlices[x][i];
				// screen pixel to texture pixel ratio
				uint32_t sourceRatio = uint32_t(float(slice.sourceHeight) / float(slice.endY - slice.startY) * 4194304.0) - 4096;
				// draw slice
				int drawStart = std::max(slice.startY, 0) * 240;
				int drawEnd = std::min(slice.endY, 240) * 240;
				uint32_t sourceLocation = (std::max(slice.startY, 0) - slice.startY) * sourceRatio;
				if (i == 0) {
					for (int j = drawStart; j < drawEnd; j += 240) {
						int sourceY = slice.sourceY + (sourceLocation >> 22);
						sourceLocation += sourceRatio;
						uint16_t pixel = *_spritesheet->p(sourceY, slice.sourceX);
						SCREEN->data[x + j] = pixel;
					}
				}
				else {
					for (int j = drawStart; j < drawEnd; j++) {
						int sourceY = slice.sourceY + (sourceLocation >> 24);
						sourceLocation += sourceRatio;
						uint16_t pixel = *_spritesheet->p(sourceY, slice.sourceX);
						// skip alpha
						if (pixel & 0x00F0 == 0) continue;
						SCREEN->data[x + (slice.startY + j) * 240] = pixel;
					}
				}
			}
		}
	}

	void render(AbstractDynamicEntity* player) {
		castWalls(player);
		renderSlices(player);
	}
}