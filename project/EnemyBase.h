// EnemyBase.h
#pragma once
#include "Object3D.h"
#include "MapChipField.h"


class Player;
class PlayerBullet;
class EnemyBase {
public:
	virtual ~EnemyBase() {}

	// 仮想デストラクタ
	virtual ~EnemyBase() = default;
	// 初期化
	virtual void Initialize() = 0;
	// 更新
	virtual void Update(MapChipField* mapChipField) = 0;
	// 描画
	virtual void Draw() = 0;


	virtual void OnCollision(const PlayerBullet* bullet) = 0;

	virtual Vector3 GetWorldPosition() ;
	virtual bool IsDead() const ;

	//Vector3 GetWorldPosition();
	virtual AABB GetAABB();
	void SetPosition(const Vector3& pos) { posiition_ = pos; }
	
protected:
	Vector3 posiition_ = { 0,0,0 };
	Vector3 velocity_ = { 0.1,0,0 };
	// 回転
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	int HP;

	float acceleration_ = 0.1;
	float ground = 0.0f;
	float maxSpeed_ = 2.0f;
	float radius = 0.5f;
	bool onGround_ = false;
	bool isDead_ = false;

	std::unique_ptr<Object3D> object3d_;
};