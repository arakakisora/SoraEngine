#pragma once
#include <memory>
#include "Sprite.h"

/// <summary>
/// フェードマネージャー
/// </summary>
class FadeManager
{
public:
    /// <summary>
	/// 初期化
    /// </summary>
    /// <param name="texture"></param>
    void Initialize(const std::string& texture = "white1x1.dds");
	/// <summary>
	/// フェードイン開始
	/// </summary>
	/// <param name="duration"></param>
    void StartFadeIn(float duration = 1.0f);
	/// <summary>
	/// フェードアウト開始
	/// </summary>
    void StartFadeOut(float duration = 1.0f);
    /// <summary>
	/// 更新
    /// </summary>
    void Update();
    /// <summary>
	/// 描画
    /// </summary>
    void Draw();
    /// <summary>
	/// フェードインが終了したか
    /// </summary>
    /// <returns></returns>
    bool IsFadeInFinished() const { return isFadeIn_ && timer_ >= duration_; }
	/// <summary>
	/// フェードアウトが終了したか
	/// </summary>
	/// <returns></returns>
    bool IsFadeOutFinished() const { return isFadeOutFinished_; }
	/// <summary>
	/// フェード中か
	/// </summary>
	/// <returns></returns>
    bool IsFading() const { return isFadeIn_ || isFadeOut_; }

private:
    std::unique_ptr<Sprite> fadeSprite_;
    bool isFadeIn_ = false;
    bool isFadeOut_ = false;
	float timer_ = 0.0f;// フェード時間
	float duration_ = 1.0f;// フェード時間
    bool isFadeOutFinished_ = false;
};
