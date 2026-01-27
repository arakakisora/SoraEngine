#pragma once
#include "Object3D.h"
#include "Object3DCommon.h"
#include "Input.h"
#include "Camera.h"
#include <Audio.h>
#include "RenderingData.h"

class Camera; // 前方宣言

enum class PauseType {
	StageSelectScene,
	GamePlayScene
};

class PauseMenu
{
public:
	//初期化
	void Initialize(Object3DCommon* object3dcommon, PauseType type);
	//更新
	void Update();
	//描画
	void Draw();
public:
	bool IsPaused()const;

	void SetCamera(Camera* camera) {
		camera_ = camera;
	}

	// コントローラー操作
	void ControllerUpdate();
	// イージング移動
	void PausedStartGamePlay();
	// イージング移動
	void PausedStartStageSelect();

private:
	PauseType pauseType_;
	Object3DCommon* object3dcommon_;
	Camera* camera_ = nullptr;
	EulerTransform transform;
	Input* input;
	//ポーズ画面のオブジェクト
	std::unique_ptr<Object3D>object;

	// テキストオブジェクトのリスト
	std::vector<std::unique_ptr<Object3D>> TextObjects;
	EulerTransform texttransform[3]{};

	uint32_t textindex = 0;

	//ポーズ開く
	bool isPaused_ = false;

	//イージングタイマー
	float easeTimer_ = 0.0f;

	bool easingsceneFlag_ = false;
	bool easingmoveFlag_ = false;

	bool isClosing_ = false;

	std::unique_ptr<Camera> pouseCamera_;

	
};

