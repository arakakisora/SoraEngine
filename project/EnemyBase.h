// EnemyBase.h
#pragma once
#include "Object3D.h"
#include "MapChipField.h"


class Player;
class PlayerBullet;
class EnemyBase {
public:
	virtual void Initialize(Object3D* obj, const Vector3& position);
	
	virtual void Update(MapChipField* mapChipField);

	virtual void Draw();

	virtual bool IsDead() ;

private:
	
};