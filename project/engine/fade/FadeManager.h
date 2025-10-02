#pragma once
#include <memory>
#include "Sprite.h"

class FadeManager
{
public:
    void Initialize(const std::string& texture = "white1x1.png");
    void StartFadeIn(float duration = 1.0f);
    void StartFadeOut(float duration = 1.0f);
    void Update();
    void Draw();
    bool IsFadeInFinished() const { return isFadeIn_ && timer_ >= duration_; }
    bool IsFadeOutFinished() const { return isFadeOutFinished_; }
    bool IsFading() const { return isFadeIn_ || isFadeOut_; }

private:
    std::unique_ptr<Sprite> fadeSprite_;
    bool isFadeIn_ = false;
    bool isFadeOut_ = false;
	float timer_ = 0.0f;// フェード時間
	float duration_ = 1.0f;// フェード時間
    bool isFadeOutFinished_ = false;
};
