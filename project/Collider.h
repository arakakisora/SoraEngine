#pragma once
#include "MyMath.h"
class Collider
{
public:
	virtual ~Collider() = default;

	//当たったよ
	virtual void OnCollision() {};
	//中心座標の取得
	virtual Vector3 GetCenterPosition() const = 0;

	//アクセッサ
	//衝突半径の設定
	void SetRadius(float radius) { radius_ = radius; };
	//衝突半径の取得
	float GetRadius() const { return radius_; };

private:
	//衝突半径
	float radius_ = 1.0f;
	

};

