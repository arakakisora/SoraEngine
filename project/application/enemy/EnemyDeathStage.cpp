#define NOMINMAX 
#include "EnemyDeathStage.h"
#include "Object3D.h"
#include "MyMath.h"

void EnemyDeathStage::Initialize(Object3D* obj, const Vector3& target, float riseDur, float pullDur, float collapseDur)
{
	object_ = obj;
	targetPos_ = target;
	riseDuration_ = riseDur;
	pullDuration_ = pullDur;
	collapseDuration_ = collapseDur;
	phase_ = EnemyDeathPhase::Rise;
	timer_ = 0.0f;
	if (object_) {
		startPos_ = object_->GetTransform().translate;
		startScale_ = object_->GetTransform().scale;
	}
}

void EnemyDeathStage::Begin()
{
	timer_ = 0.0f;
	phase_ = EnemyDeathPhase::Rise;
}

void EnemyDeathStage::Update(float dt)
{
	if (!object_) return;

	timer_ += dt;

	switch (phase_) {
	case EnemyDeathPhase::Rise:
	{
		// 少し上に上げる（イージング）
		float t = std::clamp(timer_ / riseDuration_, 0.0f, 1.0f);
		float ease = MyMath::SmoothStep(0.0f, 1.0f, t);
		Vector3 p = startPos_;
		p.y += 0.5f * ease; // 上昇量は調整可
		object_->SetTranslate(p);
		// 少しスケールアップ
		Vector3 s = MyMath::Lerp(startScale_, startScale_ * 1.1f, ease);
		object_->SetScale(s);

		if (t >= 1.0f) {
			phase_ = EnemyDeathPhase::PullIn;
			timer_ = 0.0f;
		}
		break;
	}
	case EnemyDeathPhase::PullIn:
	{
		float t = std::clamp(timer_ / pullDuration_, 0.0f, 1.0f);
		// スムーズにターゲットへ移動
		float ease = MyMath::SmoothStep(0.0f, 1.0f, t);
		Vector3 p = MyMath::Lerp(startPos_, targetPos_, ease);
		object_->SetTranslate(p);
		// 少し回転しながら
		auto rot = object_->GetTransform().rotate;
		rot.y += 0.4f * dt;
		object_->SetRotate(rot);
		// 少し拡大してから収束（ここは視覚効果で調整可）
		Vector3 s = MyMath::Lerp(startScale_ * 1.1f, startScale_ * 1.4f, ease);
		object_->SetScale(s);

		if (t >= 1.0f) {
			phase_ = EnemyDeathPhase::Collapse;
			timer_ = 0.0f;
		}
		break;
	}
	case EnemyDeathPhase::Collapse:
	{
		float t = std::clamp(timer_ / collapseDuration_, 0.0f, 1.0f);
		float ease = MyMath::SmoothStep(0.0f, 1.0f, t);
		// 収束：スケールダウンとフェードアウト
		Vector3 s = MyMath::Lerp(startScale_ * 1.4f, Vector3{0.01f,0.01f,0.01f}, ease);
		object_->SetScale(s);
		// フェード（alpha を下げる）
		auto col = object_->GetColor();
		col.w = std::max(0.0f, 1.0f - ease);
		object_->SetColor(col);

		// 位置はターゲットに固定
		object_->SetTranslate(targetPos_);

		if (t >= 1.0f) {
			phase_ = EnemyDeathPhase::Finished;
		}
		break;
	}
	case EnemyDeathPhase::Finished:
	default:
		break;
	}
}