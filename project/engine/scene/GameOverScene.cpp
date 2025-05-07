#include "GameOverScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>

#include "Input.h"
#include "SceneManager.h"
#include "CameraManager.h"

void GameOverScene::Initialize()
{

	//titeleの生成
	sprite = new Sprite();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/gameover.png");
	//titleSprite->SetSize({ 1280,720 });

}

void GameOverScene::Finalize()
{
	delete sprite;
	sprite = nullptr;

}

void GameOverScene::Update()
{
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gameOverScene");
		if (ImGui::Button("TitleScene"))
		{
			SceneManager::GetInstance()->ChangeScene("TITELE");
		}

	//スプライトの更新
	sprite->Update();
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {



		SceneManager::GetInstance()->ChangeScene("TITELE");
	}

#endif // _DEBUG


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

}
