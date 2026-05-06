// StageStartEffect.cpp
#include "StageStartEffect.h"
#include <MyMath.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Easing.h"
#include <algorithm>
#include <cmath>

void StageStartEffect::Initialize(Object3D* player, const Vector3& basePos) {
    player_ = player;
    basePos_ = basePos;

    gateLeft_ = std::make_unique<Object3D>();
    gateRight_ = std::make_unique<Object3D>();
    gateLeft_->Initialize(Object3DCommon::GetInstance());
    gateRight_->Initialize(Object3DCommon::GetInstance());

    gateLeft_->SetModel("gate.obj");
    gateRight_->SetModel("gate.obj");
    gateLeft_->SetLighting(false);
    gateRight_->SetLighting(false);

    // ゲートはプレイヤーの「後ろ側」に置く
    const float gateZOffset = 2.0f;

    gateLeft_->SetTranslate({ basePos_.x - 1.0f, basePos_.y, basePos_.z + gateZOffset });
    gateRight_->SetTranslate({ basePos_.x + 1.0f, basePos_.y, basePos_.z + gateZOffset });

    gameOrder = std::make_unique<Sprite>();
    gameOrder->Initialize(SpriteCommon::GetInstance(), "Resources/gameorder.png");
}

void StageStartEffect::Begin() {
    timer_ = 0.0f;
    isFinished_ = false;

    baseY_ = player_->GetTransform().translate.y;

    // 奥側から開始
    playerStartPos_ = { basePos_.x, baseY_, basePos_.z + moveDistZ_ };

    auto tr = player_->GetTransform();
    tr.translate = playerStartPos_;

    player_->SetTransform(tr);
}

void StageStartEffect::Update(float dt) {
    if (isFinished_) {
        return;
    }

    timer_ += dt;

    const float gateClosedOffset = 1.0f;
    const float gateOpenAdd = 1.0f;
    const float gateZOffset = 2.0f;

    const float openTime = 1.0f;
    const float moveTime = 1.0f;
    const float waitTime = 0.3f;
    const float closeTime = 1.0f;
    const float finishWaitTime = 0.5f;   // ← 閉じたあと少し残す

    const float openEnd = openTime;
    const float moveEnd = openEnd + moveTime;
    const float waitEnd = moveEnd + waitTime;
    const float closeEnd = waitEnd + closeTime;
    const float finishEnd = closeEnd + finishWaitTime;

    if (timer_ < openEnd) {
        // 扉を開く
        float t = std::clamp(timer_ / openTime, 0.0f, 1.0f);
        float u = Easing::EaseOutCubic(t);

        float offset = Easing::Lerp(gateClosedOffset, gateClosedOffset + gateOpenAdd, u);

        gateLeft_->SetTranslate({
            basePos_.x - offset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + offset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
    }
    else if (timer_ < moveEnd) {
        // プレイヤーが前に出る
        float t = std::clamp((timer_ - openEnd) / moveTime, 0.0f, 1.0f);
        float u = Easing::EaseOutCubic(t);

        auto tr = player_->GetTransform();
        tr.translate.x = basePos_.x;
        tr.translate.y = baseY_;
        tr.translate.z = Easing::Lerp(playerStartPos_.z, basePos_.z, u);
        player_->SetTransform(tr);

        const float openedOffset = gateClosedOffset + gateOpenAdd;
        gateLeft_->SetTranslate({
            basePos_.x - openedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + openedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
    }
    else if (timer_ < waitEnd) {
        // 少し待つ
        auto tr = player_->GetTransform();
        tr.translate = { basePos_.x, baseY_, basePos_.z };
        player_->SetTransform(tr);

        const float openedOffset = gateClosedOffset + gateOpenAdd;
        gateLeft_->SetTranslate({
            basePos_.x - openedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + openedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
    }
    else if (timer_ < closeEnd) {
        // 扉を閉じる
        float t = std::clamp((timer_ - waitEnd) / closeTime, 0.0f, 1.0f);
        float u = Easing::EaseInCubic(t);

        float offset = Easing::Lerp(gateClosedOffset + gateOpenAdd, gateClosedOffset, u);

        gateLeft_->SetTranslate({
            basePos_.x - offset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + offset,
            basePos_.y,
            basePos_.z + gateZOffset
            });

        auto tr = player_->GetTransform();
        tr.translate = { basePos_.x, baseY_, basePos_.z };
        player_->SetTransform(tr);
    }
    else if (timer_ < finishEnd) {
        // 閉じた状態で少し残す
        gateLeft_->SetTranslate({
            basePos_.x - gateClosedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + gateClosedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });

        auto tr = player_->GetTransform();
        tr.translate = { basePos_.x, baseY_, basePos_.z };
        player_->SetTransform(tr);
    }
    else {
        // 完了
        gateLeft_->SetTranslate({
            basePos_.x - gateClosedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });
        gateRight_->SetTranslate({
            basePos_.x + gateClosedOffset,
            basePos_.y,
            basePos_.z + gateZOffset
            });

        auto tr = player_->GetTransform();
        tr.translate = { basePos_.x, baseY_, basePos_.z };
        player_->SetTransform(tr);

        isFinished_ = true;
    }

    gateLeft_->Update();
    gateRight_->Update();

    float alpha = 0.0f;
    if (timer_ >= 0.0f && timer_ < moveEnd) {
        const float period = 0.5f;
        float phase = std::fmod(timer_, period);
        bool visible = phase < (period * 0.5f);
        alpha = visible ? 1.0f : 0.0f;
    }
    else {
        alpha = 0.0f;
    }

    gameOrder->setColor({ 1.0f, 1.0f, 1.0f, alpha });
    gameOrder->Update();
}

void StageStartEffect::Draw() {
    gateLeft_->Draw();
    gateRight_->Draw();
}

void StageStartEffect::Draw2D() {
    gameOrder->Draw();
}