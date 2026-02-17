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
	float kTimeTurn = 1.0f; // 角度補間タイム
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
	void PrayerMove(); // 自機の動き

	/// <summary>
	// 自機の振り向き
	/// </summary>
	void PrayerTurn(); // 自機の振り向き

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
	// トランスフォーム取得
	///< / summary>
	///<returns>トランスフォームを返す</returns>
	const EulerTransform& GetTransform() { return object3D_->GetTransform(); }

	/// <summary>
	// 速度取得
	///< summary>
	const Vector3& GetVelocity() const { return velocity_; }

	/// <summary>
	// 速度設定
	/// </summary>
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	bool IsHittableBlock(MapChipType type);

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
	void OnGroundSwitching(const CollisionMapInfo& info);
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
	/// world座標を取得します
	/// </summary>
	Vector3 GetWorldPosition();

	/// <summary>
	// 死亡しているかどうかを取得
	/// </summary>
	/// <returns></returns>
	bool GetIsDead_() const { return isDead_; }

	/// <summary>
	/// 死亡しているかどうかを設定
	/// </summary>
	/// <param name="isDead"></param>
	void SetIsDead(bool isDead) { isDead_ = isDead; }

	/// <summary>
	// 落下死の高さを設定
	/// </summary>
	void SetDeathHeight(float height) { deathHeight_ = height; }

	// Getter

	/// <summary>
	/// 現在の武器タイプを取得します
	/// </summary>
	/// <returns> </returns>
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }

	/// <summary>
	/// Object3Dを取得します（所有は Player） 
	/// </summary>
	Object3D* GetObject3D() const { return object3D_.get(); }

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
	// ゴールに到達したかどうか
	/// </summary>
	bool GetGoal() const { return goal_; }

	/// <summary>
	// ゴールに到達したかどうかを設定します
	/// </summary>
	void StartDirection();

	void SetOnGround(bool onground) { onGround_ = onground; }
	// 自分と弾を CollisionManager に登録する
	void RegisterColliders();
private:

	
	std::unique_ptr<Object3D> object3D_;
	Vector3 playerPosition_ = {};
	Vector3 velocity_ = {};                          // 速度
	PlayerParameter parameter_;// プレイヤーパラメータ

	// 数学的定数
	static inline constexpr float kPi = std::numbers::pi_v<float>;

	// 振り向き
	LRDirecion lrDirection_ = LRDirecion::kright;
	float turnFirstRotationY_ = 0.0f;           // 現在の向き
	float turnTimer_ = 0.0f;                    // 振り向き時間
	
	// ジャンプ
	bool onGround_ = true;                                 // 接点状態フラグ
	
	// 当たり判定
	MapChipField* mapChipField_ = nullptr;
	

	//死んだ
	bool isDead_ = false;

	//落下死高さ
	float deathHeight_; // 落下死の高さ

	//弾
	WeaponType currentWeaponType_ = WeaponType::Gatling; // 現在の武器タイプ
	std::list<std::unique_ptr<PlayerBullet>> bullets_;
	int32_t fireTimer = 0;

	// プレイヤー移動フラグ
	bool playerMoveRight_ = false;
	bool playerMoveLeft = false;
	// Player.h の private:
	float exhaustTimer_ = 0.0f;
	static inline constexpr float kExhaustInterval = 1.0f / 15.0f; // 1/15秒ごとに出す
	bool goal_ = false; // ゴールに到達したかどうか
	AABB aabb_;
	//大砲の角度（度単位）と調整ステップ
	float cannonAngleDeg_ = 20.0f; // デフォルト仰角 20度
	static inline constexpr float kCannonAngleStepDeg = 2.0f; // 1回あたりの変更量（度）

	// ライン描画用
	std::unique_ptr<Line> line_; // 角度表示用ライン

};
