#include "GameclearScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include <CameraManager.h>
#include <memory>
#include <ModelManager.h>
#include <cmath>
#include <imgui.h>

void GameClearScene::Initialize()
{
	// sprite
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameclear.png");

	// camera
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	ModelManager::GetInstance()->LoadModel("player.obj");
	playerObj = std::make_unique<Object3D>();
	playerObj->Initialize(Object3DCommon::GetInstance());
	playerObj->SetModel("player.obj");
	playerObj->SetLighting(true);
	playerObj->SetEnableLighting(true);
	playerObj->SetDirectionalLightEnable(true);

	// 画面中央に配置
	basePos_ = { 0.0f, -0.7f, 11.0f };
	playerObj->SetTranslate(basePos_);
	playerObj->SetRotate({ 0.0f, baseRotateY_, 0.0f });
	playerObj->SetScale({ 1.0f, 1.0f, 1.0f });
	playerObj->SetDirectionalLightDirection({ -1.0f,1.0f, -1.0f });

	clearAnimTimer_ = 0.0f;
	isLanding_ = false;
}

void GameClearScene::Finalize()
{
	// CameraManager から先にカメラを削除してから unique_ptr をリセットする
	CameraManager::GetInstance()->RemoveCamera("maincam");

}
void GameClearScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();

	sprite->Update();

	const float dt = 1.0f / 60.0f;
	clearAnimTimer_ += dt;

	EulerTransform tr = playerObj->GetTransform();

	// ===== クリア演出 =====
	// 最初の大ジャンプ 0.0 ~ 1.0秒
	// 着地後の小ジャンプ 1.0 ~ 1.5秒
	float yOffset = 0.0f;
	const float kPi = 3.14159265f;
	const float kTwoPi = kPi * 2.0f;

	if (clearAnimTimer_ < 1.0f) {
		float t = clearAnimTimer_ / 1.0f;             // 0~1
		yOffset = std::sin(t * kPi) * 2.0f;

		float spin = t * kTwoPi;                     // 0~360度
		tr.rotate.y = baseRotateY_ + spin;           // 基準角 + 回転

		tr.rotate.x = -std::sin(t * kPi) * 0.2f;
		tr.rotate.z = 0.0f;
	}
	else if (clearAnimTimer_ < 1.5f) {
		float t = (clearAnimTimer_ - 1.0f) / 0.5f;
		yOffset = std::sin(t * kPi) * 0.6f;

		tr.rotate.y = baseRotateY_ + kTwoPi;         // 1周した位置で止める
		tr.rotate.x = 0.0f;
		tr.rotate.z = 0.0f;


	}
	else {
		yOffset = 0.0f;
		tr.rotate.y = baseRotateY_ + kTwoPi;
		tr.rotate.x = 0.0f;
		tr.rotate.z = 0.0f;

		EulerTransform smokeTransform{};
		smokeTransform.translate = playerObj->GetTransform().translate;
		smokeTransform.translate.y += -0.5f;
		smokeTransform.translate.x -= 0.2f;


		ParticleManager::GetInstance()->Emit("clear", smokeTransform);
	}

	tr.translate = basePos_;
	tr.translate.y += yOffset;

	playerObj->SetTranslate(tr.translate);
	playerObj->SetRotate(tr.rotate);
	playerObj->Update();

	// スペースで戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		fadeManager_.StartFadeOut();
	}

	if (fadeManager_.IsFadeOutFinished()) {
		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}

	// 煙
	exhaustTimer_ += dt;

	if (exhaustTimer_ >= 1.0f) {
		exhaustTimer_ = 0.0f;


	}

	if (clearAnimTimer_ >= 1.5f) {
		clearAnimTimer_ = 0.0f;
		isLanding_ = false;
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

	// 現在のライト情報を取得してUI初期値に使う
	auto dl = playerObj->GetDirectionalLight();
	Vector3 dir = dl.direction;
	Vector4 col = dl.color;
	float intensity = dl.intensity;
	bool enabled = (dl.enable != 0);

	// ImGui 用の一時配列 / 変数
	float dirArr[3] = { dir.x, dir.y, dir.z };
	float colorArr[4] = { col.x, col.y, col.z, col.w };
	float intensityF = intensity;
	bool enabledB = enabled;

	if (ImGui::Checkbox("Directional Light Enabled", &enabledB)) {
		playerObj->SetDirectionalLightEnable(enabledB ? true : false);
	}
	if (ImGui::DragFloat3("Direction", dirArr, 0.01f, -1.0f, 1.0f)) {
		playerObj->SetDirectionalLightDirection({ dirArr[0], dirArr[1], dirArr[2] });
	}
	if (ImGui::ColorEdit4("Color", colorArr)) {
		playerObj->SetDirectionalLightColor({ colorArr[0], colorArr[1], colorArr[2], colorArr[3] });
	}
	if (ImGui::DragFloat("Intensity", &intensityF, 0.01f, 0.0f, 10.0f)) {
		playerObj->SetDirectionalLightIntensity(intensityF);
	}

	ImGui::End();
#endif
}

void GameClearScene::Draw()
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
