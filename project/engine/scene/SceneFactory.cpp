#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	BaseScene* newscene = nullptr;

	if (sceneName == "GAMEPLAY") {
		newscene = new GamePlayScene();
	}
	else if (sceneName == "TITELE") {
		newscene = new TitleScene();
	}
	else if (sceneName == "GAMEOVER") {
		newscene = new GameOverScene();
	}
	else if (sceneName == "GAMECLEAR") {
		newscene = new GameclearScene;
	}
	
	return newscene;

}
