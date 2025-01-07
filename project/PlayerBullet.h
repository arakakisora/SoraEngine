#pragma once
#include "Model.h"
#include "Object3D.h"


class PlayerBullet {

public:



	/// 初期化
	void Initialize(Object3D*obj, const Vector3& potition, const Vector3& velocity);

	/// 更新
	void Update();

	/// 描画
	void Draw();

	// 当たったときコールバック
	void OnCollison();

	//Getter
	bool GetIsDead() const { return isDead_; }
	Vector3 GetWorldPosition(); 
	// ワールド座標取得
	AABB GetAABB();


private:

	Object3D* object3D_ = nullptr;
	// textureHandle
	//uint32_t textureHandle_ = 0u;
	//速度
	Vector3 velocity_;
	//寿命
	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;//デスタイマー
	bool isDead_ = false;			//デスフラグ
};
