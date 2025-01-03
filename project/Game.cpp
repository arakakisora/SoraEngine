#include "Game.h"





void Game::Initialize()
{
	

	Framework::Initialize();






	
	camera->SetRotate({ 0,0,0, });
	camera->SetTranslate({ 0,0,-30, });
	object3DCommon->SetDefaultCamera(camera);


	object3D = new Object3D();
	object3D2nd = new Object3D();
	sounddata1 = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");

	//wvpData用のTransform変数を作る
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	useMonsterBall = true;

	bgm = false;



	//テクスチャの初期化
	std::string textureFilePath[2]{ "Resources/monsterBall.png" ,"Resources/uvChecker.png" };

	for (uint32_t i = 0; i < 12; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, textureFilePath[1]);
		sprites.push_back(sprite);

	}
	ModelManager::GetInstans()->LoadModel("plane.obj");
	ModelManager::GetInstans()->LoadModel("axis.obj");


	//3Dオブジェクトの初期化
	object3D->Initialize(object3DCommon);
	object3D->SetModel("plane.obj");

	//3Dオブジェクトの初期化

	object3D2nd->Initialize(object3DCommon);
	object3D2nd->SetModel("plane.obj");









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

	rotation = { 0 };






}

void Game::Finalize()
{

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

	Input::GetInstans()->Finalize();

	//スプライト解放
	delete spriteCommon;
	for (Sprite* sprite : sprites) {
		delete sprite; // 各Spriteオブジェクトの削除
	}
	//3Dオブジェクト解放
	delete object3DCommon;
	delete object3D;
	delete object3D2nd;
	delete camera;
	delete srvManager;


}

void Game::Update()
{

	Framework::Update();
	
	
#ifdef _DEBUG

	imGuiMnager->Begin();
	

#endif // _DEBUG


	if (!bgm) {

		Audio::GetInstance()->SoundPlayWave(sounddata1);
		bgm = true;
	}

	if (Input::GetInstans()->TriggerKey(DIK_SPACE)) {

		Audio::GetInstance()->SoundUnload(&sounddata1);

	}




	//更新
	if (Input::GetInstans()->PushKey(DIK_D)) {

		rotation += 0.03f;

	}

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

	//rotation += 0.03f;
	object3D->SetRotate(Vector3{ 0,rotation ,0 });
	object3D->Update();

	object3D2nd->SetRotate(Vector3{ 0,0 ,rotation });
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






}

void Game::Draw()
{

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
	imGuiMnager->Draw();
#endif // _DEBUG


	dxCommon->End();
}
