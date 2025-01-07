#include "GameclearScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>

void GameclearScene::Initialize()
{
}

void GameclearScene::Finalize()
{
}

void GameclearScene::Update()
{


	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {



		SceneManager::GetInstance()->ChangeScene("TITELE");
	}

}

void GameclearScene::Draw()
{

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

}
