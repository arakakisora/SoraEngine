#define NOMINMAX 
#include "EnemyDeathStage.h"
#include "Object3D.h"
#include "MyMath.h"
#include <array>
#include <functional>

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


	// フェーズ処理はテーブル駆動で記述
	timer_ += dt;

	using Handler = std::function<void(float)>;

	// フェーズ数 (Rise, PullIn, Collapse, Finished)
	constexpr size_t PHASE_COUNT = 4;

	// 各フェーズの処理をラムダで定義（this をキャプチャ）
	std::array<Handler, PHASE_COUNT> handlers{
		/* Rise */
		[this](float /*dt*/) {
			float t = std::clamp(timer_ / riseDuration_, 0.0f, 1.0f);
			float ease = MyMath::SmoothStep(EnemyDeathStage::kFadeEpsilon, EnemyDeathStage::kFadeFull, t);
			Vector3 p = startPos_;
			p.y += EnemyDeathStage::kRiseHeight * ease;
			object_->SetTranslate(p);
			Vector3 s = MyMath::Lerp(startScale_, startScale_ * EnemyDeathStage::kRiseScaleFactor, ease);
			object_->SetScale(s);

			if (t >= 1.0f) {
				phase_ = EnemyDeathPhase::PullIn;
				timer_ = 0.0f;
			}
		},
		/* PullIn */
		[this, dt](float /*unused*/) {
			float t = std::clamp(timer_ / pullDuration_, 0.0f, 1.0f);
			float ease = MyMath::SmoothStep(EnemyDeathStage::kFadeEpsilon, EnemyDeathStage::kFadeFull, t);
			Vector3 p = MyMath::Lerp(startPos_, targetPos_, ease);
			object_->SetTranslate(p);
			auto rot = object_->GetTransform().rotate;
			rot.y += EnemyDeathStage::kPullRotateSpeed * dt;
			object_->SetRotate(rot);
			Vector3 s = MyMath::Lerp(startScale_ * EnemyDeathStage::kPullScaleStartFactor, startScale_ * EnemyDeathStage::kPullScaleEndFactor, ease);
			object_->SetScale(s);

			if (t >= 1.0f) {
				phase_ = EnemyDeathPhase::Collapse;
				timer_ = 0.0f;
			}
		},
		/* Collapse */
		[this](float /*dt*/) {
			float t = std::clamp(timer_ / collapseDuration_, 0.0f, 1.0f);
			float ease = MyMath::SmoothStep(EnemyDeathStage::kFadeEpsilon, EnemyDeathStage::kFadeFull, t);
			Vector3 s = MyMath::Lerp(startScale_ * EnemyDeathStage::kPullScaleEndFactor, Vector3{EnemyDeathStage::kCollapseMinScale, EnemyDeathStage::kCollapseMinScale, EnemyDeathStage::kCollapseMinScale}, ease);
			object_->SetScale(s);
			auto col = object_->GetColor();
			col.w = std::max(0.0f, EnemyDeathStage::kFadeFull - ease);
			object_->SetColor(col);
			object_->SetTranslate(targetPos_);

			if (t >= 1.0f) {
				phase_ = EnemyDeathPhase::Finished;
			}
		},
		/* Finished */
		[this](float /*dt*/) {
			
		}
	};

	// 安全にインデックスして実行
	const size_t idx = static_cast<size_t>(phase_);
	if (idx < handlers.size()) {
		handlers[idx](dt);
	}
}