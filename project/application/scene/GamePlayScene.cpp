#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include "Input.h"
#include "TitleScene.h"
#include "CameraManager.h"
#include <ParticleMnager.h>
#include "ChargeBehabiaor.h"
#include "LineCommon.h"
#include "ControlGuide.h" 


void GamePlayScene::Initialize()
{
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	//カメラの生成	
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());

	debugCamera = std::make_unique<Camera>();
	debugCamera->SetRotate({ 0,0,0, });
	debugCamera->SetTranslate({ 15,13,-60, });
	CameraManager::GetInstance()->AddCamera("debugcam", debugCamera.get());

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	//モデルの読み込み
	Road();

	MapChipDatabase::GetInstance()->LoadJson("Resources/Data/MapChipTypes.json");
	// MapChipFiled
	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->LoadMapChipCsv("Resources/Mapdata/testmap1.csv");//testmap blocks.csv
	// ブロック生成
	generateBlock_.Initialize(mapChipField_.get());
	generateBlock_.GenerateObject3D();

	// --- プレイヤースポーン位置をマップから取得する ---
	Vector3 playerPostion = {};
	auto spawnPositions = mapChipField_->GetPositionBySpwan("player"); // 注意: 関数名はプロジェクトに合わせて 'GetPositionBySpwan'
	if (!spawnPositions.empty()) {
		// マップに player スポーンが複数ある場合は最初のものを使用
		playerPostion = spawnPositions.front();
	} else {
		// フォールバック: 既存の手打ち位置
		playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
	}
	//playerの生成	
	player = std::make_unique<Player>();
	player->SetMapChipField(mapChipField_.get());
	player->Initialize(playerPostion);
	player->SetDeathHeight(0.0f);
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ playerPostion.x,playerPostion.y,-10, });

	// スタート演出生成
	stageStartEffect_ = std::make_unique<StageStartEffect>();
	stageStartEffect_->Initialize(player->GetObject3D(), playerPostion);
	stageStartEffect_->Begin();
	isStageStartPlaying_ = true;

	//ゲームオーバー演出生成
	gameOverEffect_ = std::make_unique<GameOverEffect>();
	gameOverEffect_->Initialize(player->GetObject3D());


	//エネミー
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(mapChipField_.get());

	//3Dオブジェクトの初期化
	object3D2nd = std::make_unique<Object3D>();
	object3D2nd->Initialize(Object3DCommon::GetInstance());
	object3D2nd->SetModel("plane.obj");

	//フォローカメラ設定
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowTarget(player->GetObject3D(), { 0, 1, -12 });
	CameraManager::GetInstance()->GetCamera("maincam")->SetRotate({ 0.15f,0,0 });
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowBoundsEnabled(false);
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowBounds({ 4.0f, 5.0f, -100.0f }, { 47.0f, 20.0f, 50.0f });

	//ゴールの初期化
	goal = std::make_unique<Goal>();
	goal->Initialize(mapChipField_.get(), player.get());

	// ControlGuide の初期化
	ControlGuide::GetInstance()->Initialize(SpriteCommon::GetInstance());
}

void GamePlayScene::Finalize()
{

	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->RemoveCamera("debugcam");
	// ControlGuide の破棄
	ControlGuide::GetInstance()->Finalize();
	ControlGuide::DestroyInstance();
}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();



	goal->Update(player->GetGoal(), 1.0f / 60.0f);
	const float dt = 1.0f / 60.0f;
	if (isStageStartPlaying_ || goal->GetIsEffectStarted()) {

		stageStartEffect_->Update(1.0f / 60.0f);
		if (stageStartEffect_->IsFinished()) {
			isStageStartPlaying_ = false;
			CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(true);
		}
		player->GetObject3D()->Update();
		enemyManager_->EnemyObjectUpdate();
	} else {
		//player->Update();
		////プレイヤーの更新
		if (!player->GetIsDead_()) {
			player->Update();
			// プレイヤーの弾

		}
		enemyManager_->Update();



		// 毎フレーム、まずクリアしてから各オーナーに登録して Update を呼ぶ
		CollisionManager::GetInstance()->Clear();

		// プレイヤー自身と弾を登録（Player が担当）
		player->RegisterColliders();

		// 敵は EnemyManager が登録する
		enemyManager_->RegisterColliders();

		// 衝突判定実行
		CollisionManager::GetInstance()->Update();
		generateBlock_.SyncBlockObjectsWithMap();
	}
	//プレイヤーが死んだらゲームオーバーシーンに遷移
	if (player->GetIsDead_()) {

		gameOverEffect_->Update(dt);
		CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);


	}
	if (!gameOverEffect_->IsPlaying()) {
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}


	generateBlock_.Update();



