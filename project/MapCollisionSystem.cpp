#define NOMINMAX
#include "MapCollisionSystem.h"
#include <cmath>
#include <algorithm>

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

Vector3 MapCollisionSystem::NormalFromType(CollisionType type)
{
	static const std::array<Vector3, 4> normal{ {
			/* Top    */ Vector3{ 0.0f, -1.0f, 0.0f },
			/* Bottom */ Vector3{ 0.0f,  1.0f, 0.0f },
			/* Left   */ Vector3{ 1.0f,  0.0f, 0.0f },
			/* Right  */ Vector3{ -1.0f, 0.0f, 0.0f }
		} };

	const auto index = static_cast<std::size_t>(type);
	if (index < normal.size()) {
		return normal[index];
	}

	return { 0.0f, 0.0f, 0.0f };
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

bool MapCollisionSystem::CheckCollisionPoints(
	const Vector3& basePos,
	const std::array<Vector3, 2>& posList,
	CollisionType type,
	CollisionMapInfo& info,
	MapChipField* mapChipField,
	float width,
	float height,
	float blank,
	bool enableGoal
)
{
	if (!mapChipField) {
		return false;
	}

	const Vector3 movedCenter = basePos + info.move;

	bool hit = false;
	bool hasBest = false;
	Rect bestRect{};
	float bestPen = -1.0f;

	for (const Vector3& p : posList) {
		IndexSet idx = mapChipField->GetMapChipIndexSetByPosition(p);
		MapChipType chip = mapChipField->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

		Rect r = mapChipField->GetRectByIndex(idx.xIndex, idx.yIndex);

		float pen = 0.0f;

		switch (type) {
		case CollisionType::Right:
			pen = (movedCenter.x + width * 0.5f) - r.left;
			break;

		case CollisionType::Left:
			pen = r.right - (movedCenter.x - width * 0.5f);
			break;

		case CollisionType::Top:
			pen = (movedCenter.y + height * 0.5f) - r.bottom;
			break;

		case CollisionType::Bottom:
			pen = r.top - (movedCenter.y - height * 0.5f);
			break;
		}

		if (chip == MapChipType::Goal) {
			if (enableGoal) {
				if (pen > -blank) {
					info.hitGoal = true;
				}
			}
			continue;
		}

		if (chip == MapChipType::damageBlock) {
			if (pen > 0.0f) {
				info.hitDamageBlock = true;
			}
		}

		if (chip == MapChipType::Block) {
			if (pen > 0.0f) {
				if (!info.hasBreakBlock) {
					info.hasBreakBlock = true;
					info.breakBlockX = idx.xIndex;
					info.breakBlockY = idx.yIndex;
				}
			}
		}

		

		if (!IsHittableBlock(chip)) {
			continue;
		}

		hit = true;

		if (pen > 0.0f) {
			if (type == CollisionType::Right || type == CollisionType::Left) {
				info.penX = std::max(info.penX, pen);
			} else {
				info.penY = std::max(info.penY, pen);
			}

			if (!hasBest || pen > bestPen) {
				hasBest = true;
				bestPen = pen;
				bestRect = r;
			}
		}
	}

	if (!hit || !hasBest) {
		return false;
	}

	switch (type) {
	case CollisionType::Top:
		info.ceiling = true;
		info.move.y = bestRect.bottom - basePos.y - (height * 0.5f + blank);
		info.normal = info.normal + NormalFromType(type);
		info.hasNormal = true;
		break;

	case CollisionType::Bottom:
		info.landing = true;
		info.move.y = bestRect.top - basePos.y + (height * 0.5f + blank);
		info.normal = info.normal + NormalFromType(type);
		info.hasNormal = true;
		break;

	case CollisionType::Left:
		info.hitWall = true;
		info.move.x = bestRect.right - basePos.x + (width * 0.5f + blank);
		info.normal = info.normal + NormalFromType(type);
		info.hasNormal = true;
		break;

	case CollisionType::Right:
		info.hitWall = true;
		info.move.x = bestRect.left - basePos.x - (width * 0.5f + blank);
		info.normal = info.normal + NormalFromType(type);
		info.hasNormal = true;
		break;
	}

	return true;
}

void MapCollisionSystem::CollisionMapInfoDirection(
	const Vector3& basePos,
	CollisionMapInfo& info,
	CollisionType dir,
	const std::array<Corner, 2>& checkCorners,
	const Vector3& offset,
	float width,
	float height,
	float blank,
	MapChipField* mapChipField,
	bool enableGoal
)
{
	Vector3 movedPos = basePos + info.move;

	std::array<Vector3, 2> posList = {
		CornerPosition(movedPos, width, height, checkCorners[0]) + offset,
		CornerPosition(movedPos, width, height, checkCorners[1]) + offset
	};

	CheckCollisionPoints(
		basePos,
		posList,
		dir,
		info,
		mapChipField,
		width,
		height,
		blank,
		enableGoal
	);
}

void MapCollisionSystem::CheckCollisionAt(
	const Vector3& position,
	CollisionMapInfo& info,
	float width,
	float height,
	float blank,
	float collisionEpsilon,
	MapChipField* mapChipField,
	bool enableGoal)
{
	if (!mapChipField) {
		return;
	}

	Vector3 base = position;

	// --------------------
	// X方向だけ解く
	// --------------------
	CollisionMapInfo xInfo{};
	xInfo.move = { info.move.x, 0.0f, 0.0f };
	xInfo.normal = { 0.0f, 0.0f, 0.0f };
	xInfo.hasNormal = false;
	xInfo.penX = 0.0f;
	xInfo.penY = 0.0f;

	if (xInfo.move.x > 0.0f) {
		CollisionMapInfoDirection(
			base,
			xInfo,
			CollisionType::Right,
			{ kRightTop, kRightBottom },
			Vector3(collisionEpsilon, 0.0f, 0.0f),
			width,
			height,
			blank,
			mapChipField,
			enableGoal
		);
	}

	if (xInfo.move.x < 0.0f) {
		CollisionMapInfoDirection(
			base,
			xInfo,
			CollisionType::Left,
			{ kLeftTop, kLeftBottom },
			Vector3(-collisionEpsilon, 0.0f, 0.0f),
			width,
			height,
			blank,
			mapChipField,
			enableGoal
		);
	}

	// X反映
	base.x += xInfo.move.x;
	info.move.x = xInfo.move.x;

	if (xInfo.hasNormal) {
		info.normal = info.normal + xInfo.normal;
		info.hasNormal = true;
	}

	info.hitWall = info.hitWall || xInfo.hitWall;
	info.penX = std::max(info.penX, xInfo.penX);

	if (xInfo.hasBreakBlock && !info.hasBreakBlock) {
		info.hasBreakBlock = true;
		info.breakBlockX = xInfo.breakBlockX;
		info.breakBlockY = xInfo.breakBlockY;
	}

	if (xInfo.hitDamageBlock) {
		info.hitDamageBlock = true;
	}

	if (xInfo.hitGoal) {
		info.hitGoal = true;
	}

	// --------------------
	// Y方向だけ解く
	// X反映後の base で判定する
	// --------------------
	CollisionMapInfo yInfo{};
	yInfo.move = { 0.0f, info.move.y, 0.0f };
	yInfo.normal = { 0.0f, 0.0f, 0.0f };
	yInfo.hasNormal = false;
	yInfo.penX = 0.0f;
	yInfo.penY = 0.0f;

	if (yInfo.move.y > 0.0f) {
		CollisionMapInfoDirection(
			base,
			yInfo,
			CollisionType::Top,
			{ kLeftTop, kRightTop },
			Vector3(0.0f, collisionEpsilon, 0.0f),
			width,
			height,
			blank,
			mapChipField,
			enableGoal
		);
	}

	if (yInfo.move.y < 0.0f) {
		CollisionMapInfoDirection(
			base,
			yInfo,
			CollisionType::Bottom,
			{ kLeftBottom, kRightBottom },
			Vector3(0.0f, -collisionEpsilon, 0.0f),
			width,
			height,
			blank,
			mapChipField,
			enableGoal
		);
	}

	// Y反映
	base.y += yInfo.move.y;
	info.move.y = yInfo.move.y;

	if (yInfo.hasNormal) {
		info.normal = info.normal + yInfo.normal;
		info.hasNormal = true;
	}

	info.ceiling = info.ceiling || yInfo.ceiling;
	info.landing = info.landing || yInfo.landing;
	info.penY = std::max(info.penY, yInfo.penY);

	if (yInfo.hasBreakBlock && !info.hasBreakBlock) {
		info.hasBreakBlock = true;
		info.breakBlockX = yInfo.breakBlockX;
		info.breakBlockY = yInfo.breakBlockY;
	}

	if (yInfo.hitDamageBlock) {
		info.hitDamageBlock = true;
	}

	if (yInfo.hitGoal) {
		info.hitGoal = true;
	}
}
