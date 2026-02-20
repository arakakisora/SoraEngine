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
#include "StageStartEffect.h"
#include "Collider.h"
#include "Line.h"            // 追加: ライン描画
#include <memory>           // 追加: std::unique_ptr を使用

enum class LRDirecion {
	kright,
	kLeft,
};

enum class PlayerState {
	hard,
	sticky,
};

struct CollisionMapInfo {
	bool ceiling = false; // 天井衝突
	bool landing = false; // 着地
	bool hitWall = false; // 壁接触
	Vector3 move;         // 移動量
	Vector3 normal;       // 法線
	bool hasNormal = false;
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

struct PlayerParameter {
	//プレイヤーパラメータ
//speedパラメータ
	float kAcceleration = 0.02f;  // 定数加速度
	float kAttenuation = 0.9f;   // 速度減衰率
	float kLimitRunSpeed = 0.15f; // 最大速度制限
	//jannpパラメータ
	float kGravityAcceleration = 0.02f; // 重力加速度
	float kLimitFallSpeed = 1.0f;      // 最大落下速度
	float kJumpAcceleration = 0.3f;    // ジャンプ初速
	//当たり判定パラメータ
	float kWidth = 0.8f;//当たり判定の幅
	float kHeight = 0.8f;//当たり判定の高さ
	float kBlank = 2.0;//当たり判定の余裕
	float kCollisionEpsilon = 0.1f;//当たり判定の余裕
	//減衰パラメータ
	float kAttenuationLanding = 0.5f;//着地時の減衰率
	float kAttenuationWall = 0.1f;//壁に当たった時の減衰率
	// 振り向きパラメータ
	float kTimeTurn = 0.5; // 角度補間タイム
};


class Enemy;
class MapChipField;
/// <summary>
/// Playerクラス
/// </summary>
class Player :public Collider {

public:

	Player() : Collider(Layer::Player) {};

	AABB GetAABB() const override {
		return aabb_;
	}

	void OnCollision(Collider* other) override {
		switch (other->GetLayer()) {
		case Layer::Enemy:
			SetIsDead(true);
			break;
		case Layer::Enemy2:
			SetIsDead(true);
			break;
		default:
			break;
		}
	}
	AABB GetPlayerAABB();

	~Player() = default; // unique_ptr により自動解放

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position"></param>
	void Initialize(const Vector3& position);

	/// <summary>
	// 更新
	/// </summary>
	void Update();

	/// <summary>
	// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// 自機の動き
	/// </summary>
	void PlayerMove(); // 自機の動き
	/// <summary>
	/// 衝突の法線をタイプから取得
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	static Vector3 NormalFromType(CollisionType type);
	void Reflect(const CollisionMapInfo& info);

	/// <summary>
	/// プレイヤーの移動ライン描画
	/// </summary>
	void Playerline(); // プレイヤーの移動ライン描画

	/// <summary>
	// 自機の振り向き
	/// </summary>
	void PlayerTurn(); // 自機の振り向き

	void BulletUpdate();

	/// <summary>
	//攻撃
	/// </summary>
	void Attack();

	void PlayerParticle();

	/// <summary>
	/// イーズアウトサイン関数
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	float EaseOutSine(float x);

	/// <summary>
	/// 当たるブロックかどうか
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	bool IsHittableBlock(MapChipType type);
	/// <summary>
	/// 衝突判定の共通処理
	/// </summary>
	/// <param name="posList"></param>
	/// <param name="type"></param>
	/// <param name="info"></param>
	/// <returns></returns>
	bool CheckCollisionPoints(
		const std::array<Vector3, 2>& posList,
		CollisionType type,
		CollisionMapInfo& info
	);
	/// <summary>
	/// 衝突判定の共通処理
	/// </summary>
	/// <param name="info"></param>
	/// <param name="dir"></param>
	/// <param name="checkCorners"></param>
	/// <param name="offset"></param>
	/// <param name="moveCondition"></param>
	void CollisionMapInfoDirection(
		CollisionMapInfo& info,
		CollisionType dir,
		const std::array<Corner, 2>& checkCorners,
		const Vector3& offset,
		std::function<bool(const CollisionMapInfo&)> moveCondition
	);

	/// <summary>
	// map衝突判定
	/// </summary>
	void MapCollision(CollisionMapInfo& info);

