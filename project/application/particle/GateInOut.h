#pragma once
#include <memory>
#include <algorithm>
#include <cmath>

#include "Sprite.h"
#include "SpriteCommon.h"

// nextScene切替なし：ただの「被せ演出」
// - IN  : 画面中央から開いていく（開始は閉じた状態）
// - OUT : 画面左右から閉じていく（開始は開いた状態）
class GateInOut {
public:
    enum class Mode { kNone, kIn, kOut };

    void Initialize(SpriteCommon* spriteCommon,
                    const char* leftTex  = "Resources/gateL.png",
                    const char* rightTex = "Resources/gateR.png");

    void SetScreenSize(float w, float h);

    // 画面を閉じた状態から → 開く
    void StartIn(float durationSec = 0.5f);

    // 画面を開いた状態から → 閉じる
    void StartOut(float durationSec = 0.5f);

    void Update(float dt);
    void Draw2D();

    bool IsPlaying() const { return mode_ != Mode::kNone; }
    bool IsFinished() const { return finished_; }
    float GetProgress01() const { return progress01_; } // 0→1

    void HoldClosed(bool hold) { holdClosed_ = hold; }

private:
    static float EaseInOutCubic(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return (t < 0.5f) ? 4.0f * t * t * t
                          : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
    }

    // 実座標反映
    void ApplyPositionsFromCloseAmount(float closeAmount01);

private:
    std::unique_ptr<Sprite> gateL_;
    std::unique_ptr<Sprite> gateR_;

    float screenW_ = 1280.0f;
    float screenH_ = 720.0f;

    // ゲート画像のサイズ（片側）
    float gateW_ = 640.0f;
    float gateH_ = 720.0f;

    Mode mode_ = Mode::kNone;

    float timer_ = 0.0f;
    float duration_ = 0.5f;

    bool finished_ = true;
    float progress01_ = 0.0f; // 0→1

    bool holdClosed_ = false;
    bool lastWasOut_ = false;
};