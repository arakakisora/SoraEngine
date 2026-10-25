#pragma once
#include <cstdint>
#include "Vector3.h"

struct CollisionMapInfo {
	bool ceiling = false; // 天井衝突
	bool landing = false; // 着地
	bool hitWall = false; // 壁接触

	Vector3 move;         // 移動量
	Vector3 normal;       // 法線
	bool hasNormal = false;

	float penX = 0.0f; // 壁方向めり込み
	float penY = 0.0f; // 天井/床方向めり込み

	bool hasBreakBlock = false;
	uint32_t breakBlockX = 0;
	uint32_t breakBlockY = 0;

	bool hitDamageBlock = false;
	bool hitGoal = false;
};