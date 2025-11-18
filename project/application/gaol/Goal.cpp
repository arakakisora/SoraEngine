#include "Goal.h"
#include "Object3DCommon.h"
#include "MapChipField.h"
#include "SceneManager.h"

void Goal::Initialize(MapChipField* map, Player* paleyr)
{

	// ゴールの生成
	object3D_ = new Object3D();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("cube.obj");
	// スケールの設定
	goalPosition_ = map->GetGoalPosition();
	object3D_->SetTranslate(goalPosition_);

	stageClearEffect_ = std::make_unique<StageEndEffect>();
	stageClearEffect_->Initialize(paleyr->GetObject3D(), goalPosition_);

	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();
	isFadeOutStarted_ = false;
	
}

void Goal::Update(bool isGoal, float deltaTime)
{
	fadeManager_.Update();
	// オブジェクトの更新
	object3D_->Update();

	// ゴールに到達したかどうかの更新
	isGoal_ = isGoal;
	if (isGoal_ && !isEffectStarted_) {
		isEffectStarted_ = true;
		stageClearEffect_->Begin();
	}

	if (isEffectStarted_) {
		stageClearEffect_->Update(deltaTime);

		if (stageClearEffect_->IsFinished() && !isFadeOutStarted_) {
			isFadeOutStarted_ = true;
			fadeManager_.StartFadeOut();
		}
	}

	if (fadeManager_.IsFadeOutFinished()) {
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}
}

void Goal::Draw()
{
	// オブジェクトの描画
	if (!isEffectStarted_) {
		
		object3D_->Draw();
	}

	if (isEffectStarted_) {
		stageClearEffect_->Draw();
	}

	
}

void Goal::Draw2D()
{
	fadeManager_.Draw();

}
