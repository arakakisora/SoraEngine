

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



























#pragma region Resource
	const uint32_t kSubdbivision = 512;


	//VertexResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * kSubdbivision * kSubdbivision * 6);
	//DepthStencilTextureをウィンドウサイズで作成







	////vertexResource頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdbivision * kSubdbivision * 6;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//経度分割1つ分の経度φd
	const float kLonEvery = 2 * std::numbers::pi_v<float> / (float)kSubdbivision;
	//緯度分割１つ分の緯度Θd
	const float kLatEvery = std::numbers::pi_v<float> / (float)kSubdbivision;
	//緯度方向に分割しながら線を描く
	const float w = 2.0f;
	for (uint32_t latIndex = 0; latIndex < kSubdbivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;//θ
		for (uint32_t lonIndex = 0; lonIndex < kSubdbivision; ++lonIndex) {
			//テクスチャ用のTexcood

			//書き込む最初の場所
			uint32_t start = (latIndex * kSubdbivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//∮
			//基準点a
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };
			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex + 1.0f) / float(kSubdbivision) };
			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };

			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };

			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex + 1.0f) / float(kSubdbivision) };

			//基準点d
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1) / float(kSubdbivision), 1.0f - float(latIndex + 1) / float(kSubdbivision) };
		}


	}















	//マテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSphere = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//色
	materialDataSphere->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSphere->enableLighting = true;//有効にするか否か

	materialDataSphere->uvTransform = materialDataSphere->uvTransform.MakeIdentity4x4();

	//WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込む

	wvpData->WVP = wvpData->WVP.MakeIdentity4x4();
	wvpData->World = wvpData->World. MakeIdentity4x4();

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
		position.y = 200.0f ;
		size = Vector2(100, 100);

		sprite->SetPosition(position);
		sprite->SetSize(size);
		sprite->SetAnchorPoint(Vector2{0.0f,0.0f });
		sprite->SetIsFlipY(0);
		sprite->SetTextureLeftTop(Vector2{ i*64.0f,0.0f });
		sprite->SetTextureSize(Vector2{ 64.0f,64.0f });
		
		i++;

	}

	float rotation{0};



  
	//wvpData用のTransform変数を作る

  
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };


	bool useMonsterBall = true;

	while (true) {//ゲームループ

		camera->Update();

		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

		input->Update();

		////更新
		//if (input->TriggerKey(DIK_0)) {

		//	transform.rotate.y += 0.3f;

		//}


    
		/*Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);


		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWindth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;*/

		
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

		/*	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;*/

		//ImGui_ImplDX12_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
		//ImGui::Begin("Setting");

		////CameraTransform
		//if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		//{/*
		//	ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
		//	ImGui::DragFloat3("CameraRotate", &cameraTransform.rotate.x, 0.01f);*/
		//}
		//ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		//// SphereSetColor
		//if (ImGui::CollapsingHeader("SetcolorSphere", ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	ImGui::ColorEdit4("*SetColor", &materialDataSphere->color.x);
		//}
		//// SphereTransform
		//if (ImGui::CollapsingHeader("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	ImGui::DragFloat3("*Scale", &transform.scale.x, 0.01f);
		//	ImGui::DragFloat3("*Rotate", &transform.rotate.x, 0.01f);
		//	ImGui::DragFloat3("*Transrate", &transform.translate.x, 0.01f);
		//}
		//// ModelTransform
		//if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	transformModel = object3D->GetTransform();

		//	ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
		//	ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
		//	ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);

		//	object3D->SetTransform(transformModel);
		//}
		////if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
		////{
		////	//SpriteTransform
		////	Vector2 size = sprite->GetSize();
		////	Vector2 position = sprite->GetPosition();
		////	float rotation = sprite->GetRotation();
		////	Vector4 spritecolor = sprite->GetColor();

		////	ImGui::ColorEdit4("*spriteColor", &spritecolor.x);
		////	ImGui::DragFloat2("*ScaleSprite", &size.x, 0.1f);
		////	ImGui::DragFloat("*RotateSprite", &rotation, 0.1f);
		////	ImGui::DragFloat2("*TransrateSprite", &position.x);
		////	sprite->setColor(spritecolor);
		////	sprite->SetPosition(position);
		////	sprite->SetRotation(rotation);
		////	sprite->SetSize(size);
		////}
		////uvTransformSprite
		//if (ImGui::CollapsingHeader("uvTransformSprite", ImGuiTreeNodeFlags_DefaultOpen))
		//{

		//	/*ImGui::DragFloat2("*UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//	ImGui::DragFloat2("*UVScale", &uvTransformSprite.scale.x, 0.01f, -1.0f, 1.0f);
		//	ImGui::SliderAngle("*UVRotate", &uvTransformSprite.rotate.z);*/


  //    
		//}


  //  
		////項目4
		//if (ImGui::CollapsingHeader("directionalLight", ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	/*ImGui::ColorEdit4("*LightSetColor", &directionalLightData->color.x);
		//	ImGui::DragFloat3("*Lightdirection", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);*/
		//}



  //  

		//ImGui::End();
		//ImGui::Render();




    
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





    
		////Sphere
		//dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		////現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		////wvp用のCBufferの場所を設定
		//dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//dxCommon->GetCommandList()->SetGraphics
		// 
		// 
		// ConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		////描画！
		////dxCommon->GetCommandList()->DrawInstanced(kSubdbivision * kSubdbivision * 6, 1, 0, 0);





		/*ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());*/






		dxCommon->End();








	}


#pragma region Release

	/*ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();*/

	/*CloseHandle(fenceEvent);*/



#ifdef _DEBUG

#endif // _DEBUG
#pragma endregion

	//WindowsAPI終了処理
	winApp->Finalize();
	//WindowsAPI解放
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstans()->Finalize();
	delete winApp;
	delete dxCommon;
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
	 
	
	

	return 0;

}
