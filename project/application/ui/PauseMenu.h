#pragma once
#include "Object3D.h"
#include "Object3DCommon.h"
#include "Input.h"
#include "Camera.h"
#include <Audio.h>
#include "RenderingData.h"
#include <functional>
#include <string>
#include <vector>

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
	EulerTransform texttransform[8]{};

	uint32_t textindex = 0;

	//ポーズ開く
	bool isPaused_ = false;

	//イージングタイマー
	float easeTimer_ = 0.0f;

	bool easingsceneFlag_ = false;
	bool easingmoveFlag_ = false;

	bool isClosing_ = false;

	std::unique_ptr<Camera> pouseCamera_;

	// データ駆動：メニュー項目
	struct MenuItem {
		std::string modelPath;
		std::function<void()> action;
		// 描画・イージングパラメータ（個別に調整可）
		float maxScale = 1.0f;       // 非選択時の最大スケール（easedValue をクリップ）
		float selectedScale = 1.0f;  // 選択時の明示的スケール（ease 完了時）
		float offsetY = 1.0f;        // 項目間の垂直間隔（この値は Initialize 時に override 可能）
	};

	std::vector<MenuItem> menuItems_;
};

