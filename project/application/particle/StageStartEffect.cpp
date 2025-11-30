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

    // プレイヤー初期位置（最終的にここへ戻す）
    baseY_ = player_->GetTransform().translate.y;

    // 奥側から開始（奥=+Z 側。必要なら符号を入れ替えてください）
    playerStartPos_ = { basePos_.x, baseY_, basePos_.z + moveDistZ_ };
    auto tr = player_->GetTransform();
    tr.translate = playerStartPos_;
    player_->SetTransform(tr);
}

void StageStartEffect::Update(float dt) {
    if (isFinished_) return;
    timer_ += dt;

    if (timer_ < 1.0f) {
        float t = timer_ / 1.0f;
        // ゲートは相対開閉のままでOK
        gateLeft_->SetTranslate({ basePos_.x - (1.0f + t * 1.0f), basePos_.y, basePos_.z - 0.5f });
        gateRight_->SetTranslate({ basePos_.x + (1.0f + t * 1.0f), basePos_.y, basePos_.z - 0.5f });
    } else if (timer_ < 2.5f) {
        float t = (timer_ - 1.0f) / 1.5f;       // 0→1
        // Ease（滑らかにしたい場合。直線で良ければ t のままでOK）
        auto easeOutCubic = [](float x) { return 1.0f - powf(1.0f - x, 3.0f); };
        float u = easeOutCubic(std::clamp(t, 0.0f, 1.0f));

        auto tr = player_->GetTransform();
        tr.translate.x = basePos_.x;            // X/Yは固定（物理の揺れを抑制）
        tr.translate.y = baseY_;
        tr.translate.z = playerStartPos_.z + (basePos_.z - playerStartPos_.z) * u; // ← Lerp
        player_->SetTransform(tr);
    } else {
        // フレーム端数での誤差対策：最後に必ずピタッと初期位置へ
        auto tr = player_->GetTransform();
        tr.translate = { basePos_.x, baseY_, basePos_.z };
        player_->SetTransform(tr);

        isFinished_ = true;
    }

    gateLeft_->Update();
    gateRight_->Update();
}

void StageStartEffect::Draw() {
    gateLeft_->Draw();
    gateRight_->Draw();
}
