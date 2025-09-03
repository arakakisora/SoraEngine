#include "SceneManager.h"
#include <cassert>
#include <imgui.h>

SceneManager* SceneManager::instance_ = nullptr;
SceneManager* SceneManager::GetInstance()
{
	
	if (instance_ == nullptr) {
		instance_ = new SceneManager();
	}
	return instance_;
}
void SceneManager::Update()
{

	//シーンの切り替え
	if (nextScene) {
		//旧シーンの終了処理
		if (currentScene) {
			currentScene->Finalize();
			delete currentScene;
		}
		//新シーンの初期化
		currentScene = nextScene;
		nextScene = nullptr;

		currentScene->SetSceneManager(this);

		//新シーンの初期化
		currentScene->Initialize();
	}

	//現在のシーンの更新
	currentScene->Update();
	ImguiDebug();

}

void SceneManager::Draw()
{
	//現在のシーンの描画
	currentScene->Draw();
}

void SceneManager::Finalize()
{
	currentScene->Finalize();
	delete currentScene;

}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	assert(sceneFactory);
	assert(nextScene==nullptr);

	nextScene = sceneFactory->CreateScene(sceneName);

}

void SceneManager::ImguiDebug()
{
	ImGui::Begin("SceneManager");
	if (ImGui::Button("GameClearScene"))
	{
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
	if (ImGui::Button("GameOverScene"))
	{
		SceneManager::GetInstance()->ChangeScene("GAMEOVER");
	}
	if (ImGui::Button("TitleScene"))
	{
		SceneManager::GetInstance()->ChangeScene("TITELE");
	}
	if (ImGui::Button("DebugScene"))
	{
		SceneManager::GetInstance()->ChangeScene("DEBUG");
	}
	ImGui::End();

}


