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


void GamePlayScene::Initialize()
{
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	//カメラの生成	
	camera = new Camera();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	CameraManager::GetInstance()->AddCamera("maincam", camera);

	debugCamera = new Camera();
	debugCamera->SetRotate({ 0,0,0, });
	debugCamera->SetTranslate({ 15,13,-60, });
	CameraManager::GetInstance()->AddCamera("debugcam", debugCamera);

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	//モデルの読み込み
	Road();

	MapChipDatabase::GetInstance()->LoadJson("Resources/Data/MapChipTypes.json");
	// MapChipFiled
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/Mapdata/testmap1.csv");//testmap blocks.csv
	GenerateObject3D();



	//playerの生成	
	player = new Player();
	Vector3 playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
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
	object3D2nd = new Object3D();
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
	goal = new Goal();
	goal->Initialize(mapChipField_,player);




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


	delete camera;
	delete debugCamera;
	delete mapChipField_;
	delete player;
	delete skydome_;
	delete goal;
	delete object3D2nd;


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

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == 1) {


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

