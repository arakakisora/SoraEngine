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
#include <ParticleManager.h>
#include "ChargeBehavior.h"
#include "LineCommon.h"

#include "Easing.h"
#include <algorithm>

#include "UIeditor.h"

void GamePlayScene::Initialize()
{
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	//カメラの生成
	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0,0,0, });
	camera_->SetTranslate(cameraPos_);
	CameraManager::GetInstance()->AddCamera("maincam", camera_.get());

	debugCamera_ = std::make_unique<Camera>();
	debugCamera_->SetRotate({ 0,0,0, });
	debugCamera_->SetTranslate(cameraPos_);
	CameraManager::GetInstance()->AddCamera("debugcam", debugCamera_.get());

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	//モデルの読み込み
	Load();

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
	mapChipField_->LoadMapChipCsv(stagePath);
	// ブロック生成
	generateBlock_.Initialize(mapChipField_.get());
	generateBlock_.GenerateObject3D();

	// --- プレイヤースポーン位置をマップから取得する ---
	Vector3 playerPosition = {};
	auto spawnPositions = mapChipField_->GetPositionBySpawn("player"); 
	if (!spawnPositions.empty()) {
		// マップに player スポーンが複数ある場合は最初のものを使用
		playerPosition = spawnPositions.front();
	}
	else {
		// フォールバック: 既存の手打ち位置
		playerPosition = mapChipField_->GetMapChipPositionByIndex(6, 18);
	}
	//playerの生成
	player_ = std::make_unique<Player>();
	player_->SetMapChipField(mapChipField_.get());
	player_->Initialize(playerPosition);
	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({cameraEndPos_});

	// スタート演出生成
	stageStartEffect_ = std::make_unique<StageStartEffect>();
	stageStartEffect_->Initialize(player_->GetObject3D(), playerPosition);
	stageStartEffect_->Begin();
	isStageStartPlaying_ = true;

	//ゲームオーバー演出生成
	gameOverEffect_ = std::make_unique<GameOverEffect>();
	gameOverEffect_->Initialize(player_->GetObject3D());


	////エネミー
	//enemyManager_ = std::make_unique<EnemyManager>();
	//enemyManager_->Initialize(mapChipField_.get());

	//3Dオブジェクトの初期化
	object3D2nd_ = std::make_unique<Object3D>();
	object3D2nd_->Initialize(Object3DCommon::GetInstance());
	object3D2nd_->SetModel("plane");

	

	//ゴールの初期化
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(mapChipField_.get(), player_.get());

	

	//ポーズメニュー
	pauseMenu_ = std::make_unique<PauseMenu>();
	pauseMenu_->Initialize(Object3DCommon::GetInstance(), PauseType::GamePlayScene);
	pauseMenu_->SetCamera(CameraManager::GetInstance()->GetCamera("maincam"));

	UIeditor::GetInstance()->SetScene("GamePlay");
}

void GamePlayScene::Finalize()
{

	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->RemoveCamera("debugcam");
	
}

void GamePlayScene::UpdateGameLogic(float dt)
{
	

	goal_->Update(player_->GetGoal(), dt,player_->GetObject3D()->GetTransform().translate);
	const float fixedDt = dt;
	Vector3 offset = { 0, 0, -20 };
	Vector3 playerPostion = {};
	auto spawnPositions = mapChipField_->GetPositionBySpawn("player"); 
	if (!spawnPositions.empty()) {
		// マップに player スポーンが複数ある場合は最初のものを使用
		playerPostion = spawnPositions.front();
	}
	if (isStageStartPlaying_ || goal_->GetIsEffectStarted()) {
		CameraManager::GetInstance()->GetActiveCamera()->SetTranslate(player_->GetObject3D()->GetTransform().translate + offset);
		stageStartEffect_->Update(dt);
		if (stageStartEffect_->IsFinished()) {
			isStageStartPlaying_ = false;
			CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);

			Vector3 startPos = playerPostion + offset;
			StartCameraBlend(startPos, cameraEndPos_, 1.0f);
			
		}
		// 見た目の更新は UpdateObjects で行う（ここではロジックのみ）
	}
	else {

		if (isCameraBlending_) {
			UpdateCameraBlend(dt);
		}
		else {
			CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({cameraEndPos_});
		}

		// ゲーム進行系（ポーズ中は実行しない）
		if (!player_->GetIsDead_()) {
			player_->Update();
			UIeditor::GetInstance()->SetCount(
				"GamePlay",
				"ShotCount",
				player_->GetRemainingShots()
			);
			
		}
		//enemyManager_->Update();

		// 衝突周りはゲームロジック
		CollisionManager::GetInstance()->Clear();
		//enemyManager_->RegisterColliders();
		CollisionManager::GetInstance()->Update();

		generateBlock_.SyncBlockObjectsWithMap();
	}

	// プレイヤーが死んだらゲームオーバー演出
	if (player_->GetIsDead_()) {
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
	if (isStageStartPlaying_ || goal_->GetIsEffectStarted()) {
		// ステージ開始演出でプレイヤーオブジェクトだけ別更新している形に合わせる
		player_->GetObject3D()->Update();
		//enemyManager_->EnemyObjectUpdate();
	}
	else {
		// 普通時も見た目の更新は行う（物理・ロジックは UpdateGameLogic 側）
		player_->GetObject3D()->Update();
		//enemyManager_->EnemyObjectUpdate();
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
	//カメラブレンドの更新
	cameraBlendTimer_ += dt;

	float t = cameraBlendTimer_ / cameraBlendDuration_;
	t = std::clamp(t, 0.0f, 1.0f);
	// イージング関数を使ってカメラ位置を補間
	Vector3 pos = Easing::EaseLerp(
		cameraBlendStartPos_,
		cameraBlendEndPos_,
		t,
		Easing::EaseOutSine
	);
	// カメラ位置を更新
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
	pauseMenu_->Update();

	// フェードは常に更新
	fadeManager_.Update();

	// ポーズ中かどうかを判定
	const bool isPaused = pauseMenu_->IsPaused();

	// 固定 dt
	const float dt = 1.0f / 60.0f;

	if (!isPaused &&Input::GetInstance()->TriggerKey(DIK_R)) {
		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "R");
		ResetStage();
	
	}

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
	goal_->Draw();

	if (isStageStartPlaying_) {
		if (!pauseMenu_->IsPaused())
		{
			stageStartEffect_->Draw();
		} //ゲートのみ描画
		player_->Draw();            
	}
	else {

		player_->Draw();

	}
	//ブロックを描画
	generateBlock_.Draw();

	pauseMenu_->Draw();

	//object3D2nd->Draw();
	ParticleManager::GetInstance()->Draw();
	LineCommon::GetInstance()->Draw();
#pragma endregion

#pragma region スプライト描画

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	if (!pauseMenu_->IsPaused()) {
		// ControlGuide をここで描画すると UI レイヤーで最前面に来ます
		UIeditor::GetInstance()->Render();
	}
	/*sprite->Draw();*/
	fadeManager_.Draw();
	goal_->Draw2D();

	if (isStageStartPlaying_) {
		if (!pauseMenu_->IsPaused())
			stageStartEffect_->Draw2D();
	}
}

