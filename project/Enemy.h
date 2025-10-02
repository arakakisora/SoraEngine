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
class Enemy:public EnemyBase {

public:
	~Enemy();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize()override;

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update(MapChipField* mapChipField);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();


	Vector3 GetRayEndPosition();
	int GetRayMapChipNumber(MapChipField* mapChipField);



	// 当たり判定
	void OnCollision(const PlayerBullet* bullet);
	bool IsDead() const { return isDead_; }

	Vector3 GetWorldPosition()override;
	AABB GetAABB()override;

private:

	
	// 敵の動き
	static inline const float kWalkSpeed = 0.01f; // 歩行の速さ
	                   
	//敵のアニメーション
	static inline const float kWalkMotionAngleStart = 0.1f;//最初の角度
	static inline const float kWalkMotionAngleEnd = 0.5f;//最後の角度
	static inline const float kWalkMotionTime = 0.1f;//アニメーションの時間
	static inline const float kEnemyWidth = 0.8f;
	static inline const float kEnemyHeight = 0.8f;
	float walkTimer_ = 0.0f;

	
	float rotateY = 0.0f;
	Vector4 defaultColor_ = { 1, 1, 1, 1 }; // 通常時の色
	float damageTimer_ = 0.0f;
	static inline const float kDamageDisplayTime = 0.2f; // 赤くなる時間（秒）

	//撃破effect
	ParticleEmitter* deatheEffect = nullptr; // パーティクルエミッター
	EulerTransform effectPosition_ = { {0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }}; // エフェクトの位置
};
