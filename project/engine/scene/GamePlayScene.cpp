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
	camera1->SetTranslate({ 0,0,-30, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("maincam",camera1);

	//カメラの生成
	camera2 = new Camera();
	camera2->SetTranslate({ 0,0,-5, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("subcam", camera2);
	
	// デフォルトカメラを設定
	CameraManager::GetInstans()->SetActiveCamera("maincam");


	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("axis.obj");
	ModelManager::GetInstans()->LoadModel("plane.obj");

	object3D = new Object3D();
	object3D->Initialize(Object3DCommon::GetInstance());
	object3D->SetModel("axis.obj");

	
	ParticleMnager::GetInstance()->CreateParticleGroup("particle1", "Resources/circle.png", "plane.obj");

	//ParticleMnager::GetInstance()->CreateParticleGroup("particle2", "Resources/uvChecker.png", "axis.obj");
	//エミッタ―の初期化
	particleEmitter = new ParticleEmitter({ 0,0,0 }, { 0,0,0 }, 0.5f, 0.0f, 100, "particle1");

	//particleEmitter2 = new ParticleEmitter({ 10,0,0 }, { 0,0,0 }, 1.0f, 0.0f, 100, "particle2");
	

}

void GamePlayScene::Finalize()
{

	delete camera1;
	delete camera2;
	delete object3D;

	

		

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	object3D->Update();
	

	//パーティクルの更新
	particleEmitter->Update();
	//particleEmitter2->Update();



#ifdef _DEBUG

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gamePlayScene %d");
		if (ImGui::Button("GameClearScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
		}
		if (ImGui::Button("GameOverScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMEOVER");
		}


	}
	//particleのエミッタ-
	if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("ParticleEmitter");
		
		if (ImGui::Button("Emit"))
		{
			particleEmitter->Emit();
		}
	}

	
	if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Switch to Main Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("maincam");
		}
		if (ImGui::Button("Switch to Sub Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("subcam");
		}
	}
#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	/*object3D->Draw();*/
	ParticleMnager::GetInstance()->Draw();

#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

#pragma endregion




}
