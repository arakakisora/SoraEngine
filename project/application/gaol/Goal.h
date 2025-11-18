#pragma once
#include <Object3D.h>
#include <StageEndEffect.h>
#include "Player.h"
#include "FadeManager.h"
class MapChipField;
class Goal
{
public:

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Goal() {
		if (object3D_) {
			delete object3D_;
			object3D_ = nullptr;
		}
	}
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(MapChipField* map,Player *paleyr);
	/// <summary>
	/// 更新
	/// </summary>
	void Update(bool isGoal, float deltaTime);
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	void Draw2D();

	// Getter Setter
	/// <summary>
	/// ゴールに到達したかどうかを取得します
	/// </summary>
	bool GetIsGoal() const { return isGoal_; }
	/// <summary>
	/// ゴールに到達したかどうかを設定します
	/// </summary>
	void SetIsGoal(bool isGoal) { isGoal_ = isGoal; }

	bool GetIsEffectStarted() const { return isEffectStarted_; }
private:
	bool isGoal_ = false; // ゴールに到達したかどうか
	Object3D* object3D_ = nullptr; // ゴールの3Dオブジェクト
	Vector3 goalPosition_; // ゴールの位置
	
	std::unique_ptr<StageEndEffect> stageClearEffect_;
	bool isEffectStarted_ = false;

	FadeManager fadeManager_;
	bool isFadeOutStarted_ = false;
};

