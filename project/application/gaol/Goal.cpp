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
	// スケールの設定
	goalPosition_ = map->GetGoalPosition();
 	object3D_->SetTranslate(goalPosition_);
}

void Goal::Update(bool isGoal)
{
	// オブジェクトの更新
	object3D_->Update();
	// ゴールに到達したかどうかの更新
	isGoal_ = isGoal;
	if (isGoal_) {
		// シーンをゲームクリアに変更
		SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
	}




}

void Goal::Draw()
{
	// オブジェクトの描画
	object3D_->Draw();

}
