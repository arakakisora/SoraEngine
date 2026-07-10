#pragma once
#include "Input.h"
#include "Model.h"

#include "assert.h"
#include <algorithm>
#include <numbers>
#include "MyMath.h"

#include "RenderingData.h"

#include "Object3D.h"

#include <ParticleEmitter.h>
#include "StageStartEffect.h"
#include "Collider.h"
#include "Line.h"       
#include <memory>       
#include "MapChipField.h"

#include "CollisionMapInfo.h"

enum class LRTBDirecion {
	kRight,
	kLeft,
	kTop,
	kBottom
};

enum class PlayerState {
	hard,
	sticky,
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
	float kWidth = 0.4f;//当たり判定の幅
	float kHeight = 0.4f;//当たり判定の高さ
	float kBlank = 0.05f;//当たり判定の余裕
	float kCollisionEpsilon = 0.1f;//当たり判定の余裕
	//減衰パラメータ
	float kAttenuationLanding = 0.5f;//着地時の減衰率
	float kAttenuationWall = 0.1f;//壁に当たった時の減衰率
	// 振り向きパラメータ
	float kTimeTurn = 0.5; // 角度補間タイム

	struct DeathHeight {
		float min = 0.0f; // 落下死の高さ
		float max = 20.0f; // 落下死の有効/無効
	};
	struct Deathwidth {
		float min = -3.0f; // 落下死の高さ
		float max = 26.0f; // 落下死の有効/無効
	};
	DeathHeight deathHeight;
	Deathwidth deathwidth;
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

	void OnCollision(Collider* other) override;
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
	/// 衝突の法線をタイプから取得
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	static Vector3 NormalFromType(CollisionType type);
	/// <summary>
	/// 衝突の法線をタイプから取得
	/// </summary>
	/// <param name="info"></param>
	void Reflect(const CollisionMapInfo& info);

	//====================ライン描画=======================//
	/// <summary>
	/// プレイヤーの移動ライン描画
	/// </summary>
	void Playerline(); // プレイヤーの移動ライン描画
	/// <summary>
	/// プレイヤーの移動予測ライン描画
	/// </summary>
	void DrawPredictLine();
	/// <summary>
	/// ゴースト位置に次ターンの射出可能方向を表示
	/// </summary>
	void DrawGhostAimPreview(const Vector3& landingPos, const Vector3& nextFacingDir);
	/// <summary>
	/// Z軸回転でベクトルを回す
	/// </summary>
	Vector3 RotateVectorZ(const Vector3& v, float rad);
	/// <summary>
	/// 衝突情報から、実際のプレイヤーと同じ次ターン向きを取得
	/// </summary>
	Vector3 GetFacingDirFromCollisionInfo(const CollisionMapInfo& info);
	/// <summary>
	/// ベクトルを法線で反射させる
	/// </summary>
	/// <param name="v"></param>
	/// <param name="normal"></param>
	static void ReflectVelocity(Vector3& v, const Vector3& normal);
	/// <summary>
	/// 砲台の角度を更新
	/// </summary>
	/// <returns></returns>
	Vector3 MakeShotVelocity();

	//=====================ゴースト関連========================//
	void DrawGhost();
	void SetGhostPreview(const Vector3& landingPos, const Vector3& nextFacingDir);
	Vector3 GetGhostRotateFromFacingDir(const Vector3& facingDir);
	//=====================ゴースト関連========================//
	
	//====================ライン描画========================//

	/////////////======プレイヤーの動き======///////////////
	/// <summary>
	/// 自機の動き
	/// </summary>
	void PlayerMove(); // 自機の動き
	void PlayerCondition(const CollisionMapInfo& info);
	/// <summary>
	// 自機の振り向き
	/// </summary>
	void Playerdirection(const CollisionMapInfo& info);
	/// <summary>
	// 死亡条件しているかどうか
	/// </summary>
	void PlayerDeathTerms();



	/////////////======プレイヤーの動き======///////////////

