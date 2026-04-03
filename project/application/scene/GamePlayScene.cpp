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

#include "Easing.h"
#include <algorithm>


void GamePlayScene::Initialize()
{
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	//カメラの生成
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 15,13,-60, });
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

	const int stageIndex = SceneManager::GetInstance()->GetStageIndex();
	std::string stagePath;
	const int kAvailableMaps = 12; // Map1.csv .. Map12.csv

	if (stageIndex >= 0 && stageIndex < kAvailableMaps) {
		stagePath = "Resources/Mapdata/Map" + std::to_string(stageIndex + 1) + ".csv";
	}
	else {
		stagePath = "Resources/Mapdata/Map1.csv";
	}
	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->LoadMapChipCsv(stagePath);//testmap blocks.csv
	// ブロック生成
	generateBlock_.Initialize(mapChipField_.get());
	generateBlock_.GenerateObject3D();

	// --- プレイヤースポーン位置をマップから取得する ---
	Vector3 playerPostion = {};
	auto spawnPositions = mapChipField_->GetPositionBySpwan("player"); // 注意: 関数名はプロジェクトに合わせて 'GetPositionBySpwan'
	if (!spawnPositions.empty()) {
		// マップに player スポーンが複数ある場合は最初のものを使用
		playerPostion = spawnPositions.front();
	}
	else {
		// フォールバック: 既存の手打ち位置
		playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
	}
	//playerの生成
	player = std::make_unique<Player>();
	player->SetMapChipField(mapChipField_.get());
	player->Initialize(playerPostion);
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 13,6.85f,-32, });

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
	/*CameraManager::GetInstance()->GetCamera("maincam")->SetFollowTarget(player->GetObject3D(), { 8, 6, -20 });
	CameraManager::GetInstance()->GetCamera("maincam")->SetRotate({ 0.0f,0,0 });
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowBoundsEnabled(false);
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowBounds({ 4.0f, 5.0f, -100.0f }, { 47.0f, 20.0f, 50.0f });*/

	//ゴールの初期化
	goal = std::make_unique<Goal>();
	goal->Initialize(mapChipField_.get(), player.get());

	// ControlGuide の初期化
	ControlGuide::GetInstance()->Initialize(SpriteCommon::GetInstance());

	//ポーズメニュー
	pauseMenu = std::make_unique<PauseMenu>();
	pauseMenu->Initialize(Object3DCommon::GetInstance(), PauseType::GamePlayScene);
	pauseMenu->SetCamera(CameraManager::GetInstance()->GetCamera("maincam"));
}

void GamePlayScene::Finalize()
{

	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->RemoveCamera("debugcam");
	// ControlGuide の破棄
	ControlGuide::GetInstance()->Finalize();
	ControlGuide::DestroyInstance();
}

void GamePlayScene::UpdateGameLogic(float dt)
{
	

	goal->Update(player->GetGoal(), dt,player->GetObject3D()->GetTransform().translate);
	const float fixedDt = dt;
	Vector3 offset = { 0, 0, -20 };
	Vector3 playerPostion = {};
	auto spawnPositions = mapChipField_->GetPositionBySpwan("player"); 
	if (!spawnPositions.empty()) {
		// マップに player スポーンが複数ある場合は最初のものを使用
		playerPostion = spawnPositions.front();
	}
	if (isStageStartPlaying_ || goal->GetIsEffectStarted()) {
		CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(player->GetObject3D()->GetTransform().translate + offset);
		stageStartEffect_->Update(dt);
		if (stageStartEffect_->IsFinished()) {
			isStageStartPlaying_ = false;
			CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);

			Vector3 startPos = playerPostion + offset;
			Vector3 endPos = { 13, 6.85f, -32.0f };
			StartCameraBlend(startPos, endPos, 1.0f);
			
		}
		// 見た目の更新は UpdateObjects で行う（ここではロジックのみ）
		enemyManager_->EnemyObjectUpdate(); // 敵オブジェクト transform を更新（見た目）
	}
	else {

		if (isCameraBlending_) {
			UpdateCameraBlend(dt);
		}
		else {
			CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({ 13,6.85f,-32, });
		}

		// ゲーム進行系（ポーズ中は実行しない）
		if (!player->GetIsDead_()) {
			player->Update();
			// プレイヤーの弾などは player 内で管理される
		}
		enemyManager_->Update();

		// 衝突周りはゲームロジック
		CollisionManager::GetInstance()->Clear();
		enemyManager_->RegisterColliders();
		CollisionManager::GetInstance()->Update();

		generateBlock_.SyncBlockObjectsWithMap();
	}

	// プレイヤーが死んだらゲームオーバー演出
	if (player->GetIsDead_()) {
		gameOverEffect_->Update(dt);
		CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(true);
	}
	if (!gameOverEffect_->IsPlaying()) {
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}
}

