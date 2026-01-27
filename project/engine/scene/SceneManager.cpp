#include "SceneManager.h"
#include <cassert>

std::unique_ptr <SceneManager> SceneManager::instance_ = nullptr;
SceneManager* SceneManager::GetInstance()
{
	
	if (instance_ == nullptr) {
		instance_ = std::make_unique <SceneManager>();
	}
	return instance_.get();
}
void SceneManager::Update()
{

	//シーンの切り替え
	if (nextScene) {
		//旧シーンの終了処理
		if (currentScene) {
			currentScene->Finalize();
			currentScene.reset();
		}
		//新シーンの初期化（所有権移譲）
		currentScene = std::move(nextScene);

		currentScene->SetSceneManager(this);

		//新シーンの初期化
		currentScene->Initialize();
	}

	//現在のシーンの更新
	if (currentScene) {
		currentScene->Update();
	}

}

void SceneManager::Draw()
{
	//現在のシーンの描画
	if (currentScene) {
		currentScene->Draw();
	}
}

void SceneManager::Finalize()
{
	if (currentScene) {
		currentScene->Finalize();
		currentScene.reset();
	}
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	assert(sceneFactory);
	assert(nextScene==nullptr);

	// sceneFactory の CreateScene は std::unique_ptr を返す
	nextScene = sceneFactory->CreateScene(sceneName);

}


