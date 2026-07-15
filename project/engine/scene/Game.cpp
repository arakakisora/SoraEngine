#include "Game.h"
#include "SceneFactory.h"




void Game::Initialize()
{
	
	//初期化
	Framework::Initialize();
	
	sceneFactory = std::make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory.get());

	//シーンの変更
	//"TITELE"
	//"GAMEPLAY"
	//GAMEOVER
	//"GAMECLEAR"
	SceneManager::GetInstance()->ChangeScene("TITELE");


}

void Game::Finalize()
{
	//終了
	Framework::Finalize();
	
	

}

void Game::Update()
{
	
#ifdef _DEBUG

	imGuiManager->Begin();
#endif // _DEBUG
	//更新
	Framework::Update();

	
#ifdef _DEBUG
	offscreenRenderManager->DrawImGui();
	imGuiManager->End();
#endif // _DEBUG
}

void Game::Draw()
{

	//DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
	offscreenRenderManager->Begin();
	srvManager->PreDraw();
	SceneManager::GetInstance()->Draw();
	offscreenRenderManager->End();
	
	dxCommon->Begin();
	//描画
	offscreenRenderManager->Draw();
#ifdef _DEBUG
	imGuiManager->Draw();

#endif // _DEBUG
	dxCommon->End();
}
