#include "StageEndEffect.h"
#include <MyMath.h>
#include "Object3DCommon.h"

void StageEndEffect::Initialize(Object3D* player, const Vector3& gatePos) {
	player_ = player;
	basePos_ = gatePos;

	gateL_ = std::make_unique<Object3D>();
	gateR_ = std::make_unique<Object3D>();

	gateL_->Initialize(Object3DCommon::GetInstance());
	gateR_->Initialize(Object3DCommon::GetInstance());
	gateL_->SetModel("gate");
	gateR_->SetModel("gate");
	gateL_->SetLighting(false);
	gateR_->SetLighting(false);

	// 初期位置（閉じた状態）
	gateL_->SetTranslate({ basePos_.x - 1.0f, basePos_.y, basePos_.z });
	gateR_->SetTranslate({ basePos_.x + 1.0f, basePos_.y, basePos_.z });

	// プレイヤーの最終位置（奥へ 2 進む）
	playerEndPos_ = { basePos_.x, gatePos.y, gatePos.z + 2.0f };
}

void StageEndEffect::Begin() {
	phase_ = EndPhase::Opening;
	timer_ = 0.0f;
}

void StageEndEffect::Update(float dt) {
	timer_ += dt;

	switch (phase_)
	{
	case EndPhase::Opening:     // 最初に開く
	{
		float t = std::min(timer_ / 1.0f, 1.0f);

		gateL_->SetTranslate({ basePos_.x - (1.0f + t * 1.0f), basePos_.y, basePos_.z });
		gateR_->SetTranslate({ basePos_.x + (1.0f + t * 1.0f), basePos_.y, basePos_.z });

		float u = t * t * t; // easeIn

		auto tr = player_->GetTransform();
		tr.translate.x = basePos_.x;
		tr.translate.y = basePos_.y;
		tr.translate.z = basePos_.z + (playerEndPos_.z - basePos_.z) * u;
		tr.rotate = { 0.0f, 3.0f, 0.0f };
		player_->SetTransform(tr);

		if (t >= 1.0f) {
			phase_ = EndPhase::PullIn;
			timer_ = 0.0f;
		}
		break;
	}

	case EndPhase::PullIn:      // 吸い込まれる
	{
		float t = std::min(timer_ / 1.5f, 1.0f);
		

		if (t >= 1.0f) {
			phase_ = EndPhase::Closing;
			timer_ = 0.0f;
		}
		break;
	}

	case EndPhase::Closing:     //最後に閉じる
	{
		float t = std::min(timer_ / 1.0f, 1.0f);

		// スタート演出の逆。開いた位置 → 閉じた位置へ戻る
		gateL_->SetTranslate({ basePos_.x - (2.0f - t * 1.0f), basePos_.y, basePos_.z });
		gateR_->SetTranslate({ basePos_.x + (2.0f - t * 1.0f), basePos_.y, basePos_.z });

		if (t >= 1.0f) {
			phase_ = EndPhase::Finished;
		}
		break;
	}
	}

	gateL_->Update();
	gateR_->Update();
}

void StageEndEffect::Draw() {
	gateL_->Draw();
	gateR_->Draw();
}
