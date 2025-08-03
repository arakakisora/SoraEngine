#pragma once

#include "assert.h"
#include <algorithm>
#include <numbers>
#include "MyMath.h"
#include"Object3D.h"
#include <MapChipField.h>
#include "ParticleEmitter.h"
#include "EnemyBase.h"


class Player;
class PlayerBullet;
class Enemy2 :public EnemyBase {
public:
	~Enemy2();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3D* obj, const Vector3& position)override;

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update(MapChipField* mapChipField)override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;

	bool IsDead()override { return isDead_; }
	Vector3 GetWorldPosition();
	AABB GetAABB();


	Vector3 GetRayEndPosition();
	int GetRayMapChipNumber(MapChipField* mapChipField);

	// Object3D解放用のメソッド
	void ReleaseObject3D() {
		delete object3D_;
		object3D_ = nullptr;
	}
	// 当たり判定
	void OnCollision(const PlayerBullet* bullet);



private:

	Object3D* object3D_ = nullptr;
	// 敵の動き
	static inline const float kWalkSpeed = 0.005f; // 歩行の速さ
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
	int HP = 10;
	float rotateY = 0.0f;
	Vector4 defaultColor_ = { 1, 1, 1, 1 }; // 通常時の色
	float damageTimer_ = 0.0f;
	static inline const float kDamageDisplayTime = 0.2f; // 赤くなる時間（秒）

	//撃破effect
	ParticleEmitter* deatheEffect = nullptr; // パーティクルエミッター
	EulerTransform effectPosition_ = { {0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } }; // エフェクトの位置
};
