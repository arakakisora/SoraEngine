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

void GamePlayScene::Initialize()
{

	//カメラの生成	
	camera = new Camera();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	CameraManager::GetInstance()->AddCamera("maincam", camera);
	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	//モデルの読み込み
	Road();

	// MapChipFiled
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/Mapdate/testmap.csv");
	GenerateObject3D();

	//playerの生成	
	player = new Player();
	Vector3 playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
	player->SetMapChipField(mapChipField_);
	player->Initialize(playerPostion);
	player->SetDeathHeight(0.0f);

	ParticleMnager::GetInstance()->CreateParticleGroup("Player", "Resources/block.png", "sphere.obj");
	playeremitter_ = new ParticleEmitter(
		{ 0.0f,0.3f,0.0f },
		5.0f,
		0.0f,
		1,
		"Player"
	);
	playeroffset = { 0.0f,0.0f,0.0f };

	//エネミー
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(mapChipField_);

	//当たり判定の初期化
	collitionManager_ = std::make_unique<CollisionManager>();
	collitionManager_->Initialize(player,enemyManager_.get());


	//3Dオブジェクトの初期化
	object3D2nd = new Object3D();
	object3D2nd->Initialize(Object3DCommon::GetInstance());
	object3D2nd->SetModel("plane.obj");

	// ゴールの生成
	GoolObject3D = new Object3D();
	GoolObject3D->Initialize(Object3DCommon::GetInstance());
	GoolObject3D->SetModel("gool.obj");
	Vector3 goolPosition = mapChipField_->GetMapChipPostionByIndex(82, 15);
	GoolObject3D->SetTranslate(goolPosition);

	//SkyDome
	skydome_ = new Object3D();
	skydome_->Initialize(Object3DCommon::GetInstance());
	skydome_->SetModel("skyplane.obj");
	skydome_->SetScale(Vector3{ 50.0f,50.0f,1.0f });
	//ライト
	skydome_->SetLighting(false);

	//フォローカメラ設定
	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowTarget(player->GetObject3D(), {0, 0, -15});

	CameraManager::GetInstance()->GetCamera("maincam")->SetFollowMode(true);


	//パーティクルの初期化
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle1", "Resources/gradationLine.png", VerticesType::Cylinder, std::make_unique<MagicCircleBehavior>());
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle2", "Resources/gradationLine.png", VerticesType::Ring, std::make_unique<AttackBehavior>());


	particleEmitter = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 10, "Pariticle2");
	particleEmitter2 = std::make_unique<ParticleEmitter>(Vector3(0, 0, 0), 1.0f, 0.0f, 1, "Pariticle1");


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

	//delete object3D;
	delete object3D2nd;
	delete camera;
	delete mapChipField_;
	delete player;
	delete GoolObject3D;
	delete skydome_;
	delete playeremitter_;
	
}

void GamePlayScene::Update()
{
	skydome_->Update();
	

	//カメラの更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();

	//プレイヤーの更新
	player->Update();
	//エネミーの更新
	enemyManager_->Update();
	collitionManager_->Update();

	// プレイヤーが右に移動中
	if (player->GetPrayerMoveRight()) {
		playeroffset = { -0.3f,0.0f,0.0f };
		playeremitter_->SetPosition(player->GetObject3D()->GetTransform().translate + playeroffset);
		// 左方向に設定
		playeremitter_->SetisRight(false);
		// プレイヤーのパーティクルを発生させる
		playeremitter_->PlayerEmit();
	}

	// プレイヤーが左に移動中
	if (player->GetPrayerMoveLeft()) {
		playeroffset = { 0.3f,0.0f,0.0f };
		playeremitter_->SetPosition(player->GetObject3D()->GetTransform().translate + playeroffset);
		// 右方向に設定
		playeremitter_->SetisRight(true);
		// プレイヤーのパーティクルを発生させる
		playeremitter_->PlayerEmit();
	}

	playeremitter_->SetPosition(player->GetObject3D()->GetTransform().translate + playeroffset);
	// パーティクルの更新
	//playeremitter_->Update();

	//Goolの更新
	GoolObject3D->Update();


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
	//SkyDome
	skydome_->Draw();

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

	//Goolの描画
	GoolObject3D->Draw();

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

//void GamePlayScene::CheckAllCollisions()
//{
//
//
//
//	// ゴールとの当たり判定
//	AABB goolAABB;
//	goolAABB.min = GoolObject3D->GetTransform().translate - Vector3(0.5f, 0.5f, 0.5f);
//	goolAABB.max = GoolObject3D->GetTransform().translate + Vector3(0.5f, 0.5f, 0.5f);
//
//	//if (MyMath::IsCollision(aabb1, goolAABB)) {
//	//	// ゴールシーンに遷移
//	//	SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
//
//	//}
//
//
//}

void GamePlayScene::Imguidebug()
{


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
