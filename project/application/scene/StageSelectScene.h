#pragma once
#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"
#include <FadeManager.h>
#include "application\\ui\\PauseMenu.h"

struct StageObject {
	std::unique_ptr<Object3D> object;
	Vector3 basePos;  // 並べる基準位置
};

class StageSelectScene :public BaseScene
{

public:

	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize()override;
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

	void SelectMove();

private:

	std::unique_ptr<Camera> camera1;

	int MaxSelectIndex_ = 3;	// 最大ステージ数
	int currentIndex_ = 0;	// 現在のステージ
	float deltaTime_ = 1.0f / 60.0f;	// デルタタイム

	std::vector<StageObject> stages_;
	float scrollOffset_ = 0.0f; // 補間用のオフセット
	float inrerval_ = 4.0f; // ステージ間の距離

	FadeManager fadeManager_;	// 背景

	std::unique_ptr<Object3D>playerobj;

	// PauseMenu を追加
	PauseMenu pauseMenu_;
	
};

