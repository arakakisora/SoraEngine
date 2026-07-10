#include "DebugScene.h"
#include <Input.h>
#include <SceneManager.h>
#include <Object3DCommon.h>
#include <SpriteCommon.h>
#include <CameraManager.h>
#include <memory>
#include <ModelManager.h>
#include <cmath>
#include <imgui.h>

void DebugScene::Initialize()
{
	
}

void DebugScene::Finalize()
{
	// CameraManager から先にカメラを削除してから unique_ptr をリセットする
	CameraManager::GetInstance()->RemoveCamera("maincam");

}
void DebugScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	

#ifdef USE_IMGUI
	ImGui::Begin("SWgame");

	ImGui::End();
#endif
}

void DebugScene::Draw()
{

	SpriteCommon::GetInstance()->CommonDraw();

#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();


	ParticleManager::GetInstance()->Draw();
#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//Spriteの描画
	

}
