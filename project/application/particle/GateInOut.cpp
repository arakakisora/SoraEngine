#define NOMINMAX 

#include "GateInOut.h"

void GateInOut::Initialize(SpriteCommon* spriteCommon,
    const char* leftTex,
    const char* rightTex) {
    gateL_ = std::make_unique<Sprite>();
    gateR_ = std::make_unique<Sprite>();

    gateL_->Initialize(spriteCommon, leftTex);
    gateR_->Initialize(spriteCommon, rightTex);

    SetScreenSize(screenW_, screenH_);

    // デフォルトは「開いてる」状態
    ApplyPositionsFromCloseAmount(0.0f);
}

void GateInOut::SetScreenSize(float w, float h) {
    screenW_ = w;
    screenH_ = h;

    gateH_ = screenH_;

    gateL_->SetSize({ gateW_, gateH_ });
    gateR_->SetSize({ gateW_, gateH_ });

    // 位置は現在の進行度に合わせて再適用
    float closeAmount = 0.0f;
    if (mode_ == Mode::kOut) closeAmount = EaseInOutCubic(progress01_);
    if (mode_ == Mode::kIn)  closeAmount = 1.0f - EaseInOutCubic(progress01_);
    ApplyPositionsFromCloseAmount(closeAmount);
}

void GateInOut::StartIn(float durationSec) {
    mode_ = Mode::kIn;
    bool holdClosed_ = false;
    bool lastWasOut_ = false;
    duration_ = std::max(0.01f, durationSec);
    timer_ = 0.0f;
    progress01_ = 0.0f;
    finished_ = false;

    // IN開始は「閉じた状態」から
    ApplyPositionsFromCloseAmount(1.0f);
}

void GateInOut::StartOut(float durationSec) {
    mode_ = Mode::kOut;
    bool holdClosed_ = false;
    bool lastWasOut_ = false;
    duration_ = std::max(0.01f, durationSec);
    timer_ = 0.0f;
    progress01_ = 0.0f;
    finished_ = false;

    // OUT開始は「開いた状態」から
    ApplyPositionsFromCloseAmount(0.0f);
}
void GateInOut::ApplyPositionsFromCloseAmount(float closeAmount01) {
    closeAmount01 = std::clamp(closeAmount01, 0.0f, 1.0f);


    const float leftOpenX = -gateW_;
    const float rightOpenX = screenW_;

    const float leftCloseX = screenW_ * 0.5f - gateW_;
    const float rightCloseX = screenW_ * 0.5f;

    float leftX = leftOpenX + (leftCloseX - leftOpenX) * closeAmount01;
    float rightX = rightOpenX + (rightCloseX - rightOpenX) * closeAmount01;

    gateL_->SetPosition({ leftX, 0.0f });
    gateR_->SetPosition({ rightX, 0.0f });
}

void GateInOut::Update(float dt) {
    if (mode_ == Mode::kNone) return;

    timer_ += dt;
    progress01_ = std::clamp(timer_ / duration_, 0.0f, 1.0f);

    float u = EaseInOutCubic(progress01_);

    if (mode_ == Mode::kOut) {
        // OUT: 開→閉（closeAmount 0→1）
        ApplyPositionsFromCloseAmount(u);
    }
    else if (mode_ == Mode::kIn) {
        // IN: 閉→開（closeAmount 1→0）
        ApplyPositionsFromCloseAmount(1.0f - u);
    }

    gateL_->Update();
    gateR_->Update();

    if (progress01_ >= 1.0f) {
        finished_ = true;

        if (lastWasOut_) {
            
            ApplyPositionsFromCloseAmount(1.0f);
            holdClosed_ = true;
        }
        else {
           
            ApplyPositionsFromCloseAmount(0.0f);
            holdClosed_ = false;
        }

        mode_ = Mode::kNone; 
    }
}

void GateInOut::Draw2D() {
    
    if (!IsPlaying() && !holdClosed_) return;
    gateL_->Draw();
    gateR_->Draw();
}