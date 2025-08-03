#pragma once
#include "Input.h"
#include "Model.h"

#include "assert.h"
#include <algorithm>
#include <numbers>
#include "MyMath.h"

#include "RenderingData.h"

#include "Object3D.h"
#include <PlayerBullet.h>
#include <ParticleEmitter.h>

enum class LRDirecion {
	kright,
	kLeft,
};

struct CollisionMapInfo {

	bool ceiling = false; // 天井衝突
	bool landing = false; // 着地
	bool hitWall = false; // 壁接触
	Vector3 move;         // 移動量
};

enum Corner {
	kRightBottom,
	kLeftBottom,
	kRightTop,
	kLeftTop,
	kNumCorner // 要素数
};

enum class CollisionType {
	Top,
	Bottom,
	Left,
	Right
};

enum class WeaponType {
	Gatling,
	Cannon,
};

struct PlayerParameter {
	//プレイヤーパラメータ
//speedパラメータ
	float kAccleration = 0.01f;  // 定数加速度
	float kAttenuation = 0.2f;   // 速度減衰率
	float kLimitRunSpeed =1.0f; // 最大速度制限
	//jannpパラメータ
	float kGravityAccleration = 0.05f; // 重力加速度
	float kLimitFallSpeed = 1.0f;      // 最大落下速度
	float kJampAcceleration = 0.5f;    // ジャンプ初速
	float kJampBlockAcceleration = 0.8f;//ジャンプブロックのジャンプ初速
	float kAccumulateJumpTime_ = 0.2f;   //溜め時間
	//当たり判定パラメータ
	float kWidth = 0.8f;//当たり判定の幅
	float kHeight = 0.8f;//当たり判定の高さ
	float kBlank = 1.0;//当たり判定の余裕
	float kCollisionsmallnumber = 0.1f;//当たり判定の余裕
	//減衰パラメータ
	float kAttenuationLanding = 0.1f;//着地時の減衰率
	float kAttenuationWall = 0.1f;//壁に当たった時の減衰率
	// 振り向きパラメータ
	float KtimeTurn = 0.5f; // 角度補間タイム
};


class Enemy;
class MapChipField;
class Player {

public:
	// 初期化
	void Initialize( const Vector3& position);

	~Player();

	// 更新
	void Update();

	// 描画
	void Draw();

	void PrayerMove(); // 自機の動き
	void PrayerTurn(); // 自機の振り向き

	//攻撃
	void Attack();

	float EaseOutSine(float x);
	const EulerTransform& GetTransform() { return object3D_->GetTransform(); }
	const Vector3& GetVelocity() const { return velocity_; }
	void SetMapChipField(MapChipField* mapChipFild) { mapChipFild_ = mapChipFild; }


	bool CheckCollisionPoints(
		const std::array<Vector3, 2>& posList,
		CollisionType type,
		CollisionMapInfo& info
	);

	void CollisionMapInfoDirection(
		CollisionMapInfo& info,
		CollisionType dir,
		const std::array<Corner, 2>& checkCorners,
		const Vector3& offset,
		std::function<bool(const CollisionMapInfo&)> moveCondition
	);

	bool IsHittableBlock(MapChipType type);

	void MapCollision(CollisionMapInfo& info); //ポインタ関数std::functionを使って、当たり判定の方向を指定する

	

	// map衝突判定

	Vector3 CornerPosition(const Vector3& centor, Corner corner);
	void PlayerCollisionMove(const CollisionMapInfo& inffo);
	void CeilingCollisionMove(const CollisionMapInfo& info);
	void OnGroundSwitching(const CollisionMapInfo& info);
	void HitWallCollisionMove(const CollisionMapInfo& info);

	// 当たり判定
	

	Vector3 GetWorldPosition();
	AABB GetAABB();
	//void OnCollision(const Enemy*enemy);

	bool GetIsDead_() const { return isDead_; }
	void SetIsDead(bool isDead) { isDead_ = isDead; }

	// 落下死の高さを設定
	void SetDeathHeight(float height) { deathHeight_ = height; }

	// Getter
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }
	Object3D* GetObject3D() const { return object3D_; }

	bool GetPrayerMoveRight() { return playermoveright; }
	bool GetPrayerMoveLeft() { return playermoveleft; }
	void SetPrayerMoveRight(bool right) { playermoveright = right; }
	void SetPrayerMoveLeft(bool left) { playermoveleft = left; }

	// ゴールに到達したかどうか
	bool GetGoal() const { return goal_; }

	//get足らんスレート
	//const Vector3& GetTranslate() const { return object3D_->GetTransform().translate; }

	int GetHP() const { return hp_; }
	void SetHP(int hp) { hp_ = hp; }

	int GetLevel() const { return level_; }
	void SetLevel(int level) { level_ = level; }

	void TakeDamage(int damage); // ダメージ処理
	bool CanTakeDamage() const;//クールタイムじゃないか調べる
private:
	
	//objec3D
	Object3D *object3D_=nullptr;

	//バレットオブジェクト
	Object3D* object3DBullet_ = nullptr;
	
	PlayerParameter playerParameter_; // プレイヤーパラメータ
	
	Vector3 velocity_ = {};                          // 速度
	
	// 振り向き
	LRDirecion lrDirection_ = LRDirecion::kright;
	float turnFirstRotationY_ = 0.0f;           // 現在の向き
	float turnTimer_ = 0.0f;                    // 振り向き時間
	static inline const float KtimeTurn = 0.5f; // 角度補間タイム
	// ジャンプ
	bool onGround_ = true;                                 // 接点状態フラグ
	
	
	    // ジャンプ初速
	// 当たり判定
	MapChipField* mapChipFild_ = nullptr;
	
	
	

	//死んだ
	bool isDead_ = false;
	int hp_ = 100; // プレイヤーのHP
	int damageCoolTimer_ = 0; // ダメージクールタイム用タイマー
	static const int kDamageCoolTime = 60; // 無敵時間：60フレーム（1秒相当）

	int level_ = 1; // プレイヤーのレベル

	//落下死高さ
	float deathHeight_; // 落下死の高さ

	//弾
	WeaponType currentWeaponType_ = WeaponType::Gatling; // 現在の武器タイプ
	std::list<PlayerBullet* > bullets_;
	int32_t fireTimer = 0;

	// プレイヤー移動フラグ
	bool playermoveright = false;
	bool playermoveleft = false;

	bool goal_ = false; // ゴールに到達したかどうか

	//プレイヤーパーティクル
	ParticleEmitter* dashparticleEmitter_ = nullptr; // プレイヤーのパーティクルエミッター
	ParticleEmitter* jumpParticleEmitter_ = nullptr; // ジャンプのパーティクルエミッター
	
};
