#include "GameOverScene.h"
#include <memory>
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include "CameraManager.h"

void GameOverScene::Initialize()
{

	//titeleの生成
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameover.dds");
	//titleSprite->SetSize({ 1280,720 });

	// カメラ
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	fadeManager_.Initialize("Resources/white.dds");
	fadeManager_.StartFadeIn();

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

	//スプライトの更新
	sprite->Update();
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {



		SceneManager::GetInstance()->ChangeScene("TITELE");
	}



}

void GameOverScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();


#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//Spriteの描画
	sprite->Draw();
	fadeManager_.Draw();

}
