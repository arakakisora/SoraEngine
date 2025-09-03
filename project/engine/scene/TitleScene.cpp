#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include <CameraManager.h>


void TitleScene::Initialize()
{
	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->Finalize();
	//カメラの生成
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstance()->AddCamera("maincam", camera1.get());

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");
	
}

void TitleScene::Finalize()
{
	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->Finalize();
}

void TitleScene::Update()
{
	//カメラの更新
	
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		
	}

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("titleScene %d");
		if (ImGui::Button("gamePlayScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		}
		


	}

}

void TitleScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

}
