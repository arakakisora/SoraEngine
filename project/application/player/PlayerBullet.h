#pragma once
#include "Model.h"
#include "Object3D.h"
#include <MapChipField.h>
#include "Collider.h"

/// <summary>
/// プレイヤーの弾クラス
/// </summary>
class PlayerBullet : public Collider {

public:
	PlayerBullet() : Collider(Layer::PlayerBullet) {}
	~PlayerBullet();  // デストラクタ

	AABB GetAABB() const override { return aabb_; }
	AABB GetBulletAABB();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3D* obj, const Vector3& potition, const Vector3& velocity, MapChipField* mapChipField);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	// 当たったときコールバック
	void OnCollision(Collider* other) override;
	/// </summary>

	//Getter
	/// <summary>
	/// 死亡しているかどうかを取得
	/// </summary>
	bool GetIsDead() const { return isDead_; }

	/// <summary>
	/// warld座標を取得します
	/// </summary>
	/// <returns></returns>
	Vector3 GetWorldPosition();

	/// <summary>
	/// AABBを取得します
	/// </summary>
	
	/// <summary>
	// レイを出してマップチップ番号を取得
	/// </summary>
	Vector3 GetRayEndPosition();
	/// <summary>
	// レイの先のマップチップ番号を取得
	/// </summary>
	int GetRayMapChipNumber(MapChipField* mapChipField);
	/// <summary>
	/// 弾の威力を取得します
	/// </summary>
	/// <returns></returns>
	int GetPower() const { return power_; }
	/// <summary>
	/// 弾の威力を設定します	
	/// </summary>
	void SetPower(int p) { power_ = p; }

	Object3D* Getobject3DBullet_() { return object3D_; }


private:
	AABB aabb_;
	Object3D* object3D_ = nullptr;
	// textureHandle
	//uint32_t textureHandle_ = 0u;
	//速度
	Vector3 velocity_;
	//寿命
	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;//デスタイマー
	bool isDead_ = false;			//デスフラグ
	MapChipField* mapChipField_ = nullptr;
	int power_ = 1; // 弾の威力
};
