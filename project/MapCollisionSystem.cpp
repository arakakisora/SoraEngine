#include "MapCollisionSystem.h"

void MapCollisionSystem::CheckPlayerCollision(
	Player* player,
	MapChipField* mapChipField,
	CollisionMapInfo& info
)
{
	if (!player) {
		return;
	}
	if (!mapChipField) {
		return;
	}

	
	player->MapCollision(info);
}

bool MapCollisionSystem::IsHittableBlock(MapChipType type)
{
	switch (type) {
	case MapChipType::Block:
	case MapChipType::UnbreakableBlock:
	case MapChipType::damageBlock:
		return true;

	default:
		return false;
	}
}

Vector3 MapCollisionSystem::CornerPosition(
	const Vector3& center,
	float width,
	float height,
	Corner corner
)
{
	Vector3 offset{};

	switch (corner) {
	case kRightBottom:
		offset = { width / 2.0f, -height / 2.0f, 0.0f };
		break;

	case kLeftBottom:
		offset = { -width / 2.0f, -height / 2.0f, 0.0f };
		break;

	case kRightTop:
		offset = { width / 2.0f, height / 2.0f, 0.0f };
		break;

	case kLeftTop:
		offset = { -width / 2.0f, height / 2.0f, 0.0f };
		break;

	default:
		offset = { 0.0f, 0.0f, 0.0f };
		break;
	}

	return center + offset;
}