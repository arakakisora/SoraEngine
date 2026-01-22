#pragma once
#include "Vector3.h"
#include <memory>

class Object3D;

enum class EnemyDeathPhase {
	Rise,    // 少し浮き上がる
	PullIn,  // 目的地へ吸い込まれる
	Collapse,// 収束してフェード
	Finished
};

class EnemyDeathStage {
public:
	// デフォルトと調整用定数（公開して外部で使えるように）
	static inline constexpr float kDefaultRiseDuration     = 0.25f;
	static inline constexpr float kDefaultPullDuration     = 0.8f;
	static inline constexpr float kDefaultCollapseDuration = 0.5f;

	// 初期化。target は吸い込む先。duration は各フェーズの長さ（秒）
	void Initialize(Object3D* obj, const Vector3& target, float riseDur = kDefaultRiseDuration, float pullDur = kDefaultPullDuration, float collapseDur = kDefaultCollapseDuration);

	// 開始（初期値セット）
	void Begin();

	// 更新（呼び出し側は dt を渡す）
	void Update(float dt);
	// 終了判定
	bool IsFinished() const { return phase_ == EnemyDeathPhase::Finished; }

private:
	Object3D* object_ = nullptr;
	Vector3 startPos_;// 開始時の位置
	Vector3 targetPos_;// 吸い込む先の位置
	Vector3 startScale_;// 開始時のスケール
	float timer_ = 0.0f;// フェーズ内タイマー
	EnemyDeathPhase phase_ = EnemyDeathPhase::Rise;

	// durations
	float riseDuration_ = kDefaultRiseDuration;// 少し浮き上がる時間
	float pullDuration_ = kDefaultPullDuration;// 吸い込まれる時間
	float collapseDuration_ = kDefaultCollapseDuration;// 収束してフェードする時間

	// フェーズ挙動の調整定数（意味のある名前に
	static inline constexpr float kRiseHeight               = 0.5f;   // 上昇量（ユニット）
	static inline constexpr float kRiseScaleFactor         = 1.1f;   // 浮き上がり時のスケール倍率
	static inline constexpr float kPullRotateSpeed         = 0.4f;   // 吸い込まれる際の回転速度（rad/sec）
	static inline constexpr float kPullScaleStartFactor    = 1.1f;   // 吸い込み開始時のスケール倍率
	static inline constexpr float kPullScaleEndFactor      = 1.4f;   // 吸い込み終了時のスケール倍率
	static inline constexpr float kCollapseMinScale        = 0.01f;  // 収束時の最小スケール
	static inline constexpr float kFadeEpsilon            = 0.0f;   // フェード用開始値（明示）
	static inline constexpr float kFadeFull               = 1.0f;   // フェード用終了値（明示）
};