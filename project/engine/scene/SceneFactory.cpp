#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include "GameClearScene.h"
#include "GameOverScene.h"
#include "StageSelectScene.h"
#include <memory>
#include <cassert>

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
	std::unique_ptr<BaseScene> newscene = nullptr;

	if (sceneName == "GAMEPLAY") {
		newscene = std::make_unique<GamePlayScene>();
	}
	else if (sceneName == "TITELE") {
		newscene = std::make_unique<TitleScene>();
	}
	else if (sceneName == "GAMECLEAR") {
		newscene = std::make_unique<GameClearScene>();
	}
	else if (sceneName == "GAMEOVER") {
		newscene = std::make_unique<GameOverScene>();
	}
	else if (sceneName == "STAGESELECT") {
		newscene = std::make_unique<StageSelectScene>();
	}
	else {
		assert(0);
	}

	return newscene;
}
