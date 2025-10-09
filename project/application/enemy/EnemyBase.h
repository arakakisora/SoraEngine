// EnemyBase.h
#pragma once
#include "Object3D.h"
#include "MapChipField.h"


class Player;
class PlayerBullet;
class EnemyBase {
public:
	virtual ~EnemyBase() {}

	virtual void Initialize(Object3D* obj, const Vector3& position) = 0;
	virtual void Update(MapChipField* mapChipField) = 0;
	virtual void Draw() = 0;
	virtual AABB GetAABB() = 0;
	
	virtual void OnCollision(const PlayerBullet* bullet) = 0;

	virtual Vector3 GetWorldPosition() = 0;
	virtual bool IsDead() const = 0;
	
};