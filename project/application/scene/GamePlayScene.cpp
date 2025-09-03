#include "GamePlayScene.h"
#include <ModelManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include <ImGuiManager.h>
#include <imgui.h>
#include <Input.h>
#include <CameraManager.h>
#include <ParticleMnager.h>
#include <Logger.h>
#include <LineCommon.h>


void GamePlayScene::Initialize()
{
	//カメラの生成
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstance()->AddCamera("maincam", camera1.get());

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");


	
	


}

void GamePlayScene::Finalize()
{
	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->Finalize();
	

}

void GamePlayScene::Update()
{
	//カメラの更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();

	

#ifdef _DEBUG

#endif // _DEBUG
}

void GamePlayScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();


	Object3DCommon::GetInstance()->SkinNingCommonDraw();


	ParticleMnager::GetInstance()->Draw();

	LineCommon::GetInstance()->Draw();


#pragma endregion

#pragma region スプライト描画
	SpriteCommon::GetInstance()->CommonDraw();
	

#pragma endregion
}

void GamePlayScene::LoadModel()
{

	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("axis.obj");
	ModelManager::GetInstans()->LoadModel("plane.gltf");
	ModelManager::GetInstans()->LoadModel("sphere.obj");
	ModelManager::GetInstans()->LoadModel("cube.obj");


}









