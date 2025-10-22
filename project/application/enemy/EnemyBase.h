// EnemyBase.h
#pragma once
#include "Object3D.h"
#include "MapChipField.h"


class Player;
class PlayerBullet;
/// <summary>
/// EnemyBaseクラス
/// 敵の基底クラス
/// </summary>
class EnemyBase {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~EnemyBase() {}

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="obj"></param>
	/// <param name="position"></param>
	virtual void Initialize(Object3D* obj, const Vector3& position) = 0;
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="mapChipField"></param>
	virtual void Update(MapChipField* mapChipField) = 0;
	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;
	/// <summary>
	/// AABBを取得します
	/// </summary>
	virtual AABB GetAABB() = 0;
	/// <summary>
	/// 衝突時の処理
	/// </summary>
	virtual void OnCollision(const PlayerBullet* bullet) = 0;
	/// <summary>
	/// warld座標を取得します
	/// </summary>
	/// <returns></returns>
	virtual Vector3 GetWorldPosition() = 0;
	/// <summary>
	/// 死亡しているかどうかを取得
	/// </summary>
	virtual bool IsDead() const = 0;
	
};