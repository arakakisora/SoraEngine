#pragma once
#include <Object3D.h>
class MapChipField;
class Goal
{
public:
	~Goal() {
		if (object3D_) {
			delete object3D_;
			object3D_ = nullptr;
		}
	}
	void Initialize(MapChipField* map);
	void Update(bool isGoal);
	void Draw();
	bool GetIsGoal() const { return isGoal_; }
	void SetIsGoal(bool isGoal) { isGoal_ = isGoal; }
private:
	bool isGoal_ = false; // ゴールに到達したかどうか
	Object3D* object3D_ = nullptr; // ゴールの3Dオブジェクト
	Vector3 goalPosition_; // ゴールの位置
	

};

