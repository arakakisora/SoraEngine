#include "Goal.h"
#include "Object3DCommon.h"
#include "MapChipField.h"
#include "SceneManager.h"

void Goal::Initialize(MapChipField* map)
{

	// ゴールの生成
	object3D_ = new Object3D();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("cube.obj");

	goalPosition_ = map->GetGoalPosition();
 	object3D_->SetTranslate(goalPosition_);
}

void Goal::Update(bool isGoal)
{
	object3D_->Update();
	isGoal_ = isGoal;
	if (isGoal_) {
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}




}

void Goal::Draw()
{

	object3D_->Draw();

}
