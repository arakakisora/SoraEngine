#pragma once

#include "Sprite.h"
#include "MyMath.h"
//ゲームUI 
class UI
{
public:
	/// <summary>
	/// UIの初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// UIの終了処理
	/// </summary>
	void Update();
	/// <summary>
	/// UIの更新
	/// </summary>
	void Draw();
	/// <summary>
	/// UIの描画
	/// </summary>
	void ImguiDebug();


private:

	Sprite spaceSprite_;
	Sprite mouseSprite_;





};

