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

	//当たり判定のまとまり
	void CheckAllCollisions();

public:

	Object3D* GoolObject3D = nullptr;

	//カメラのポインタ
	Camera* camera = nullptr;
	//スプライトの初期化

	
	Object3D* object3D2nd = nullptr;
	

	//player
	Player* player = nullptr;
	Object3D* object3DPlayer = nullptr;

	//wvpData用のTransform変数を作る
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//mapchip
	//ブロック
	std::vector<std::vector<Object3D*>> blockobject3D;
	MapChipField* mapChipField_;

	//Enemy
	std::list<Enemy*> enemies_;
	//Object3D* object3DEnemy = nullptr;
	int32_t enemynumber = 3;

	// SkyDome
	Object3D* skydome_ = nullptr;
	
	
};

