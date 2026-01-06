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


void GamePlayScene::Initialize()
{
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	//カメラの生成	
	camera= std::make_unique<Camera>();
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
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/Mapdata/testmap1.csv");//testmap blocks.csv
	GenerateObject3D();

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
	player->SetMapChipField(mapChipField_);
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
	enemyManager_->Initialize(mapChipField_);






	//3Dオブジェクトの初期化
	object3D2nd = std::make_unique<Object3D>();
	object3D2nd->Initialize(Object3DCommon::GetInstance());
	object3D2nd->SetModel("plane.obj");

	//SkyDome
	skydome_ = new Object3D();
	skydome_->Initialize(Object3DCommon::GetInstance());
	skydome_->SetModel("skyplane.obj");
	skydome_->SetScale(Vector3{ 50.0f,50.0f,1.0f });
	//ライト
	skydome_->SetLighting(false);

	//フォローカメラ設定
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowTarget(player->GetObject3D(), { 0, 0, -15 });

	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);

	//ゴールの初期化
	goal = std::make_unique<Goal>();
	goal->Initialize(mapChipField_,player.get());




}

void GamePlayScene::Finalize()
{

	//object3Dの解放

	for (std::vector<Object3D*>& objext3dLine : blockobject3D)
	{
		for (Object3D* obj : objext3dLine)
		{
			delete obj;
		}
	}
	blockobject3D.clear();

	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->RemoveCamera("debugcam");
	
	delete mapChipField_;
	
	delete skydome_;
	


}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();


	skydome_->Update();
	goal->Update(player->GetGoal(), 1.0f / 60.0f);
	const float dt = 1.0f / 60.0f;
	if (isStageStartPlaying_|| goal->GetIsEffectStarted()) {

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
		SyncBlockObjectsWithMap();
	}
	//プレイヤーが死んだらゲームオーバーシーンに遷移
	if (player->GetIsDead_()) {

		gameOverEffect_->Update(dt);
		CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);


	}
	if (!gameOverEffect_->IsPlaying()) {
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}


	//エネミーの更新




	//3Dオブジェクトの更新
	for (std::vector<Object3D*>& objext3dLine : blockobject3D)
	{
		for (Object3D* obj : objext3dLine)
		{
			if (!obj)
				continue;
			obj->Update();
		}
	}



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
	//skydome_->Draw();
	goal->Draw();


	if (isStageStartPlaying_) {
		stageStartEffect_->Draw(); // ←ゲートのみ描画
		player->Draw();            // ←プレイヤーを別に描画
	} else {

		player->Draw();

	}
	//エネミーの描画o 
	enemyManager_->Draw();



	for (std::vector<Object3D*>& objext3dLine : blockobject3D)
	{

		for (Object3D* obj : objext3dLine)
		{
			if (!obj) {
				continue;
			}
			obj->Draw();
		}

	}



	//object3D2nd->Draw();
	ParticleMnager::GetInstance()->Draw();
	LineCommon::GetInstance()->Draw();
#pragma endregion

#pragma region スプライト描画

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	/*sprite->Draw();*/
	fadeManager_.Draw();
	goal->Draw2D();
}

void GamePlayScene::GenerateObject3D()
{
	if (!mapChipField_) return;

	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 既存のオブジェクトを全部削除して配列を初期化（安全策）
	for (auto& row : blockobject3D) {
		for (Object3D* obj : row) {
			delete obj;
		}
	}
	blockobject3D.clear();

	blockobject3D.resize(numBlokVirtical);
	for (uint32_t i = 0; i < numBlokVirtical; ++i) {
		blockobject3D[i].assign(numBlokHorizontal, nullptr);
	}

	// 実際の作成は Sync に任せる（初回はここで生成される）
	SyncBlockObjectsWithMap();
}

void GamePlayScene::SyncBlockObjectsWithMap()
{
	if (!mapChipField_) return;

	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 配列サイズが一致している前提
	for (uint32_t y = 0; y < numBlokVirtical; ++y) {
		for (uint32_t x = 0; x < numBlokHorizontal; ++x) {

			int type = mapChipField_->GetMapChipTypeByIndex(x, y);
			Object3D* obj = nullptr;
			if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
				obj = blockobject3D[y][x];
			}

			// タイプ 1 はブロック（必要に応じ他のIDも対応）
			if (type == 1) {
				// ブロックが存在すべきだがオブジェクトが無ければ作る
				if (!obj) {
					Object3D* newObj = new Object3D();
					newObj->Initialize(Object3DCommon::GetInstance());
					newObj->SetModel("blokc.obj");
					newObj->SetTranslate(mapChipField_->GetMapChipPostionByIndex(x, y));
					newObj->SetLighting(true);
					newObj->SetDirectionalLightEnable(true);
					newObj->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });

					// 保持配列へ格納
					if (y < blockobject3D.size() && x < blockobject3D[y].size()) {
						blockobject3D[y][x] = newObj;
					}
				}
			} else {
				// ブロックが消えているのにオブジェクトが残っていれば削除
				if (obj) {
					delete obj;
					blockobject3D[y][x] = nullptr;
				}
			}
		}
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
		// 既存のオブジェクト削除
		for (auto& row : blockobject3D) {
			for (auto& obj : row) {
				delete obj;
			}
		}
		blockobject3D.clear();

		// 再生成
		GenerateObject3D();


		enemyManager_.reset();
		enemyManager_ = std::make_unique<EnemyManager>();
		enemyManager_->Initialize(mapChipField_);


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

