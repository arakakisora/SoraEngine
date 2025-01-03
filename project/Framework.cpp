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
	//カメラの生成	
	camera = new Camera();
	//スプライト共通部分の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);
	//3Dモデルマネージャの初期化
	ModelManager::GetInstans()->Initialize(dxCommon, srvManager);
	//カメラの生成	
	camera = new Camera();
	//3Dオブジェクト共通部の初期化
	object3DCommon = new Object3DCommon;
	object3DCommon->Initialize(dxCommon);
	object3DCommon->SetDefaultCamera(camera);



#ifdef _DEBUG
	//imguiMnagerの初期化

	imGuiMnager = new ImGuiManager();
	imGuiMnager->Initialize(dxCommon, winApp);

#endif // _DEBUG



}

void Framework::Finalize()
{
}

void Framework::Update()
{


	//Windowsのメッセージ処理
	if (winApp->ProcessMessage()) {
		//ゲームループを抜ける
		endRequst_ = true;
	}
	
	camera->Update();
	Input::GetInstans()->Update();


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
