#include "GameOverScene.h"
#include "GameClearScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "SceneManager.h"

void GameOverScene::Initialize()
{
}

void GameOverScene::Finalize()
{
}

void GameOverScene::Update()
{

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gameOverScene");
		if (ImGui::Button("TitleScene"))
		{
			SceneManager::GetInstance()->ChangeScene("TITELE");
		}




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

#pragma endregion

}
