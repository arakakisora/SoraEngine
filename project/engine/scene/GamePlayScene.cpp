#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include "imgui.h"
#include "Input.h"
#include "TitleScene.h"
#include "CameraManager.h"
#include "ParticleMnager.h"
#include <Logger.h>

void GamePlayScene::Initialize()
{
	//カメラの生成
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("maincam", camera1.get());

	//カメラの生成
	camera2 = std::make_unique<Camera>();
	camera2->SetTranslate({ 0,6,-20, });//カメラの位置
	camera2->SetRotate({ 0.3f,0,0 });//カメラの向き
	CameraManager::GetInstans()->AddCamera("subcam", camera2.get());

	// デフォルトカメラを設定
	CameraManager::GetInstans()->SetActiveCamera("maincam");




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





	// MapChipFiled
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateObject3D();

	//playerの生成	
	player = new Player();
	object3DPlayer = new Object3D();
	Vector3 playerPostion = mapChipField_->GetMapChipPostionByIndex(6, 18);
	object3DPlayer->Initialize(Object3DCommon::GetInstance());
	object3DPlayer->SetModel("player.obj");
	object3DPlayer->SetScale(Vector3{ 0.25f,0.25f,0.25f });
	player->SetMapChipField(mapChipField_);
	player->Initialize(object3DPlayer, playerPostion);
	player->SetDeathHeight(0.0f);
	

	//カメラのターゲットをプレイヤーに設定
	camera->SetFollowTarget(object3DPlayer, { 0.0f, 0.0f, -15.0f });
	camera->SetFollowMode(true);


	/*camera->SetFollowTarget(player, { 0.0f, 0.0f, -15.0f });
	camera->SetFollowMode(false);*/

	//3Dオブジェクトの初期化
	object3D2nd = new Object3D();
	object3D2nd->Initialize(Object3DCommon::GetInstance());
	object3D2nd->SetModel("plane.obj");

	// Enemy
	// CSVから敵の位置を取得
	std::vector<Vector3> enemyPositions = mapChipField_->GetEnemyPositions();
	for (const Vector3& enemyPos : enemyPositions) {
		Object3D* object3DEnemy = new Object3D();
		object3DEnemy->Initialize(Object3DCommon::GetInstance());
		object3DEnemy->SetModel("enemy.obj");

		Enemy* newEnemy = new Enemy();
		newEnemy->Initialize(object3DEnemy, enemyPos);
		enemies_.push_back(newEnemy);
	}


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

	//パーティクルの初期化
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle1", "Resources/uvChecker.png", "sphere.obj");
	particleEmitter = std::make_unique<ParticleEmitter>(Vector3(10, 0, 0), 1.0f, 0.0f, 100, "Pariticle1");
}

void GamePlayScene::Finalize()
{
	CameraManager::GetInstans()->RemoveCamera("maincam");
	CameraManager::GetInstans()->RemoveCamera("subcam");
	CameraManager::GetInstans()->Finalize();

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
	delete object3DPlayer;
	//delete object3DEnemy;
	delete GoolObject3D;
	delete skydome_;

	for (auto& enemy : enemies_) {
		if (enemy) {
			enemy->ReleaseObject3D();
			delete enemy; // Enemy自体を解放
		}
	}



}

void GamePlayScene::Update()
{
	skydome_->Update();

	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	object3D->Update();
	terrain->Update();


	//プレイヤーの更新
	player->Update();

	//Goolの更新
	GoolObject3D->Update();


	//プレイヤーが死んだらゲームオーバーシーンに遷移
	if (player->GetIsDead_()) {

		SceneManager::GetInstance()->ChangeScene("GAMEOVER");

	}

	//敵の更新
	for (Enemy* enemy : enemies_) {

		if (!nullptr) {
			enemy->Update(mapChipField_);
		}
	}

	//死んだ敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});




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
	CheckAllCollisions();


#ifdef _DEBUG

	if (ImGui::CollapsingHeader("object3D", ImGuiTreeNodeFlags_DefaultOpen))
	{
		/*transformModel = object3D->GetTransform();

		ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
		ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
		ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);

		object3D->SetTransform(transformModel);*/
	}

	//}
	////particleのエミッタ-
	//if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	ImGui::Text("ParticleEmitter");
	//	
	//	if (ImGui::Button("Emit"))
	//	{
	//		particleEmitter->Emit();
	//	}
	//}

	//
	//if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
	//	if (ImGui::Button("Switch to Main Camera")) {
	//		CameraManager::GetInstans()->SetActiveCamera("maincam");
	//	}
	//	if (ImGui::Button("Switch to Sub Camera")) {
	//		CameraManager::GetInstans()->SetActiveCamera("subcam");
	//	}
	//}
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

	//Enemyの描画

	for (Enemy* enemy : enemies_) {
		if (!nullptr) {
			enemy->Draw();
		}
	}

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


			}
		}
	}


}

void GamePlayScene::CheckAllCollisions()
{

	AABB aabb1, aabb2;
	aabb1 = player->GetAABB();
	for (Enemy* enemy : enemies_) {

		aabb2 = enemy->GetAABB();

		if (MyMath::IsCollision(aabb1, aabb2)) {

			player->OnCollision(enemy);
			enemy->OnCollision(player);
		}
	}

	// 弾と敵の衝突
	for (PlayerBullet* bullet : player->GetBullets()) { // GetBullets を追加で実装
		AABB bulletAABB = bullet->GetAABB();
		for (Enemy* enemy : enemies_) {
			aabb2 = enemy->GetAABB();
			if (MyMath::IsCollision(bulletAABB, aabb2)) {
				bullet->OnCollison();  // 弾を削除
				enemy->OnCollision(bullet); // 敵の処理
				break; // 弾が消滅するので、これ以上判定を行わない
			}
		}
	}

	// ゴールとの当たり判定
	AABB goolAABB;
	goolAABB.min = GoolObject3D->GetTransform().translate - Vector3(0.5f, 0.5f, 0.5f);
	goolAABB.max = GoolObject3D->GetTransform().translate + Vector3(0.5f, 0.5f, 0.5f);

	if (MyMath::IsCollision(aabb1, goolAABB)) {
		// ゴールシーンに遷移
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
		
	}

#pragma endregion
}

void GamePlayScene::LoadModel()
{

	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("axis.obj");
	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("sphere.obj");
	ModelManager::GetInstans()->LoadModel("terrain.obj");
	

}

void GamePlayScene::Loadparticle()
{

	//パーティクルの初期化
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle1", "Resources/uvChecker.png", "sphere.obj");
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle2", "Resources/uvChecker.png", "plane.obj");

}

void GamePlayScene::LoadAudio()
{
	//サウンドの読み込み
	sampleSoundData = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");//今のところwavのみ対応
	

}