void GamePlayScene::UpdateObjects(float dt)
{
	// オブジェクトの見た目更新はここで行う（ポーズ中でも継続）
	if (isStageStartPlaying_ || goal->GetIsEffectStarted()) {
		// ステージ開始演出でプレイヤーオブジェクトだけ別更新している形に合わせる
		player->GetObject3D()->Update();
		enemyManager_->EnemyObjectUpdate();
	}
	else {
		// 普通時も見た目の更新は行う（物理・ロジックは UpdateGameLogic 側）
		player->GetObject3D()->Update();
		enemyManager_->EnemyObjectUpdate();
	}

	// ブロックやパーティクルなどは常に更新
	generateBlock_.Update();

}

void GamePlayScene::StartCameraBlend(const Vector3& start, const Vector3& end, float duration)
{
	isCameraBlending_ = true;
	cameraBlendTimer_ = 0.0f;
	cameraBlendDuration_ = duration;
	cameraBlendStartPos_ = start;
	cameraBlendEndPos_ = end;

}

void GamePlayScene::UpdateCameraBlend(float dt)
{

	if (!isCameraBlending_) {
		return;
	}

	cameraBlendTimer_ += dt;

	float t = cameraBlendTimer_ / cameraBlendDuration_;
	t = std::clamp(t, 0.0f, 1.0f);

	Vector3 pos = Easing::EaseLerp(
		cameraBlendStartPos_,
		cameraBlendEndPos_,
		t,
		Easing::EaseOutSine
	);

	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(pos);

	if (t >= 1.0f) {
		isCameraBlending_ = false;
		CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(cameraBlendEndPos_);
	}

}

void GamePlayScene::Update()
{
	// Camera は常に現在の active camera を更新（ポーズ用カメラ切替後に反映されるよう、ここで行う）
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	// ポーズメニューの入力・イージングを先に処理（カメラ切替等を即時反映させるため）
	pauseMenu->Update();

	// フェードは常に更新
	fadeManager_.Update();

	// ポーズ中かどうかを判定
	const bool isPaused = pauseMenu->IsPaused();

	// 固定 dt
	const float dt = 1.0f / 60.0f;

	// ゲームロジックはポーズ時に止める
	if (!isPaused) {
		UpdateGameLogic(dt);
	}

	// オブジェクトの見た目更新は常に行う
	UpdateObjects(dt);


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
		if (!pauseMenu->IsPaused())
		{
			stageStartEffect_->Draw();
		} // ←ゲートのみ描画
		player->Draw();            
	}
	else {

		player->Draw();

	}
	//エネミーの描画o 
	enemyManager_->Draw();

	generateBlock_.Draw();

	pauseMenu->Draw();

	//object3D2nd->Draw();
	ParticleManager::GetInstance()->Draw();
	LineCommon::GetInstance()->Draw();
#pragma endregion

#pragma region スプライト描画

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	if (!pauseMenu->IsPaused()) {
		// ControlGuide をここで描画すると UI レイヤーで最前面に来ます
		ControlGuide::GetInstance()->Render();
	}
	/*sprite->Draw();*/
	fadeManager_.Draw();
	goal->Draw2D();

	if (isStageStartPlaying_) {
		if (!pauseMenu->IsPaused())
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
		auto spawnPositions = mapChipField_->GetPositionBySpwan("player");
		if (!spawnPositions.empty()) {
			// マップに player スポーンが複数ある場合は最初のものを使用
			playerPostion = spawnPositions.front();
		}
		else {
			// フォールバック: 既存の手打ち位置
			playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
		}
		player->GetObject3D()->SetTranslate(playerPostion);

	}

#ifdef USE_IMGUI
	ControlGuide::GetInstance()->DebugImGui();

	ImGui::Begin("Camera Menu");
	if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Switch to Main Camera")) {
			CameraManager::GetInstance()->SetActiveCamera("maincam");
			CameraManager::GetInstance()->GetActiveCamera()->SetFollowMode(false);
		}
		if (ImGui::Button("Switch to Sub Camera")) {
			CameraManager::GetInstance()->SetActiveCamera("debugcam");
			CameraManager::GetInstance()->GetActiveCamera()->SetFollowMode(false);
		}
	}
	ImGui::End();

	if (ImGui::Button("TITELEScene"))
	{
		SceneManager::GetInstance()->ChangeScene("TITELE");
	}

	ParticleManager::GetInstance()->ImguiDrawEditor();
#endif // USE_IMGUI

}

void GamePlayScene::Road()
{
	//3Dオブジェクト読み込み
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj");
	ModelManager::GetInstance()->LoadModel("player.obj");
	ModelManager::GetInstance()->LoadModel("blokc.obj");
	ModelManager::GetInstance()->LoadModel("skyplane.obj");
	ModelManager::GetInstance()->LoadModel("enemy.obj");
	ModelManager::GetInstance()->LoadModel("goal.obj");
	ModelManager::GetInstance()->LoadModel("bullet.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");
	ModelManager::GetInstance()->LoadModel("gate.obj");
	ModelManager::GetInstance()->LoadModel("unbreakableBlokc.obj");
	ModelManager::GetInstance()->LoadModel("damageblock.obj");
}