	/// <summary>
	/// コーナーのワールド座標を取得
	/// </summary>
	/// <param name="center"></param>
	/// <param name="corner"></param>
	/// <returns></returns>
	Vector3 CornerPosition(const Vector3& centor, Corner corner);
	/// <summary>
	// プレイヤーの移動処理
	/// </summary>
	void PlayerCollisionMove(const CollisionMapInfo& inffo);
	/// <summary>
	/// 天井衝突時の移動処理
	/// </summary>
	/// <param name="info"></param>
	void CeilingCollisionMove(const CollisionMapInfo& info);
	/// <summary>
	// 着地時の移動処理
	/// </summary>
	/// <param name="info"></param>
	void LandingCollisionMove(const CollisionMapInfo& info);
	/// <summary>
	// 地面があるかどうか
	/// </summary>
	/// <param name="movedCenterPos"></param>
	bool HasGroundBelow(const Vector3& movedCenterPos);
	/// <summary>
	// 壁衝突時の移動処理
	/// </summary>
	/// <param name="info"></param>
	void HitWallCollisionMove(const CollisionMapInfo& info);
	/// <summary>
	/// コライダーの登録
	/// </summary>
	void RegisterColliders();

	//====================アクセッサ======================//
	///////////////======getter======///////////////
	/// <summary>
	// ゴールに到達したかどうか
	/// </summary>
	bool GetGoal() const { return goal_; }
	/// <summary>
	/// 右移動フラグを取得します
	/// </summary>
	/// <returns></returns>
	bool GetPrayerMoveRight() { return playerMoveRight_; }
	/// <summary>
	/// 左移動フラグを取得します
	/// </summary>
	/// <returns></returns>
	bool GetPrayerMoveLeft() { return playerMoveLeft; }
	/// <summary>
	/// Object3Dを取得します（所有は Player） 
	/// </summary>
	Object3D* GetObject3D() const { return object3D_.get(); }
	/// <summary>
	/// 現在の武器タイプを取得します
	/// </summary>
	/// <returns> </returns>
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }
	/// <summary>
	/// world座標を取得します
	/// </summary>
	Vector3 GetWorldPosition();
	/// <summary>
	// 死亡しているかどうかを取得
	/// </summary>
	/// <returns></returns>
	bool GetIsDead_() const { return isDead_; }
	/// <summary>
	// トランスフォーム取得
	///< / summary>
	///<returns>トランスフォームを返す</returns>
	const EulerTransform& GetTransform() { return object3D_->GetTransform(); }

	/// <summary>
	// 速度取得
	///< summary>
	const Vector3& GetVelocity() const { return velocity_; }
	///////////////======setter======///////////////
	/// <summary>
	/// 死亡しているかどうかを設定
	/// </summary>
	/// <param name="isDead"></param>
	void SetIsDead(bool isDead) { isDead_ = isDead; }
	/// <summary>
	/// 右移動フラグを設定します
	/// </summary>
	/// <param name="right"></param>
	void SetPrayerMoveRight(bool right) { playerMoveRight_ = right; }
	/// <summary>
	/// 左移動フラグを設定します
	/// </summary>
	/// <param name="left"></param>
	void SetPrayerMoveLeft(bool left) { playerMoveLeft = left; }
	/// <summary>
	// 落下死の高さを設定
	/// </summary>
	void SetDeathHeight(float height) { deathHeight_ = height; }
	/// <summary>
	// 速度設定
	/// </summary>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

private:

	std::unique_ptr<Object3D> object3D_;//Player3Dオブジェクト
	PlayerState playerState_ = PlayerState::hard;//プレイヤーステート
	Vector3 playerPosition_ = {};// プレイヤーの位置
	Vector3 velocity_ = {};// 速度
	PlayerParameter parameter_;// プレイヤーパラメータ
	AABB aabb_;// 当たり判定用AABB
	LRDirecion lrDirection_ = LRDirecion::kright;// 振り向き
	float deathHeight_; // 落下死の高さ

	//振り向き
	float turnFirstRotationY_ = 0.0f;           // 現在の向き
	float turnTimer_ = 0.0f;                    // 振り向き時間

	//マップ
	MapChipField* mapChipField_ = nullptr;

	//Particle
	float exhaustTimer_ = 0.0f;//パーティクルの間隔

	//攻撃
	float cannonAngleDeg_ = 20.0f; // デフォルト仰角 20度
	std::unique_ptr<Line> line_; // 角度表示用ライン
	WeaponType currentWeaponType_ = WeaponType::Gatling; // 現在の武器タイプ
	std::list<std::unique_ptr<PlayerBullet>> bullets_;//弾
	int32_t fireTimer = 0;

	

	//フラグ
	bool goal_ = false; // ゴールに到達したかどうか
	bool playerMoveRight_ = false;
	bool playerMoveLeft = false;
	bool isDead_ = false;//死んだ
	int hitCount = 0; // 衝突した数
	bool wasTouching_ = false;
	
	//定数
	static inline constexpr float kCannonAngleStepDeg = 2.0f; //大砲の角度定数
	static inline constexpr float kExhaustInterval = 1.0f / 15.0f; // 1/15秒ごとに出す
	static inline constexpr float kPi = std::numbers::pi_v<float>;//π
	static inline constexpr float kAimMinDeg = -45.0f;//仰角の上限
	static inline constexpr float kAimMaxDeg = 45.0f;//仰角の下限
};
