#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include <memory>
#include <FadeManager.h>
#include "GateInOut.h"

class Camera;
class GameClearScene :public BaseScene
{

public:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize()override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update()override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw()override;



public:


	std::unique_ptr<Sprite> sprite = nullptr;

	std::unique_ptr<Camera> camera = nullptr;

	FadeManager fadeManager_;

	std::unique_ptr<GateInOut> gate_;
	bool gateOutRequested_ = false;
	bool fadeOutRequested_ = false;


};

