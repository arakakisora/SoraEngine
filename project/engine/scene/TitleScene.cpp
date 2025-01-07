#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"


void TitleScene::Initialize()
{
	//titeleの生成
	
	titleSprite = new Sprite();
	titleSprite->Initialize(SpriteCommon::GetInstance(), "uvChecker.png");
	
	
}

void TitleScene::Finalize()
{
	delete titleSprite;
	titleSprite = nullptr;
}

void TitleScene::Update()
{
	////スプライトの更新
	//titleSprite->Update();

	//スペースキーが押されたらゲームプレイシーンに遷移
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		
		
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}



}

void TitleScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

	////スプライトの描画
	//titleSprite->Draw();

}
