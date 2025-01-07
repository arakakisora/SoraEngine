#pragma once
#include "BaseScene.h"
#include "Sprite.h"
class GameOverScene:public BaseScene
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

	//taitorusprite
	Sprite* sprite = nullptr;

};

