#define NOMINMAX 

#include "FadeManager.h"
#include "SpriteCommon.h"

void FadeManager::Initialize(const std::string& texture)
{
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
    if (!isFadeIn_ && !isFadeOut_) return;

    timer_ += 1.0f / 60.0f;
    float t = std::min(timer_ / duration_, 1.0f);
    float alpha = isFadeIn_ ? 1.0f - t : t;

    fadeSprite_->setColor(Vector4(0, 0, 0, alpha));

    if (t >= 1.0f) {
        if (isFadeOut_) {
            isFadeOut_ = false;
            isFadeOutFinished_ = true;
        }
        if (isFadeIn_) {
            isFadeIn_ = false;
        }
    }

    fadeSprite_->Update();
}

void FadeManager::Draw()
{
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }
}
