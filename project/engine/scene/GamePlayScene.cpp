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
#include <Logger.h>

void GamePlayScene::Initialize()
{
	//カメラの生成
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstans()->AddCamera("maincam", camera1.get());

	//カメラの生成
	camera2 = std::make_unique<Camera>();
	camera2->SetTranslate({ 0,6,-20, });//カメラの位置
	camera2->SetRotate({ 0.3f,0,0 });//カメラの向き
	CameraManager::GetInstans()->AddCamera("subcam", camera2.get());

	// デフォルトカメラを設定
	CameraManager::GetInstans()->SetActiveCamera("maincam");


	// ロード処理全体の時間を計測
	auto start = std::chrono::high_resolution_clock::now();


	// ロード処理用スレッドを作成
	std::thread modelThread([&]() {
		LoadModel();
		});

	std::thread particleThread([&]() {
		Loadparticle();
		});

	std::thread audioThread([&]() {
		LoadAudio();
		});

	// スレッドの終了を待機
	modelThread.join();
	particleThread.join();
	audioThread.join();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	Logger::Log("Total loading time: " + std::to_string(duration.count()) + " milliseconds");



	object3D = std::make_unique<Object3D>();
	object3D->Initialize(Object3DCommon::GetInstance());
	object3D->SetModel("sphere.obj");
	object3D->SetLighting(true);
	object3D->SetDirectionalLightIntensity(1.0f);

	terrain = std::make_unique<Object3D>();
	terrain->Initialize(Object3DCommon::GetInstance());
	terrain->SetModel("terrain.obj");
	terrain->SetTransform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,-1.0f,0.0f} });
	terrain->SetLighting(true);
	terrain->SetDirectionalLightIntensity(1.0f);

	//スプライトの生成
	sprite = std::make_unique<Sprite>();
	sprite->Initialize(SpriteCommon::GetInstance(), "Resources/uvChecker.png");

	light = true;

	
	particleEmitter = std::make_unique<ParticleEmitter>(Vector3(10, 0, 0), 1.0f, 0.0f, 100, "Pariticle1");
}

