#include "Game.h"
#include "SceneFactory.h"



void Game::Initialize()
{
	
	//初期化
	Framework::Initialize();
	
	sceneFactory = new SceneFactory();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory);

	//シーンの変更
	//"TITELE"
	//"GAMEPLAY"
	//GAMEOVER
	//"GAMECLEAR"
	SceneManager::GetInstance()->ChangeScene("GAMEPLAY");

}

void Game::Finalize()
{
	//終了
	Framework::Finalize();
	
	

}

void Game::Update()
{
	
#ifdef _DEBUG

	imGuiMnager->Begin();
#endif // _DEBUG
	//更新
	Framework::Update();


	
#ifdef _DEBUG
	imGuiMnager->End();
#endif // _DEBUG
}

void Game::Draw()
{

	//DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
	dxCommon->Begin();
	srvManager->PreDraw();

	SceneManager::GetInstance()->Draw();
#ifdef _DEBUG
	imGuiMnager->Draw();
#endif // _DEBUG
	dxCommon->End();
}
