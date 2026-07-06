#pragma once

#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"

#include "SceneManager.h"
#include "MapChipField.h"
#include "MapChipDatabase.h"

#include <vector>
#include <Player.h>

#include <ParticleEmitter.h>

#include <memory>
#include "CollisionManager.h"
#include "Goal.h"
#include "SkyBox.h"

#include "StageEditor.h"
#include "FadeManager.h"

#include "StageStartEffect.h"
#include "StageEndEffect.h"
#include "GameOverEffect.h"
#include "GenerateBlock.h"
#include <PauseMenu.h>

/// <summary>
/// ゲームプレイシーン
/// メインシーン
/// </summary>
class GamePlayScene :public BaseScene
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

	/// <summary>
	/// デバッグ用ImGui
	/// </summary>
	void Imguidebug();

	/// <summary>
	/// モデルをロードする
	/// </summary>
	void Road();

	//ステージreset
	void ResetStage();

private:
	// ゲームのロジック更新（ポーズ時は呼ばない）
	void UpdateGameLogic(float dt);
	// オブジェクト・見た目の更新（ポーズ中でも常に呼ぶ）
	void UpdateObjects(float dt);

	void StartCameraBlend(const Vector3& start, const Vector3& end, float duration);
	void UpdateCameraBlend(float dt);

public:

	//カメラのポインタ
	std::unique_ptr<Camera> camera = nullptr;
	std::unique_ptr<Camera> debugCamera = nullptr;
	//スプライトの初期化
	std::unique_ptr<Object3D> object3D2nd = nullptr;
	//player
	std::unique_ptr<Player> player = nullptr;
	std::unique_ptr<Goal> goal = nullptr; // ゴールオブジェクト

	//wvpData用のTransform変数を作る
	EulerTransform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	EulerTransform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//mapchip
	
	std::unique_ptr <MapChipField> mapChipField_;
	
	//エネミー
	//std::unique_ptr<EnemyManager> enemyManager_ = nullptr;

	//ブロック生成
	GenerateBlock generateBlock_;
	StageEditor editor;

	FadeManager fadeManager_;

	//スタート演出
	std::unique_ptr<StageStartEffect> stageStartEffect_;
	bool isStageStartPlaying_ = false;
	bool isCameraBlending_ = false;
	float cameraBlendTimer_ = 0.0f;
	float cameraBlendDuration_ = 1.0f;

	Vector3 cameraBlendStartPos_ = {};
	Vector3 cameraBlendEndPos_ = {};
	
	//ゲームオーバー演出
	std::unique_ptr<GameOverEffect> gameOverEffect_;

	std::unique_ptr<PauseMenu>pauseMenu;

	//debug用
#ifdef _DEBUG

#endif // _DEBUG



};

