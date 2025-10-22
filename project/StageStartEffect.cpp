// StageStartEffect.cpp
#include "StageStartEffect.h"
#include <MyMath.h>
#include "Object3DCommon.h"

void StageStartEffect::Initialize(Object3D* player, const Vector3& basePos) {
    player_ = player;
    basePos_ = basePos;

    gateLeft_ = std::make_unique<Object3D>();
    gateRight_ = std::make_unique<Object3D>();
    gateLeft_->Initialize(Object3DCommon::GetInstance());
    gateRight_->Initialize(Object3DCommon::GetInstance());
    gateLeft_->SetModel("gate.obj");
    gateLeft_->SetLighting(false);
    gateRight_->SetModel("gate.obj");
	gateRight_->SetLighting(false);

    // 基準位置から相対配置（左右対称）
    gateLeft_->SetTranslate({ basePos_.x - 1.0f, basePos_.y, basePos_.z - 0.5f });
    gateRight_->SetTranslate({ basePos_.x + 1.0f, basePos_.y, basePos_.z - 0.5f });
}

void StageStartEffect::Begin() {
    timer_ = 0.0f;
    isFinished_ = false;
}

void StageStartEffect::Update(float dt) {
    if (isFinished_) return;
    timer_ += dt;

    // 0.0～1.0s: ゲートが開く
    if (timer_ < 1.0f) {
        float t = timer_ / 1.0f;  // 必要ならEase
        gateLeft_->SetTranslate({ basePos_.x - (1.0f + t * 1.0f), basePos_.y, basePos_.z - 0.5f });
        gateRight_->SetTranslate({ basePos_.x + (1.0f + t * 1.0f), basePos_.y, basePos_.z - 0.5f });
    }
    // 1.0～2.5s: プレイヤーが前進
    else if (timer_ < 2.5f) {
        float t = (timer_ - 1.0f) / 1.5f;         // 0→1
        auto tr = player_->GetTransform();
        tr.translate = { tr.translate.x, tr.translate.y, basePos_.z - t * 2.0f }; // ← 符号をマイナスに
        player_->SetTransform(tr);
    }
    else {
        isFinished_ = true;
    }

    gateLeft_->Update();
    gateRight_->Update();
}

void StageStartEffect::Draw() {
    gateLeft_->Draw();
    gateRight_->Draw();
}