void GamePlayScene::Imguidebug()
{
	//マップ作製エディタ
	editor_.Run();

	if (editor_.GetReloadRequested()) {
		mapChipField_->ApplyStageData(editor_.GetStageData());

		generateBlock_.GenerateObject3D();

		// プレイヤー位置更新
		auto spawnPositions = mapChipField_->GetPositionBySpawn("player");
		if (!spawnPositions.empty()) {
			Vector3 playerPosition = spawnPositions.front();
			player_->GetObject3D()->SetTranslate(playerPosition);
		}

		// ゴール位置更新
		goal_ = std::make_unique<Goal>();
		goal_->Initialize(mapChipField_.get(), player_.get());

		editor_.SetReloadRequested(false);
	}

	
#ifdef USE_IMGUI
	

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

void GamePlayScene::Load()
{
	//3Dオブジェクト読み込み
	ModelManager::GetInstance()->LoadModel("plane");
	ModelManager::GetInstance()->LoadModel("axis");
	ModelManager::GetInstance()->LoadModel("cube");

	ModelManager::GetInstance()->LoadModel("player");
	ModelManager::GetInstance()->LoadModel("block");
	ModelManager::GetInstance()->LoadModel("skyplane");
	ModelManager::GetInstance()->LoadModel("enemy");
	ModelManager::GetInstance()->LoadModel("goal");
	ModelManager::GetInstance()->LoadModel("sphere");
	ModelManager::GetInstance()->LoadModel("gate");
	ModelManager::GetInstance()->LoadModel("unbreakableBlock");
	ModelManager::GetInstance()->LoadModel("damageblock");
}

void GamePlayScene::ResetStage()
{

	const int stageIndex = SceneManager::GetInstance()->GetStageIndex();
	const int kAvailableMaps = 12;

	std::string stagePath;
	if (stageIndex >= 0 && stageIndex < kAvailableMaps) {
		stagePath = "Resources/Mapdata/Map" + std::to_string(stageIndex + 1) + ".csv";
	} else {
		stagePath = "Resources/Mapdata/Map1.csv";
	}

	// マップをCSVから戻す
	mapChipField_->LoadMapChipCsv(stagePath);

	// ブロックを作り直す
	generateBlock_.Initialize(mapChipField_.get());
	generateBlock_.GenerateObject3D();

	// プレイヤースポーン取得
	Vector3 playerPosition{};
	auto spawnPositions = mapChipField_->GetPositionBySpawn("player");
	if (!spawnPositions.empty()) {
		playerPosition = spawnPositions.front();
	}

	player_ = std::make_unique<Player>();
	player_->SetMapChipField(mapChipField_.get());
	player_->Initialize(playerPosition);

	goal_ = std::make_unique<Goal>();
	goal_->Initialize(mapChipField_.get(), player_.get());
	

	gameOverEffect_ = std::make_unique<GameOverEffect>();
	gameOverEffect_->Initialize(player_->GetObject3D());

	isStageStartPlaying_ = false;
	isCameraBlending_ = false;

	CameraManager::GetInstance()->GetActiveCamera()->SetTranslate({cameraEndPos_});

}

