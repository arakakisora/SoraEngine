#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"
#include "CameraManager.h"

void GamePlayScene::Initialize()
{
	//カメラの生成
	camera1 = new Camera();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("maincam",camera1);

	// デフォルトカメラを設定
	CameraManager::GetInstans()->SetActiveCamera("maincam");


	

	

	
	

}

void GamePlayScene::Finalize()
{

	delete camera1;
	

	CameraManager::GetInstans()->Finalize();

		

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	

	




#ifdef _DEBUG

	if (ImGui::CollapsingHeader("ans", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gamePlayScene %d");
		
		


	}

	
	
#endif // _DEBUG
}

void GamePlayScene::Draw()
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
