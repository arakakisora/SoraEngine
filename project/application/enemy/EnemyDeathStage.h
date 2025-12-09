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
	// 初期化。target は吸い込む先。duration は各フェーズの長さ（秒）
	void Initialize(Object3D* obj, const Vector3& target, float riseDur = 0.25f, float pullDur = 0.8f, float collapseDur = 0.5f);

	// 開始（初期値セット）
	void Begin();

	// 更新（呼び出し側は dt を渡す）
	void Update(float dt);

	bool IsFinished() const { return phase_ == EnemyDeathPhase::Finished; }

private:
	Object3D* object_ = nullptr;
	Vector3 startPos_;
	Vector3 targetPos_;
	Vector3 startScale_;
	float timer_ = 0.0f;
	EnemyDeathPhase phase_ = EnemyDeathPhase::Rise;

	// durations
	float riseDuration_ = 0.25f;
	float pullDuration_ = 0.8f;
	float collapseDuration_ = 0.5f;
};