#include "GamePlayScene.h"  
#include <ModelManager.h>  
#include "Object3DCommon.h"  
#include "SpriteCommon.h"  
#include "ImGuiManager.h"  
#include <imgui.h>  
#include "Input.h"  
#include "TitleScene.h"  
#include "CameraManager.h"  
#include <ParticleMnager.h>  
#include "ChargeBehabiaor.h"  

void GamePlayScene::Initialize()
{
	loadJson_ = new LoadJson();
	loadJson_->LoadJsonFile();
	loadJson_->Initialize();

	//カメラの生成  
	camera = new Camera();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	CameraManager::GetInstance()->AddCamera("maincam", camera);
	// デフォルトカメラを設定  
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	debuckCamera = new Camera();
	debuckCamera->SetRotate({ 0,0,0, });
	debuckCamera->SetTranslate({ 0,0,-10, });
	CameraManager::GetInstance()->AddCamera("debugcam", debuckCamera);

	//モデルの読み込み  
	Road();

	// MapChipFiled  
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/Mapdate/testmap.csv");//testmap blocks.csv  
	GenerateObject3D();

	//playerの生成  
	player = new Player();
	Vector3 playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
	player->SetMapChipField(mapChipField_);
	player->Initialize(playerPostion);
	player->SetDeathHeight(0.0f);

	if (!loadJson_->GetLevelData()->players.empty()) {
		for (const auto& playerData : loadJson_->GetLevelData()->players) {
			// プレイヤー生成
			auto tPlayer = std::make_unique<testplayer>();

			// 各プレイヤーのtransform取得
			EulerTransform transform = playerData.transform;

			// 初期化
			tPlayer->Initialize(transform);

			// vector に追加
			tPlayers_.push_back(std::move(tPlayer));
		}
	}

	//エネミー  
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(mapChipField_);

	//当たり判定の初期化  
	collitionManager_ = std::make_unique<CollisionManager>();
	collitionManager_->Initialize(player, enemyManager_.get());

	//3Dオブジェクトの初期化  
	object3D2nd = new Object3D();
	object3D2nd->Initialize(Object3DCommon::GetInstance());
	object3D2nd->SetModel("plane.obj");

	//SkyDome  
	skydome_ = new Object3D();
	skydome_->Initialize(Object3DCommon::GetInstance());
	skydome_->SetModel("skyplane.obj");
	skydome_->SetScale(Vector3{ 50.0f,50.0f,1.0f });
	skydome_->SetTranslate(Vector3{ 0.0f, 0.0f, 3.0f });
	//ライト  
	skydome_->SetLighting(false);

	//フォローカメラ設定  
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowTarget(player->GetObject3D(), { 0, 0, -15 });

	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(false);

	//ゴールの初期化  
	goal = new Goal();
	goal->Initialize(mapChipField_);
}

void GamePlayScene::Finalize()
{
	//object3Dの解放  

	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->RemoveCamera("debugcam");

	for (std::vector<Object3D*>& objext3dLine : blockobject3D)
	{
		for (Object3D* obj : objext3dLine)
		{
			delete obj;
		}
	}
	blockobject3D.clear();

	//delete object3D;  
	delete object3D2nd;
	//delete camera;  
	delete mapChipField_;
	delete player;
	delete skydome_;
	delete goal;

	loadJson_->Finalize();
	delete loadJson_;

}

void GamePlayScene::Update()
{
	//カメラの更新  
	CameraManager::GetInstance()->GetActiveCamera()->Update();

	loadJson_->Update();
	for (auto& tPlayer : tPlayers_) {
		tPlayer->Update();
		
	}

	skydome_->Update();
	goal->Update(player->GetGoal());


	//プレイヤーの更新  
	player->Update();
	//エネミーの更新  
	enemyManager_->Update();
	collitionManager_->Update();
	//プレイヤーが死んだらゲームオーバーシーンに遷移  
	if (player->GetIsDead_()) {

		SceneManager::GetInstance()->ChangeScene("GAMEOVER");

	}

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
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む  
	Object3DCommon::GetInstance()->CommonDraw();

	loadJson_->Draw();
	for (auto& tPlayer : tPlayers_) {
		
		tPlayer->Draw();
	}
	//SkyDome  
	skydome_->Draw();
	goal->Draw();

	//Playerの描画  
	if (player->GetIsDead_() == false) {
		player->Draw();
	}
	//エネミーの描画  
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
#pragma endregion  

#pragma region スプライト描画  

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む  
	SpriteCommon::GetInstance()->CommonDraw();
	/*sprite->Draw();*/
}

void GamePlayScene::GenerateObject3D()
{
	// 要素数  
	uint32_t numBlokVirtical = mapChipField_->GetNumBlockVirtical();     // 縦  
	uint32_t numBlokHorizontal = mapChipField_->GetNumBlockHorizontal(); // 横  

	blockobject3D.resize(numBlokVirtical);

	for (uint32_t i = 0; i < numBlokVirtical; ++i)
	{
		blockobject3D[i].resize(numBlokHorizontal);
	}
	// キューブ生成  
	for (uint32_t i = 0; i < numBlokVirtical; ++i) {
		for (uint32_t j = 0; j < numBlokHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {

				Object3D* object3D_ = new Object3D();
				object3D_->Initialize(Object3DCommon::GetInstance());
				object3D_->SetModel("blokc.obj");
				blockobject3D[i][j] = object3D_;
				blockobject3D[i][j]->SetTranslate(mapChipField_->GetMapChipPostionByIndex(j, i));
				//ライト  
				object3D_->SetLighting(true);
				object3D_->SetDirectionalLightEnable(true);
				object3D_->SetDirectionalLightDirection({ 0.88f, -1.90f, 4.0f });
			}
		}
	}
}

void GamePlayScene::Imguidebug()
{
	//カメラの切り替え
	ImGui::Begin("Camera");

	if (ImGui::Button("Main Camera")) {
		CameraManager::GetInstance()->SetActiveCamera("maincam");
	}
	if (ImGui::Button("Debug Camera")) {
		CameraManager::GetInstance()->SetActiveCamera("debugcam");
	}

	EulerTransform transform = CameraManager::GetInstance()->GetActiveCamera()->GetTransform();
	ImGui::DragFloat3("*cameraRotate", &transform.rotate.x, 0.01f);
	ImGui::DragFloat3("*cameraTransrate", &transform.translate.x, 0.01f);
	CameraManager::GetInstance()->GetActiveCamera()->SetTransform(transform);


	//カメラの位置と向きの表示


	ImGui::End();



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
}

