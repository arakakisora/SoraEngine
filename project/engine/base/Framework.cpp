#include "Framework.h"


void Framework::Initialize()
{
	//初期化
	//WindousAPI初期化
	//ポインタ
	endRequst_ = false;

	winApp = new WinApp;
	winApp->Initialize();
	//DX初期化

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);
	//srvマネージャの初期化	
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);
	//テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
	//Input初期化
	Input::GetInstans()->Initialize(winApp);
	//Audio初期化
	audio_->GetInstance()->Initialize();
	//スプライト共通部分の初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon);


	//3Dモデルマネージャの初期化
	ModelManager::GetInstans()->Initialize(dxCommon, srvManager);

	//3Dオブジェクト共通部の初期化

	Object3DCommon::GetInstance()->Initialize(dxCommon);
	//object3DCommon->SetDefaultCamera(camera);




#ifdef _DEBUG
	//imguiMnagerの初期化

	imGuiMnager = new ImGuiManager();
	imGuiMnager->Initialize(dxCommon, winApp);

#endif // _DEBUG

	//sceneManager = new SceneManager();

}

void Framework::Finalize()
{
	delete sceneFactory;

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
	delete srvManager;
#ifdef _DEBUG
	delete imGuiMnager;
#endif // _DEBUG

	Input::GetInstans()->Finalize();

	SpriteCommon::GetInstance()->Finalize();
	Object3DCommon::GetInstance()->Finalize();
	//SceneManagerの解放
	SceneManager::GetInstance()->Finalize();

}

void Framework::Update()
{

	//Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		//ゲームループを抜ける
		endRequst_ = true;
	}


	Input::GetInstans()->Update();
	SceneManager::GetInstance()->Update();



}

void Framework::Run()
{

	Initialize();
	while (true)
	{
		Update();

		if (IsEndRequest()) {

			break;
		}
		//描画
		Draw();


	}
	Finalize();

}