	void PlayerParticle();

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
		const Vector3& basePos,
		const std::array<Vector3, 2>& posList,
		CollisionType type,
		CollisionMapInfo& info,
		bool enableGoal
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
		const Vector3& basePos,
		CollisionMapInfo& info,
		CollisionType dir,
		const std::array<Corner, 2>& checkCorners,
		const Vector3& offset,
		std::function<bool(const CollisionMapInfo&)> moveCondition,
		bool enableGoal
	);

	/// <summary>
	// map衝突判定
	/// </summary>
	void MapCollision(CollisionMapInfo& info);
	/// <summary>
	/// map衝突判定（位置指定版）
	/// </summary>
	/// <param name="position"></param>
	/// <param name="info"></param>
	/// <param name="enableGoal"></param>
	void MapCollisionAt(const Vector3& position, CollisionMapInfo& info, bool enableGoal = true);

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

	/// <summary>
	// プレイヤーステート取得
	/// </summary>
	PlayerState GetPlayerState() const { return playerState_; }


	/// <summary>
	/// プレイヤーの発射アニメーション
	/// </summary>
	void PlayerShotAnimation();

	//portal用
	bool TryPortalWarp(Vector3& position, Vector3& velocity, bool useCooldown);
	Vector3 RotateVelocityByPortal(
		const Vector3& velocity,
		const Vector3& inDir,
		const Vector3& outDir
	);

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
	LRTBDirecion direction_ = LRTBDirecion::kRight;// 振り向き

	//Animation
	Vector3 shotAnimationRotate_ = {};//発射アニメーションの回転
	float shotAnimationTimer_ = 0.0f; //発射アニメーションのタイマー
	bool isShotAnimation_ = false; //発射アニメーション中かどうか

	//振り向き
	float turnTimer_ = 0.0f;                    // 振り向き時間

	//マップ
	MapChipField* mapChipField_ = nullptr;

	//Particle
	float exhaustTimer_ = 0.0f;//パーティクルの間隔
	std::unique_ptr<ParticleEmitter> exhaustEmitter_;//排気ガスエミッター 
	std::unique_ptr<ParticleEmitter> deatheEffect;//排気ガスエミッター 

	//攻撃
	float cannonAngleDeg_ = 20.0f; // デフォルト仰角 20度
	std::unique_ptr<Line> line_; // 角度表示用ライン
	int32_t fireTimer = 0;

	//フラグ
	bool goal_ = false; // ゴールに到達したかどうか
	bool playerMoveRight_ = false;
	bool playerMoveLeft = false;
	bool isDead_ = false;//死んだ
	int hitCount = 0; // 衝突した数
	bool wasTouching_ = false;
	bool hitDamageBlock = false;
	int portalCooldown_ = 0;

	//定数
	static inline constexpr float kCannonAngleStepDeg = 2.0f; //大砲の角度定数
	static inline constexpr float kExhaustInterval = 1.0f / 15.0f; // 1/15秒ごとに出す
	static inline constexpr float kPi = std::numbers::pi_v<float>;//π
	static inline constexpr float kAimMinDeg = -60.0f;//仰角の上限
	static inline constexpr float kAimMaxDeg = 60.0f;//仰角の下限
	static inline constexpr float kStopEps = 1e-4f;//停止判定のための微小値

	Vector3 shotVel_{};
	bool hasShotVel_ = false;
	bool isStopped_ = false;
	bool isBarrierActive_ = false;

	//==================== 予測ゴースト ====================//
	std::unique_ptr<Object3D> ghostObject_;

	bool isGhostVisible_ = false;
	EulerTransform ghostTransform_{};

	// カメラシェイク
	float cameraShakeTimer_ = 0.0f;
	float cameraShakeDuration_ = 0.0f;
	float cameraShakePower_ = 0.0f;
	Vector3 cameraShakePrevOffset_ = { 0.0f, 0.0f, 0.0f };
	void StartCameraShake(float power, float duration);
	void UpdateCameraShake();
};