#ifdef _DEBUG
	Imguidebug();

#endif // _DEBUG
}

void GamePlayScene::Draw()
{


	Object3DCommon::GetInstance()->SkinNingCommonDraw();

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	//SkyDome
	goal->Draw();

	if (isStageStartPlaying_) {
		stageStartEffect_->Draw(); // ←ゲートのみ描画
		player->Draw();            // ←プレイヤーを別に描画
	} else {

		player->Draw();

	}
	//エネミーの描画o 
	enemyManager_->Draw();

	generateBlock_.Draw();



	//object3D2nd->Draw();
	ParticleMnager::GetInstance()->Draw();
	LineCommon::GetInstance()->Draw();
#pragma endregion

#pragma region スプライト描画

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	// ControlGuide をここで描画すると UI レイヤーで最前面に来ます
	ControlGuide::GetInstance()->Render();
	/*sprite->Draw();*/
	fadeManager_.Draw();
	goal->Draw2D();

	if (isStageStartPlaying_) {

		stageStartEffect_->Draw2D();
	}
}

void GamePlayScene::Imguidebug()
{
	//マップ作製エディタ
	editor.Run();


	//マップチップエディターでリロードが押されたらマップチップを再読み込みして3Dオブジェクトを再生成する
	if (editor.GetReloadRequested() == true) {
		editor.SetReloadRequested(false);

		std::string filePath = "Resources/Mapdata/";
		filePath += editor.GetFileName();
		mapChipField_->LoadMapChipCsv(filePath);

		mapChipField_->LoadMapChipCsv(filePath);
		
		// 再生成
		generateBlock_.GenerateObject3D();


		enemyManager_.reset();
		enemyManager_ = std::make_unique<EnemyManager>();
		enemyManager_->Initialize(mapChipField_.get());


		// --- プレイヤースポーン位置をマップから取得する ---
		Vector3 playerPostion = {};
		auto spawnPositions = mapChipField_->GetPositionBySpwan("player"); // 注意: 関数名はプロジェクトに合わせて 'GetPositionBySpwan'
		if (!spawnPositions.empty()) {
			// マップに player スポーンが複数ある場合は最初のものを使用
			playerPostion = spawnPositions.front();
		} else {
			// フォールバック: 既存の手打ち位置
			playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
		}
		player->GetObject3D()->SetTranslate(playerPostion);

	}

#ifdef USE_IMGUI
	ControlGuide::GetInstance()->DebugImGui();

	if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Switch to Main Camera")) {
			CameraManager::GetInstance()->SetActiveCamera("maincam");
			CameraManager::GetInstance()->GetActiveCamera()->SetFollowMode(true);
		}
		if (ImGui::Button("Switch to Sub Camera")) {
			CameraManager::GetInstance()->SetActiveCamera("debugcam");
			CameraManager::GetInstance()->GetActiveCamera()->SetFollowMode(false);
		}
	}

	if (ImGui::Button("TITELEScene"))
	{
		SceneManager::GetInstance()->ChangeScene("TITELE");
	}
#endif // USE_IMGUI

}

void GamePlayScene::Road()
{
	//3Dオブジェクト読み込み
	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("axis.obj");
	ModelManager::GetInstans()->LoadModel("cube.obj");
	ModelManager::GetInstans()->LoadModel("player.obj");
	ModelManager::GetInstans()->LoadModel("blokc.obj");
	ModelManager::GetInstans()->LoadModel("skyplane.obj");
	ModelManager::GetInstans()->LoadModel("enemy.obj");
	ModelManager::GetInstans()->LoadModel("gool.obj");
	ModelManager::GetInstans()->LoadModel("bullet.obj");
	ModelManager::GetInstans()->LoadModel("sphere.obj");
	ModelManager::GetInstans()->LoadModel("gate.obj");
}

