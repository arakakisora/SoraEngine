#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"
#include "CameraManager.h"
#include "ParticleMnager.h"


void GamePlayScene::Initialize()
{
	////カメラの生成
	//camera1 = new Camera();

	//camera1->SetTranslate({ 0,0,-20, });//カメラの位置

	//CameraManager::GetInstans()->AddCamera("maincam",camera1);

	////カメラの生成
	//camera2 = new Camera();
	//camera2->SetTranslate({ 0,0,-10, });//カメラの位置
	//CameraManager::GetInstans()->AddCamera("subcam", camera2);
	//
	//// デフォルトカメラを設定
	//CameraManager::GetInstans()->SetActiveCamera("maincam");


	////モデルの読み込み

	//ModelManager::GetInstans()->LoadModel("axis.obj");
	//ModelManager::GetInstans()->LoadModel("plane.obj");
	//ModelManager::GetInstans()->LoadModel("sphere.obj");
	////ModelManager::GetInstans()->LoadModel("bunny.obj");

	//
	//

	//object3D = new Object3D();
	//object3D->Initialize(Object3DCommon::GetInstance());
	//object3D->SetModel("sphere.obj");
	//object3D->SetLighting(true);

	//
	//light = true;

	//////パーティクルの初期化
	////ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle1", "Resources/uvChecker.png", "sphere.obj");
	////particleEmitter = new ParticleEmitter(Vector3(0, 0, 0), 1.0f, 0.0f, 100, "Pariticle1");

}

void GamePlayScene::Finalize()
{

	//delete camera1;
	//delete camera2;
	//delete object3D;
	//delete particleEmitter;
	

	

		

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	/*object3D->Update();*/
	

	////パーティクルの更新
	//particleEmitter->Update();
	////particleEmitter2->Update();



#ifdef _DEBUG

	//if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	ImGui::Text("gamePlayScene %d");
	//	if (ImGui::Button("GameClearScene"))
	//	{
	//		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	//	}
	//	if (ImGui::Button("GameOverScene"))
	//	{
	//		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	//	}

	//	//ライト
	//	if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
	//		Vector4 color = object3D->GetDirectionalLight().color;
	//		Vector3 direction = object3D->GetDirectionalLight().direction;
	//		float intensity = object3D->GetDirectionalLight().intensity;
	//		if (ImGui::ColorEdit4("Color", &color.x)) {
	//			object3D->SetDirectionalLightColor(color);
	//		}
	//		if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) {
	//			object3D->SetDirectionalLightDirection(direction);
	//		}
	//		if (ImGui::DragFloat("Intensity", &intensity, 0.01f)) {
	//			object3D->SetDirectionalLightIntensity(intensity);
	//		}
	//		//ライトのオンオフ
	//	
	//		if (ImGui::Checkbox("Enable Lighting", &light)) {
	//			object3D->SetLighting(light);
	//		}




	//	}


	//}
	////particleのエミッタ-
	//if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	ImGui::Text("ParticleEmitter");
	//	
	//	if (ImGui::Button("Emit"))
	//	{
	//		particleEmitter->Emit();
	//	}
	//}

	//
	//if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
	//	if (ImGui::Button("Switch to Main Camera")) {
	//		CameraManager::GetInstans()->SetActiveCamera("maincam");
	//	}
	//	if (ImGui::Button("Switch to Sub Camera")) {
	//		CameraManager::GetInstans()->SetActiveCamera("subcam");
	//	}
	//}
#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	///*object3D->Draw();*/
	//ParticleMnager::GetInstance()->Draw();

#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

#pragma endregion




}
