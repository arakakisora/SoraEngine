#include "GameclearScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include <CameraManager.h>
#include <memory>


void GameClearScene::Initialize()
{
	//titleの生成
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameclear.png");
	//titleSprite->SetSize({ 1280,720 });


	// カメラ
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	// フェードインの初期化
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	gate_ = std::make_unique<GateInOut>();
	gate_->Initialize(SpriteCommon::GetInstance(),
		"Resources/block.png", "Resources/block.png");
	gate_->SetScreenSize(1280.0f, 720.0f); // WinAppから取れるならそれで
	gate_->StartIn(0.6f);
	gateOutRequested_ = false;
}

void GameClearScene::Finalize()
{
	// CameraManager から先にカメラを削除してから unique_ptr をリセットする
	CameraManager::GetInstance()->RemoveCamera("maincam");

	// unique_ptr によって自動的に解放される
	sprite.reset();
	camera.reset();
}

void GameClearScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	// フェード更新
	fadeManager_.Update();
	float dt = 1.0f / 60.0f;
	if (gate_) gate_->Update(dt);
	//スプライトの更新
	sprite->Update();

	if (!gateOutRequested_ &&
		(!gate_ || !gate_->IsPlaying()) &&
		Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		gateOutRequested_ = true;
		if (gate_) gate_->StartOut(0.6f);
	}

	if (gateOutRequested_ && !fadeOutRequested_ && gate_ && gate_->IsFinished()) {
		fadeOutRequested_ = true;

		gate_->HoldClosed(true);

		fadeManager_.StartFadeOut();
	}

	if (fadeOutRequested_ && fadeManager_.IsFadeOutFinished()) {
		SceneManager::GetInstance()->ChangeScene("TITELE");
	}

}

void GameClearScene::Draw()
{

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//Spriteの描画
	sprite->Draw();
	if (gate_) gate_->Draw2D();
	fadeManager_.Draw();
}
