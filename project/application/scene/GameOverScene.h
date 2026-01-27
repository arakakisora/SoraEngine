#pragma once
#include <memory>
#include "BaseScene.h"
#include "Sprite.h"
#include "FadeManager.h"

class Camera;

class GameOverScene : public BaseScene
{
public:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update() override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw() override;

public:
	//タイトル用スプライト
	std::unique_ptr<Sprite> sprite;

	//fade
	FadeManager fadeManager_;

	//カメラ
	std::unique_ptr<Camera> camera;
};

