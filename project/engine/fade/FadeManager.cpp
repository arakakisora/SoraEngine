#define NOMINMAX 

#include "FadeManager.h"
#include "SpriteCommon.h"

void FadeManager::Initialize(const std::string& texture)
{
	// フェード用スプライトの初期化
    fadeSprite_ = std::make_unique<Sprite>();
    fadeSprite_->Initialize(SpriteCommon::GetInstance(), texture);
    fadeSprite_->SetSize(Vector2(1280, 720));
    fadeSprite_->SetPosition(Vector2(0, 0));
    fadeSprite_->SetAnchorPoint(Vector2(0, 0));
    fadeSprite_->setColor(Vector4(0, 0, 0, 0));
}

void FadeManager::StartFadeIn(float duration)
{
    isFadeIn_ = true;
    isFadeOut_ = false;
    timer_ = 0.0f;
    duration_ = duration;
    fadeSprite_->setColor(Vector4(0, 0, 0, 1.0f)); // 初期は黒
}

void FadeManager::StartFadeOut(float duration)
{
    isFadeOut_ = true;
    isFadeIn_ = false;
    isFadeOutFinished_ = false;
    timer_ = 0.0f;
    duration_ = duration;
    fadeSprite_->setColor(Vector4(0, 0, 0, 0.0f));
}

void FadeManager::Update()
{
	// フェード処理の更新
    if (!isFadeIn_ && !isFadeOut_) return;
	// タイマー更新
	const float kFrameRate = 1.0f / 60.0f;
    timer_ += kFrameRate;
	// フェード率計算
    float t = std::min(timer_ / duration_, 1.0f);
    float alpha = isFadeIn_ ? 1.0f - t : t;
    //α値をセット
    fadeSprite_->setColor(Vector4(0, 0, 0, alpha));
	// フェード完了判定
    if (t >= 1.0f) {
        if (isFadeOut_) {
            isFadeOut_ = false;
            isFadeOutFinished_ = true;
        }
        if (isFadeIn_) {
            isFadeIn_ = false;
        }
    }
	/// スプライト更新
    fadeSprite_->Update();
}

void FadeManager::Draw()
{
	// フェードスプライトの描画
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }
}
