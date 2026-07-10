#pragma once

#include "CollisionMapInfo.h"
#include "MapChipField.h"
#include "Vector3.h"
#include "Player.h"

enum Corner;

class MapCollisionSystem {
public:
	static void CheckPlayerCollision(
		Player* player,
		MapChipField* mapChipField,
		CollisionMapInfo& info
	);

	static bool IsHittableBlock(MapChipType type);

	static Vector3 CornerPosition(
		const Vector3& center,
		float width,
		float height,
		Corner corner
	);
};