void GamePlayScene::Finalize()
{
	CameraManager::GetInstans()->RemoveCamera("maincam");
	CameraManager::GetInstans()->RemoveCamera("subcam");
	CameraManager::GetInstans()->Finalize();

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstans()->GetActiveCamera()->Update();
	object3D->Update();
	terrain->Update();

	//パーティクルの更新
	particleEmitter->Update();
	sprite->Update();

#ifdef _DEBUG

	if (ImGui::CollapsingHeader("Camera Control", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Switch to Main Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("maincam");
		}
		if (ImGui::Button("Switch to Sub Camera")) {
			CameraManager::GetInstans()->SetActiveCamera("subcam");
		}

		//カメラの位置
		Transform cameraTransform = CameraManager::GetInstans()->GetActiveCamera()->GetTransform();
		if (ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.01f)) {
			CameraManager::GetInstans()->GetActiveCamera()->SetTranslate(cameraTransform.translate);
		}
		//カメラの向き
		if (ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.01f)) {
			CameraManager::GetInstans()->GetActiveCamera()->SetRotate(cameraTransform.rotate);
		}


	}

	if (ImGui::CollapsingHeader("object3D", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//potision
		Transform transform = object3D->GetTransform();
		if (ImGui::DragFloat3("obj3Position", &transform.translate.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
		//rotation
		if (ImGui::DragFloat3("obj3Rotation", &transform.rotate.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
		//scale
		if (ImGui::DragFloat3("obj3Scale", &transform.scale.x, 0.01f)) {
			object3D->SetTransform(transform);
		}
		//color
		Vector4 color = object3D->GetColor();
		if (ImGui::ColorEdit4("obj3Color", &color.x)) {
			object3D->SetColor(color);
		}
		//ライトのオンオフ
		if (ImGui::Checkbox("Enable Lighting", &light)) {
			object3D->SetLighting(light);

		}
	}

	//sprite
	if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//position
		Vector2 position = sprite->GetPosition();
		if (ImGui::DragFloat2("spritePosition", &position.x, 0.01f)) {
			sprite->SetPosition(position);
		}
		//rotation
		float rotation = sprite->GetRotation();
		if (ImGui::DragFloat("spriteRotation", &rotation, 0.01f)) {
			sprite->SetRotation(rotation);
		}
		//scale
		Vector2 scale = sprite->GetSize();
		if (ImGui::DragFloat2("spriteScale", &scale.x, 0.01f)) {
			sprite->SetSize(scale);
		}
		//color
		Vector4 color = sprite->GetColor();
		if (ImGui::ColorEdit4("spriteColor", &color.x)) {
			sprite->setColor(color);
		}
	}

	ImGui::Text("gamePlayScene %d");
	if (ImGui::Button("GameClearScene"))
	{
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
	if (ImGui::Button("GameOverScene"))
	{
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}

	//ライト
	if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector4 color = object3D->GetDirectionalLight().color;
		Vector3 direction = object3D->GetDirectionalLight().direction;
		float intensity = object3D->GetDirectionalLight().intensity;
		if (ImGui::ColorEdit4("Color", &color.x)) {
			object3D->SetDirectionalLightColor(color);

		}
		if (ImGui::DragFloat3("Direction", &direction.x, 0.01f)) {
			object3D->SetDirectionalLightDirection(direction);
		}
		if (ImGui::DragFloat("Intensity", &intensity, 0.01f)) {
			object3D->SetDirectionalLightIntensity(intensity);

		}
		//ライトのオンオフ
		if (ImGui::Checkbox("Enable DirectionalLight", &directionLight)) {
			object3D->SetDirectionalLightEnable(directionLight);
		}
		
	}
	//ポイントライト
	if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector4 color = object3D->GetPointLight().color;
		Vector3 position = object3D->GetPointLight().position;
		float intensity = object3D->GetPointLight().intensity;
		float decay = object3D->GetPointLightDecay();
		float radius = object3D->GetPointLightRadius();
		//ライトのオンオフ
		if (ImGui::Checkbox("Enable PointLight", &pointLight)) {
			object3D->SetPointLightEnable(pointLight);
			terrain->SetPointLightEnable(pointLight);
		}
		
		if (ImGui::ColorEdit4("pointColor", &color.x)) {
			object3D->SetPointLightColor(color);

		}
		if (ImGui::DragFloat3("pointPosition", &position.x, 0.01f)) {
			object3D->SetPointLightPosition(position);
			terrain->SetPointLightPosition(position);

		}
		if (ImGui::DragFloat("pointIntensity", &intensity, 0.01f)) {
			object3D->SetPointLightIntensity(intensity);
			terrain->SetPointLightIntensity(intensity);
		}
		if (ImGui::DragFloat("pointRadius", &radius, 0.01f)) {
			object3D->SetPointLightRadius(radius);
			terrain->SetPointLightRadius(radius);
		}
		if (ImGui::DragFloat("pointDecay", &decay, 0.01f)) {
			object3D->SetPointLightDecay(decay);
			terrain->SetPointLightDecay(decay);
		}
		
	}
	//スポットライト
	if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector4 color = object3D->GetSpotLight().color;
		Vector3 position = object3D->GetSpotLight().position;
		Vector3 direction = object3D->GetSpotLight().direction;
		float intensity = object3D->GetSpotLight().intensity;
		float distance = object3D->GetSpotLight().distance;
		float decay = object3D->GetSpotLight().decay;
		float consAngle = object3D->GetSpotLight().consAngle;
		float cosFalloffstrt = object3D->GetSpotLight().cosFalloffstrt;
		//ライトのオンオフ
		if (ImGui::Checkbox("Enable SpotLight", &spotLight)) {
			object3D->SetSpotLightEnable(spotLight);
			terrain->SetSpotLightEnable(spotLight);
		}
		if (ImGui::ColorEdit4("spotColor", &color.x)) {
			object3D->SetSpotLightColor(color);
			terrain->SetSpotLightColor(color);
		}
		if (ImGui::DragFloat3("spotPosition", &position.x, 0.01f)) {
			object3D->SetSpotLightPosition(position);
			terrain->SetSpotLightPosition(position);
		}
		if (ImGui::DragFloat3("spotDirection", &direction.x, 0.01f)) {
			object3D->SetSpotLightDirection(direction);
			terrain->SetSpotLightDirection(direction);
		}
		if (ImGui::DragFloat("spotIntensity", &intensity, 0.01f)) {
			object3D->SetSpotLightIntensity(intensity);
			terrain->SetSpotLightIntensity(intensity);
		}
		if (ImGui::DragFloat("spotDistance", &distance, 0.01f)) {
			object3D->SetSpotLightDistance(distance);
			terrain->SetSpotLightDistance(distance);
		}
		if (ImGui::DragFloat("spotDecay", &decay, 0.01f)) {
			object3D->SetSpotLightDecay(decay);
			terrain->SetSpotLightDecay(decay);
		}
		if (ImGui::DragFloat("spotConsAngle", &consAngle, 0.01f)) {
			object3D->SetSpotLightConsAngle(consAngle);
			terrain->SetSpotLightConsAngle(consAngle);
		}
		if (ImGui::DragFloat("spotCosFalloffstrt", &cosFalloffstrt, 0.01f)) {
			object3D->SetSpotLightCosFalloffstrt(cosFalloffstrt);
			terrain->SetSpotLightCosFalloffstrt(cosFalloffstrt);
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
		//エミット位置
		Vector3 position = particleEmitter->GetPosition();
		if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
			particleEmitter->SetPosition(position);
		}

	}

	
#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	object3D->Draw();
	terrain->Draw();



	ParticleMnager::GetInstance()->Draw();

#pragma endregion

#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//sprite->Draw();

#pragma endregion
}

void GamePlayScene::LoadModel()
{

	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("axis.obj");
	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("sphere.obj");
	ModelManager::GetInstans()->LoadModel("terrain.obj");
	

}

void GamePlayScene::Loadparticle()
{

	//パーティクルの初期化
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle1", "Resources/uvChecker.png", "sphere.obj");
	ParticleMnager::GetInstance()->CreateParticleGroup("Pariticle2", "Resources/uvChecker.png", "plane.obj");

}

void GamePlayScene::LoadAudio()
{
	//サウンドの読み込み
	sampleSoundData = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");//今のところwavのみ対応
	

}

