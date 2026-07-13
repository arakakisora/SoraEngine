#pragma once
#include "FadeManager.h"
#include "Player.h"
#include <Object3D.h>
#include <StageEndEffect.h>
#include <memory>

class MapChipField;
class Goal
{
  public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Goal() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(MapChipField* map, Player* player);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(bool isGoal, float deltaTime, Vector3 goalpos);

	/// <summary>
	/// 3D 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 2D 描画（UI やスクリーンフェードなど）
	/// </summary>
	void Draw2D();

	// Getter / Setter
	/// <summary>
	/// ゴールに到達したかどうかを取得
	/// </summary>
	bool GetIsGoal() const { return isGoal_; }

	/// <summary>
	/// ゴールに到達したかどうかを設定
	/// </summary>
	void SetIsGoal(bool isGoal) { isGoal_ = isGoal; }

	/// <summary>
	/// ゴールのエフェクト開始済みか取得
	/// </summary>
	bool GetIsEffectStarted() const { return isEffectStarted_; }

  private:
	static constexpr float kDefaultFadeDuration = 1.0f;

	// メンバ（処理コメント付き）
	bool isGoal_ = false; // ゴール到達フラグ

	std::unique_ptr<Object3D> object3D_; // ゴールの3Dオブジェクト（所有）

	Vector3 goalPosition_; // ゴールのワールド位置

	std::unique_ptr<StageEndEffect> stageClearEffect_;
	bool isEffectStarted_ = false; // エフェクト開始済みフラグ

	FadeManager fadeManager_;
	bool isFadeOutStarted_ = false; // フェードアウト開始フラグ
};
