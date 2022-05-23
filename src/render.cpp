#include <algorithm>

#include "../../picoDoom/src/entity.cpp"

namespace drawsys {
	void sliceWall(float x, float y, int z, float angle, int i) {
		float thetaOffset = columnToTheta(i);
		float theta = angle + thetaOffset;
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
		wallDist = ((mapX + (1 - tileStepX) / 2) - x) / dirX;
		textureX = fmod(y + wallDist * dirY, 1.0);
		if (tileStepX < 0) textureX = 1.0 - textureX;
		goto hitGeneral;
	hitY:
		wallDist = ((mapY + (1 - tileStepY) / 2) - y) / dirY;
		textureX = fmod(x + wallDist * dirX, 1.0);
		if (tileStepY < 0) textureX = 1.0 - textureX;

	hitGeneral:
		int tile = (tileData % 256);
		int wallScale = (tileData >> 8) % 16;
		int textureHeight = tileData >> 12;

		float trueDist = wallDist * cos(thetaOffset);

		DrawSlice slice = createSlice(
			trueDist,
			SCENE_WALL_SCALE * wallScale - z,
			-SCENE_WALL_SCALE - z,
			(tile % TILE_SHEET_WIDTH) * TILE_WIDTH + int(textureX * TILE_WIDTH),
			(tile / TILE_SHEET_WIDTH) * TILE_HEIGHT,
			(TILE_HEIGHT >> 1) * (textureHeight + 1)
		);
		appendWall(i, trueDist, slice);
	}

	void sliceWalls(entity::AbstractEntity* player) {
		float
			x = player->x,
			y = player->y,
			z = player->z,
			angle = player->angle;

		for (int i = 0; i < 240; i++) {
			sliceWall(x, y, z, angle, i);
		}
	}

	void renderSlices(entity::AbstractEntity* player) {
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
			float theta = (columnToTheta(x + 1) + angle);
			int skyX = int(theta / TAU * 256.0) & 0xFF;

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
			{ // draw floor
				DrawSlice wallSlice = columnSlices[x][0];
				int startIndex = x + wallSlice.endY * 240;
				for (int i = startIndex; i < 240 * 240; i += 240) {
					SCREEN->data[i] = 0x22F3;
				}
			}
			// draw contents of column
			for (int i = 0; i < 2; i++) {
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
					for (int j = drawStart; j < drawEnd; j += 240) {
						int sourceY = slice.sourceY + (sourceLocation >> 22);
						sourceLocation += sourceRatio;
						uint16_t pixel = *_spritesheet->p(sourceY, slice.sourceX);
						// skip alpha
						if (pixel & 0x00F0 == 0) continue;
						SCREEN->data[x + j] = pixel;
					}
				}
			}
		}
	}

	void render(entity::AbstractEntity* player) {
		sliceWalls(player);
		entity::drawEntities(player);
		renderSlices(player);
	}
}