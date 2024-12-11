

#include <string>
#include <format>

#include <windows.h>


#pragma comment(lib,"dxguid.lib")

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include"MyMath.h"
//#include "RenderingPipeline.h"




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
#include "Audio.h"
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

	//srvマネージャの宣言	
	SrvManager* srvManager = nullptr;	
	//srvマネージャの初期化	
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

#ifdef _DEBUG

	//imguiMnagerの初期化
	ImGuiManager* imGuiMnager = nullptr;
	imGuiMnager = new ImGuiManager();
	imGuiMnager->Initialize(dxCommon, winApp);

#endif // _DEBUG


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
	ModelManager::GetInstans()->Initialize(dxCommon, srvManager);


#pragma endregion 

#pragma region 最初のシーン初期化


	//カメラの生成	
	Camera* camera = new Camera();
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-10, });
	object3DCommon->SetDefaultCamera(camera);

	//テクスチャの初期化
	std::string textureFilePath[2]{ "Resources/monsterBall.png" ,"Resources/uvChecker.png" };

	
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

	Audio* audio_ = nullptr;
	audio_->GetInstance()->Initialize();


#pragma endregion

	SoundData sounddata1 = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");


	int i = 0;
	



	//wvpData用のTransform変数を作る
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	bool useMonsterBall = true;

	bool bgm = false;
	while (true) {//ゲームループ

		camera->Update();

		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
#ifdef _DEBUG

		imGuiMnager->Begin();
		srvManager->PreDraw();

#endif // _DEBUG

		input->Update();

		if (!bgm) {

			Audio::GetInstance()->SoundPlayWave( sounddata1);
			bgm = true;
		}

		if (input->TriggerKey(DIK_SPACE)) {

			Audio::GetInstance()->SoundUnload(&sounddata1);
			
		}

		


		


		

		
		
		object3D->Update();

		
		object3D2nd->Update();

#ifdef _DEBUG

		if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			transformModel = object3D->GetTransform();

			ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
			ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
			ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);

			object3D->SetTransform(transformModel);
		}
#endif // _DEBUG




#ifdef _DEBUG
		imGuiMnager->End();
#endif // _DEBUG


		//DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
		dxCommon->Begin();

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

		

#pragma endregion

#ifdef _DEBUG
		imGuiMnager->Draw();
#endif // _DEBUG


		dxCommon->End();


	}





#ifdef _DEBUG
	imGuiMnager->Finalize();
#endif // DEBUG

	//aoudio解放
	Audio::GetInstance()->Finalize();
	//WindowsAPI終了処理
	winApp->Finalize();
	//WindowsAPI解放
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstans()->Finalize();
	delete winApp;
	delete dxCommon;
#ifdef _DEBUG
	delete imGuiMnager;
#endif // _DEBUG

	delete input;

	//スプライト解放
	delete spriteCommon;
	
	//3Dオブジェクト解放
	delete object3DCommon;
	delete object3D;
	delete object3D2nd;
	delete camera;
	delete srvManager;

	return 0;

}
