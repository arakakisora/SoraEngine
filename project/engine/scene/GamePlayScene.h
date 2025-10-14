#pragma once

#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"

#include "SceneManager.h"
#include "MapChipField.h"


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
	void GenerateObject3D();

	////当たり判定のまとまり
	//void CheckAllCollisions();

	void Imguidebug();

	void Road();

public:

	


	//カメラのポインタ
	Camera* camera = nullptr;
	Camera* debugCamera = nullptr;
	//スプライトの初期化
	Object3D* object3D2nd = nullptr;
	//player
	Player* player = nullptr;
	Goal* goal = nullptr; // ゴールオブジェクト

	//wvpData用のTransform変数を作る
	EulerTransform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	EulerTransform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//mapchip
	//ブロック
	std::vector<std::vector<Object3D*>> blockobject3D;
	MapChipField* mapChipField_;

	//エネミー
	std::unique_ptr<EnemyManager> enemyManager_ = nullptr;


	// SkyDome
	Object3D* skydome_ = nullptr;

	//当たり判定
	std::unique_ptr<CollisionManager> collitionManager_ = nullptr;


	StageEditor editor;

	FadeManager fadeManager_;

	//debug用
#ifdef _DEBUG
	
#endif // _DEBUG

	
	
};

