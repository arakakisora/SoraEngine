#pragma once
#include "Collider.h"	
/// <summary>
/// 当たり判定管理クラス
/// </summary>
class CollisionManager
{
public:
	void AddCollider(Collider* collider);

    void Clear() {
        colliders_.clear();
    }

	void Update();

	
private:
	bool CanCollide(Collider::Layer a, Collider::Layer b) const ;
	std::vector<Collider*> colliders_;

};

