#pragma once

#include <array>

#include "CollisionMapInfo.h"
#include "MapChipField.h"
#include "MapCollisionTypes.h"
#include "Vector3.h"


class MapCollisionSystem {
public:
	/// <summary>
	/// 当たるブロックかどうか
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	static bool IsHittableBlock(MapChipType type);

	/// <summary>
	/// 衝突の法線をタイプから取得
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	static Vector3 NormalFromType(CollisionType type);

	/// <summary>
	/// コーナーのワールド座標を取得
	/// </summary>
	/// <param name="center"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <param name="corner"></param>
	/// <returns></returns>
	static Vector3 CornerPosition(
		const Vector3& center,
		float width,
		float height,
		Corner corner
	);
	/// <summary>
	/// 衝突判定の共通処理
	/// </summary>
	/// <param name="basePos"></param>
	/// <param name="posList"></param>
	/// <param name="type"></param>
	/// <param name="info"></param>
	/// <param name="mapChipField"></param>
	/// <param name="enableGoal"></param>
	/// <returns></returns>
	static bool CheckCollisionPoints(
		const Vector3& basePos,
		const std::array<Vector3, 2>& posList,
		CollisionType type,
		CollisionMapInfo& info,
		MapChipField* mapChipField,
		float width,
		float height,
		float blank,
		bool enableGoal
	);

	/// <summary>
	/// 衝突判定の共通処理
	/// </summary>
	/// <param name="basePos"></param>
	/// <param name="info"></param>
	/// <param name="dir"></param>
	/// <param name="checkCorners"></param>
	/// <param name="offset"></param>
	/// <param name="playerWidth"></param>
	/// <param name="playerHeight"></param>
	/// <param name="mapChipField"></param>
	/// <param name="enableGoal"></param>
	static void CollisionMapInfoDirection(
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
	);
	/// <summary>
	/// map衝突判定
	/// </summary>
	/// <param name="position"></param>
	/// <param name="info"></param>
	/// <param name="playerWidth"></param>
	/// <param name="playerHeight"></param>
	/// <param name="blank"></param>
	/// <param name="mapChipField"></param>
	/// <param name="enableGoal"></param>
	static void CheckCollisionAt(
		const Vector3& position,
		CollisionMapInfo& info,
		float width,
		float height,
		float blank,
		float collisionEpsilon,
		MapChipField* mapChipField,
		bool enableGoal = true
	);


};