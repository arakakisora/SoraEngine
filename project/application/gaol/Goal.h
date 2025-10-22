#pragma once
#include <Object3D.h>
class MapChipField;
class Goal
{
public:

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Goal() {
		if (object3D_) {
			delete object3D_;
			object3D_ = nullptr;
		}
	}
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(MapChipField* map);
	/// <summary>
	/// 更新
	/// </summary>
	void Update(bool isGoal);
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	// Getter Setter
	/// <summary>
	/// ゴールに到達したかどうかを取得します
	/// </summary>
	bool GetIsGoal() const { return isGoal_; }
	/// <summary>
	/// ゴールに到達したかどうかを設定します
	/// </summary>
	void SetIsGoal(bool isGoal) { isGoal_ = isGoal; }
private:
	bool isGoal_ = false; // ゴールに到達したかどうか
	Object3D* object3D_ = nullptr; // ゴールの3Dオブジェクト
	Vector3 goalPosition_; // ゴールの位置
	

};

