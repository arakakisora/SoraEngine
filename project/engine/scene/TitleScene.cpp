#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"


void TitleScene::Initialize()
{
	
	
	
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
	if (Input::GetInstans()->TriggerKey(DIK_SPACE)) {

		
		
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}



}

void TitleScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

}
