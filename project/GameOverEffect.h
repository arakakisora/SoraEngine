#pragma once
#include "Object3D.h"
#include <numbers>

/// <summary>
/// ゲームオーバーエフェクト
/// </summary>
class Player;
class GameOverEffect
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="player"></param>
	void Initialize(Object3D* obj);
	/// <summary>
	/// 更新
	/// </summary>
	void Update(float dt);
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// エフェクト終了判定
	/// </summary>
	/// <returns></returns>
	
	bool IsPlaying() const { return isPlaying_; }

private:
	Object3D* playerobject3D_;
	bool isPlaying_ = false;
	float timer_ = 0.0f;
	Vector3 startpos_{};

	float jumpPower_ = 0.2f;
	float gravity_ = 0.01f;
	float forwardSpeed_ = 0.05f;


};

