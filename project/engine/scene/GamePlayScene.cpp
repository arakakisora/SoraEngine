#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"

void GamePlayScene::Initialize()
{
	

	//カメラの生成	
	camera = new Camera();



	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-30, });

	Object3DCommon::GetInstance()->SetDefaultCamera(camera);



	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("axis.obj");


	









	



}

void GamePlayScene::Finalize()
{

	

	
	delete camera;


}

void GamePlayScene::Update()
{




	camera->Update();

	




	

	


	

#ifdef _DEBUG

	/*if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		transformModel = object3D->GetTransform();

		ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
		ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
		ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);

		object3D->SetTransform(transformModel);
	}*/
#endif // _DEBUG








}

void GamePlayScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	

#pragma endregion

#pragma region スプライト描画

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	/*sprite->Draw();*/

	

}
