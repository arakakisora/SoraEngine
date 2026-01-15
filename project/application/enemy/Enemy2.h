#pragma once

#include "assert.h"
#include <algorithm>
#include <numbers>
#include "MyMath.h"
#include "Object3D.h"
#include <MapChipField.h>
#include "ParticleEmitter.h"
#include "Collider.h"
#include "HitDeathComponent.h"

#include "EnemyBase.h"



class Player;
class PlayerBullet;
/// <summary>
/// Enemyクラス
/// 敵の細かい部分を管理するクラス
/// </summary>
class Enemy2 : public EnemyBase {
public:

	~Enemy2();
	/// <summary>
	/// 初期化
	/// </summary>
	void  Initialize()override;

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update()override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw()override;


private:

	static inline const float kWalkSpeed = 0.01f; // 歩行の速さ

	//敵のアニメーション
	static inline const float kWalkMotionAngleStart = 0.1f;//最初の角度
	static inline const float kWalkMotionAngleEnd = 0.5f;//最後の角度
	static inline const float kWalkMotionTime = 0.1f;//アニメーションの時間

	float walkTimer_ = 0.0f;//歩行時間

	float rotateY = 0.0f;//回転角度
	float scale = 0.0f;//スケール

	//パアーティクルパラメータ
	float lifeTime = 1.0f;
	float currentTime = 1.0f;
	uint32_t maxParticles = 100;

};
