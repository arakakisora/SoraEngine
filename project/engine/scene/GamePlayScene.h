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
#include <Enemy.h>
#include <ParticleEmitter.h>

#include "EnemyManager.h"	
#include <memory>
#include "CollisionManager.h"


#include "Goal.h"
#include "SkyBox.h"

#include "StageEditor.h"
#include "FadeManager.h"

#include "StageStartEffect.h"
#include "StageEndEffect.h"
#include "GameOverEffect.h"

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


	/// ブロックの生成

	/// <summary>
	/// マップのブロックを配置します
	/// </summary>
	void GenerateObject3D();

	////当たり判定のまとまり
	//void CheckAllCollisions();

	/// <summary>
	/// デバッグ用ImGui
	/// </summary>
	void Imguidebug();

	/// <summary>
	/// モデルをロードする
	/// </summary>
	void Road();

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
	//ブロック
	std::vector<std::vector<Object3D*>> blockobject3D;
	MapChipField* mapChipField_;
	/*MapChipDatabase mapChipDatabase_;*/

	//エネミー
	std::unique_ptr<EnemyManager> enemyManager_ = nullptr;


	// SkyDome
	Object3D* skydome_ = nullptr;

	


	StageEditor editor;

	FadeManager fadeManager_;

	//スタート演出
	std::unique_ptr<StageStartEffect> stageStartEffect_;
	bool isStageStartPlaying_ = false;
	
	//ゲームオーバー演出
	std::unique_ptr<GameOverEffect> gameOverEffect_;

	

	//debug用
#ifdef _DEBUG

#endif // _DEBUG



};

