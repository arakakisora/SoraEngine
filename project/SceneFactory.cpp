#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	BaseScene* newscene = nullptr;

	if (sceneName == "GAMEPLAY") {
		newscene = new GamePlayScene();
	}
	else if (sceneName == "TITELE") {
		newscene = new TitleScene();
	}
	return newscene;

}
