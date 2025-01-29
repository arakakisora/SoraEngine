#pragma once

#include "assert.h"
#include <algorithm>
#include <algorithm>
#include <numbers>
#include "MyMath.h"
#include"Object3D.h"
#include <MapChipField.h>

 
class Player;
class PlayerBullet;
class Enemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3D *obj, const Vector3& position);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update(MapChipField* mapChipField);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	Vector3 GetWorldPosition();
	AABB GetAABB();
	void OnCollision(const Player* player);
	Vector3 GetRayEndPosition();
	int GetRayMapChipNumber( MapChipField* mapChipField);



	// Object3D解放用のメソッド
	void ReleaseObject3D() {
		delete object3D_;
		object3D_ = nullptr;
	}
	// 当たり判定
	void OnCollision(const PlayerBullet* bullet);
	bool IsDead() const { return isDead_; }



private:
	
	Object3D* object3D_ = nullptr;
	// 敵の動き
	static inline const float kWalkSpeed = 0.01f; // 歩行の速さ
	Vector3 velocity_ = {};                      // 速度
	//敵のアニメーション
	static inline const float kWalkMotionAngleStart = 0.1f;//最初の角度
	static inline const float kWalkMotionAngleEnd = 0.5f;//最後の角度
	static inline const float kWalkMotionTime = 0.1f;//アニメーションの時間
	static inline const float kEnemyWidth = 0.8f;
	static inline const float kEnemyHeight = 0.8f;
	float walkTimer_ = 0.0f;

	//death
	bool isDead_ = false;
	
	float rotateY = 0.0f;
};


