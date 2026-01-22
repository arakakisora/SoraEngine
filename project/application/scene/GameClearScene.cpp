#include "GameclearScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include <CameraManager.h>

void GameClearScene::Initialize()
{
	//titeleの生成
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameclear.png");
	//titleSprite->SetSize({ 1280,720 });


	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera);
	CameraManager::GetInstance()->SetActiveCamera("maincam");
}

void GameClearScene::Finalize()
{
	delete sprite;
	sprite = nullptr;
	delete camera;
	CameraManager::GetInstance()->RemoveCamera("maincam");
}

void GameClearScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();

	//スプライトの更新
	sprite->Update();

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {



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

}
