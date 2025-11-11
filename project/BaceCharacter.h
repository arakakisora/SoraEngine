#pragma once
#include "Collider.h"
class BaceCharacter :public Collider
{
public:
	
	//中心座標を取得
	virtual Vector3 GetCenterPosition() const override;

};

