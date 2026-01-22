#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include "GameClearScene.h"
#include "GameOverScene.h"
#include "StageSelectScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	BaseScene* newscene = nullptr;

	if (sceneName == "GAMEPLAY") {
		newscene = new GamePlayScene();
	}
	else if (sceneName == "TITELE") {
		newscene = new TitleScene();
	}
	else if (sceneName == "GAMECLEAR") {
		newscene = new GameClearScene();
	}
	else if (sceneName == "GAMEOVER") {
		newscene = new GameOverScene();
	} 
	else if (sceneName == "STAGESELECT") {
		newscene = new StageSelectScene();
	}
	else {
		assert(0);
	}

	return newscene;

}
