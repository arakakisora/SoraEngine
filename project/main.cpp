

#include <string>
#include <format>

#include <windows.h>


#pragma comment(lib,"dxguid.lib")

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include"MyMath.h"

#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include"Sprite.h"

#include "Object3DCommon.h"
#include "Object3D.h"
#include "RenderingData.h"
#include "Model.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include"ImGuiManager.h"
#include <imgui.h>
#include "SrvManager.h"

// windowアプリでのエントリ―ポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	//出力ウィンドウへの文字出力
	OutputDebugStringA("HEllo,DIrectX!\n");


#pragma region 基盤システム初期化
	//ポインタ
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;

	//初期化
	//WindousAPI初期化
	winApp = new WinApp;
	winApp->Initialize();
	//DX初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

#ifdef _DEBUG

	//imguiMnagerの初期化
	ImGuiManager* imGuiMnager = nullptr;
	imGuiMnager = new ImGuiManager();
	imGuiMnager->Initialize(dxCommon, winApp);
#endif

	//srvマネージャの宣言
	SrvManager* srvManager = nullptr;
	//srvマネージャの初期化
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	//テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	//入力宣言
	input = new Input();
	//入力初期化
	input->Initialize(winApp);

	//スプライト宣言
	SpriteCommon* spriteCommon = nullptr;
	//スプライト共通部分の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//3Dオブジェクト共通部宣言
	Object3DCommon* object3DCommon = nullptr;
	//3Dオブジェクト共通部の初期化
	object3DCommon = new Object3DCommon;
	object3DCommon->Initialize(dxCommon);

	//3Dモデルマネージャの初期化
	ModelManager::GetInstans()->Initialize(dxCommon,srvManager);


#pragma endregion 


#pragma region 最初のシーン初期化

	//カメラの生成
	Camera* camera = new Camera();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	object3DCommon->SetDefaultCamera(camera);

	//テクスチャの初期化
	std::string textureFilePath[2]{ "Resources/monsterBall.png" ,"Resources/uvChecker.png" };
		 
	//スプライトの初期化
	std::vector<Sprite*>sprites;
	for (uint32_t i = 0; i < 12; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, textureFilePath[1]);
		sprites.push_back(sprite);


    
	}
	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("axis.obj");


	//3Dオブジェクトの初期化
	Object3D* object3D = new Object3D();
	object3D->Initialize(object3DCommon);
	object3D->SetModel("plane.obj");


	//3Dオブジェクトの初期化
	Object3D* object3D2nd = new Object3D();
	object3D2nd->Initialize(object3DCommon);
	object3D2nd->SetModel("plane.obj");

#pragma endregion




	int i = 0;
	for (Sprite* sprite : sprites) {
		Vector2 position = sprite->GetPosition();
		Vector2 size = sprite->GetSize();

		position.x = 100.0f * i;
		position.y = 200.0f;
		size = Vector2(100, 100);

		sprite->SetPosition(position);
		sprite->SetSize(size);
		sprite->SetAnchorPoint(Vector2{ 0.0f,0.0f });
		sprite->SetIsFlipY(0);
		sprite->SetTextureLeftTop(Vector2{ i * 64.0f,0.0f });
		sprite->SetTextureSize(Vector2{ 64.0f,64.0f });

		i++;

	}

	float rotation{ 0 };



  
	//wvpData用のTransform変数を作る





	bool useMonsterBall = true;

	while (true) {//ゲームループ

		camera->Update();

		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

#ifdef _DEBUG

		imGuiMnager->Begin();
#endif

		input->Update();




		for (Sprite* sprite : sprites) {

			//rotation.x+=0.03f;
			//sprite->SetRotation(rotation.x);
			sprite->Update();


		}

		rotation += 0.03f;
		object3D->SetRotate(Vector3{ 0,rotation ,0 });
		object3D->Update();

		object3D2nd->SetRotate(Vector3{ 0,0 ,rotation });
		object3D2nd->Update();





#ifdef _DEBUG
		
		//CameraTransform
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{/*
			ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
			ImGui::DragFloat3("CameraRotate", &cameraTransform.rotate.x, 0.01f);*/
		}
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		// ModelTransform
		if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			transformModel = object3D->GetTransform();

		//	object3D->SetTransform(transformModel);
		//}
		////if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
		////{
		////	//SpriteTransform
		////	Vector2 size = sprite->GetSize();
		////	Vector2 position = sprite->GetPosition();
		////	float rotation = sprite->GetRotation();
		////	Vector4 spritecolor = sprite->GetColor();

			object3D->SetTransform(transformModel);
		}


		//項目4
		if (ImGui::CollapsingHeader("directionalLight", ImGuiTreeNodeFlags_DefaultOpen))
		{
			/*ImGui::ColorEdit4("*LightSetColor", &directionalLightData->color.x);
			ImGui::DragFloat3("*Lightdirection", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);*/
		}
#endif


#ifdef _DEBUG

		imGuiMnager->End();
#endif // DEBUG

		//DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む

    
		dxCommon->Begin();
		srvManager->PreDraw();

#pragma region 3Dオブジェクト描画

		//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3DCommon->CommonDraw();
		object3D->Draw();
		object3D2nd->Draw();

#pragma endregion

#pragma region スプライト描画

		//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->CommonDraw();
		/*sprite->Draw();*/

		for (Sprite* sprite : sprites) {

			sprite->Draw();

		}

#pragma endregion

#ifdef _DEBUG
		//imgui描画処理
		imGuiMnager->Draw();
#endif // DEBUG

		//dx共通部PostDraw
		dxCommon->End();

	}
#pragma region Finalize処理
#ifdef _DEBUG

	//imgui終了処理
	imGuiMnager->Finalize();

#endif // DEBUG

	//WindowsAPI終了処理
	winApp->Finalize();
	//WindowsAPI解放
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstans()->Finalize();

#pragma endregion

#pragma region Release
	delete winApp;
	delete dxCommon;

#ifdef _DEBUG
	delete imGuiMnager;
#endif // DEBUG

	delete input;
	delete srvManager;

	//スプライト解放
	delete spriteCommon;
	for (Sprite* sprite : sprites) {
		delete sprite; // 各Spriteオブジェクトの削除
	}
	//3Dオブジェクト解放
	delete object3DCommon;
	delete object3D;
	delete object3D2nd;

#pragma endregion

	return 0;

}
