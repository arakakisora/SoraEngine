#include "GameOverScene.h"
#include <memory>
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include "CameraManager.h"
#include "ModelManager.h"
#include <imgui.h>
#include <ChargeBehabiaor.h>

void GameOverScene::Initialize()
{
	// sprite
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameover.png");

	// camera
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	ModelManager::GetInstance()->LoadModel("player");
	playerObj = std::make_unique<Object3D>();
	playerObj->Initialize(Object3DCommon::GetInstance());
	playerObj->SetModel("player");
	playerObj->SetLighting(true);
	playerObj->SetEnableLighting(true);
	playerObj->SetDirectionalLightEnable(true);

	// 画面中央に配置
	playerObj->SetTranslate({ 0.0f, -0.7f, 11.0f });
	playerObj->SetRotate({ 2.6f, 0.0f, 0.0f });
	playerObj->SetScale({ 2.0f, 2.0f, 2.0f });


	/*ParticleManager::GetInstance()->CreateParticleGroup(
		"enemydeath",
		"Resources/honoo.png",
		VerticesType::Quad,
		std::make_unique<ExplosionBehavior>()
	);*/
}

void GameOverScene::Finalize()
{
	// 先にカメラをマネージャから除去してから破棄する
	CameraManager::GetInstance()->RemoveCamera("maincam");

	
	
}

void GameOverScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();

	sprite->Update();

	// 回転処理だけ
	EulerTransform tr = playerObj->GetTransform();

	tr.rotate.y += 0.03f; 

	playerObj->SetRotate(tr.rotate);

	playerObj->Update();

	// スペースで戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		fadeManager_.StartFadeOut();
	}

	if (fadeManager_.IsFadeOutFinished()) {

		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}
		//SceneManager::GetInstance()->ChangeScene("TITELE");

	const float dt = 1.0f / 60.0f;
	exhaustTimer_ += dt;

	if (exhaustTimer_ >= 1.0f) {
		exhaustTimer_ = 0.0f;

		EulerTransform smokeTransform{};
		smokeTransform.translate = playerObj->GetTransform().translate;
		smokeTransform.translate.y += 0.5f;
		smokeTransform.translate.x -= 0.2f;

		for (int i = 0; i < 3; i++) {
			ParticleManager::GetInstance()->Emit("enemydeath", smokeTransform);
		}

	}
#ifdef USE_IMGUI
	ImGui::Begin("GameOver Debug");

	// 回転速度
	ImGui::DragFloat("Rotate Speed Y", &rotateSpeedY_, 0.001f, -1.0f, 1.0f);

	// Transform取得
	Vector3 pos = tr.translate;
	Vector3 rot = tr.rotate;
	Vector3 scale = tr.scale;

	// 位置
	if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
		playerObj->SetTranslate(pos);
	}

	// 回転（手動調整もできる）
	if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) {
		playerObj->SetRotate(rot);
	}

	// スケール
	if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.0f, 5.0f)) {
		playerObj->SetScale(scale);
	}

	// リセットボタン
	if (ImGui::Button("Reset Rotation")) {
		playerObj->SetRotate({ 0,0,0 });
	}

	ImGui::End();
#endif


}

void GameOverScene::Draw()
{

	SpriteCommon::GetInstance()->CommonDraw();
	//Spriteの描画
	//sprite->Draw();
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	playerObj->Draw();

	ParticleManager::GetInstance()->Draw();
#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//Spriteの描画
	sprite->Draw();
	fadeManager_.Draw();

	

}
