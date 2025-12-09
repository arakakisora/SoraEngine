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


class Player;
class PlayerBullet;
/// <summary>
/// Enemyクラス
/// 敵の細かい部分を管理するクラス
/// </summary>
class Enemy2  :public Collider {
public:
	Enemy2() : Collider(Layer::Enemy2) {}
	AABB GetAABB() const override { return aabb_; }
	void OnCollision(Collider* other) override;
	void Setbulelt(PlayerBullet* b) { bullet_ = b; }
	AABB GetEnemyAABB();

	~Enemy2();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3D* obj, const Vector3& position);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update(MapChipField* mapChipField);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ワールド座標を取得します
	/// </summary>
	///<returns>vector3、ワールドポジションを返す</returns>
	Vector3 GetWorldPosition();

	/// <summary>
	/// 目の前にブロックがあるかどうか
	/// </summary>
	/// <returns>エネミー目の前にレイを出す向きに応じて代わる</returns>
	Vector3 GetRayEndPosition();

	/// <summary>
	/// レイの先のマップチップ番号を取得
	/// </summary>
	/// <returns>レイに当たってるマップチップ番号</returns>
	int GetRayMapChipNumber(MapChipField* mapChipField);

	/// <summary>
	// Object3D解放用のメソッド
	/// </summary>
	void ReleaseObject3D() {
		delete object3D_;
		object3D_ = nullptr;
	}

	/// <summary>
	// 死亡しているかどうかを取得
	/// </summary>
	/// <returns>死亡しているかどうか</returns>
	bool IsDead() const { return isDead_; }

	/// <summary>
	/// マネージャ用: 演出完了で削除可能か
	/// </summary>
	bool IsPendingRemove() const { return pendingRemove_; }

	Object3D* GetObject3D() const { return object3D_; }

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

	// 共通化したヒット・デスコンポーネント
	HitDeathComponent hitDeath_;
	bool pendingRemove_ = false; // マネージャ用
	AABB aabb_;
	PlayerBullet* bullet_ = nullptr;
};
