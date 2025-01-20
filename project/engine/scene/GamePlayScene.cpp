#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"
#include "CameraManager.h"
#include "MyMath.h"

void GamePlayScene::Initialize()
{
	//カメラの生成
	camera1 = new Camera();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("maincam",camera1);

	//カメラの生成
	camera2 = new Camera();
	camera2->SetTranslate({ 0,0,-5, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("subcam", camera2);
	
	// デフォルトカメラを設定
	CameraManager::GetInstans()->SetActiveCamera("maincam");


	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("axis.obj");

	object3D = new Object3D();
	object3D->Initialize(Object3DCommon::GetInstance());
	object3D->SetModel("axis.obj");

	

}

void GamePlayScene::Finalize()
{

	delete camera1;
	delete camera2;
	delete object3D;

	CameraManager::GetInstans()->Finalize();

		

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	object3D->Update();

	
	Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
	Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };

	Quaternion identity = IdentityQuaternion();
	Quaternion conj = Conjugate(q1);
	Quaternion inv = Inverse(q1);
	Quaternion normal = Normalize(q1);
	Quaternion mul1 = Multiply(q1, q2);
	Quaternion mul2 = Multiply(q2, q1);



#ifdef _DEBUG

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gamePlayScene %d");
		/*if (ImGui::Button("GameClearScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
		}
		if (ImGui::Button("GameOverScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMEOVER");
		}*/
		QuaternionImGuiText(q1, "q1");
		QuaternionImGuiText(q2, "q2");
		QuaternionImGuiText(identity, "identity");
		QuaternionImGuiText(conj, "conj");
		QuaternionImGuiText(inv, "inv");
		QuaternionImGuiText(normal, "normal");
		QuaternionImGuiText(mul1, "mul1");
		QuaternionImGuiText(mul2, "mul2");


	}


	
	/*if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Switch to Main Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("maincam");
		}
		if (ImGui::Button("Switch to Sub Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("subcam");
		}
	}*/
#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	object3D->Draw();


#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

#pragma endregion




}
