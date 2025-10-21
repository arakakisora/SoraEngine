// StageStartEffect.cpp
#include "StageStartEffect.h"
#include <MyMath.h>
#include "Object3DCommon.h"

void StageStartEffect::Initialize(Object3D* player) {
    gateLeft_ = std::make_unique<Object3D>();
	gateLeft_->Initialize(Object3DCommon::GetInstance());

    gateRight_ = std::make_unique<Object3D>();
	gateRight_->Initialize(Object3DCommon::GetInstance());

    gateLeft_->SetModel("plane.obj");
    gateRight_->SetModel("plane.obj");
   
	this->player_ = player;

    // 初期位置（ゲートは閉じた状態）
    gateLeft_->SetTranslate({ player->GetTransform().translate.x - 2.0f, 0.0f, 0.0f});
    gateRight_->SetTranslate({ 2.0f, 0.0f, 0.0f });
   
}

void StageStartEffect::Update() {
    timer_ += 1.0f / 60.0f; // 1フレームあたりの時間加算

    // ①ゲート開く (0.0～1.0秒)
    if (timer_ < 1.0f) {
        float t = timer_ / 1.0f;
        gateLeft_->SetTranslate({ -2.0f - t * 2.0f, 0.0f, 0.0f });
        gateRight_->SetTranslate({ 2.0f + t * 2.0f, 0.0f, 0.0f });
    }
    // ②プレイヤー前進 (1.0～2.5秒)
    else if (timer_ < 2.5f) {
        float t = (timer_ - 1.0f) / 1.5f;
        player_->SetTranslate({ 0.0f, 0.0f, -5.0f + t * 5.0f });
    }
    // ③演出終了
    else {
        isFinished_ = true;
    }

    gateLeft_->Update();
    gateRight_->Update();
    player_->Update();
}

void StageStartEffect::Draw() {
    gateLeft_->Draw();
    gateRight_->Draw();
    player_->Draw();
}
