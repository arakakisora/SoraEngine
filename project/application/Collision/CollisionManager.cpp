#include "CollisionManager.h"
#include "Player.h"
#include "EnemyManager.h"
#include "PlayerBullet.h"
#include "MyMath.h"
#include <assert.h>

// 修正: std::unique_ptr に合わせて定義
std::unique_ptr<CollisionManager> CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance()
{
	if (!instance_) {
		instance_ = std::make_unique<CollisionManager>();
	}
	return instance_.get();
}

void CollisionManager::Finalize()
{
	instance_.reset();
}

void CollisionManager::AddCollider(Collider* collider)
{
    colliders_.push_back(collider);
    
}

void CollisionManager::Update()
{
    const size_t n = colliders_.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Collider* a = colliders_[i];
            Collider* b = colliders_[j];

            // どちらかが当たり判定無効ならスキップ
            if (!a->IsCollisionEnabled() || !b->IsCollisionEnabled()) {
                continue;
            }
			// レイヤー同士で当たり判定を取るべきか
            if (!CanCollide(a->GetLayer(), b->GetLayer())) {
                continue;
            }

            if (MyMath::IsCollision(a->GetAABB(), b->GetAABB())) {
                // 両方に知らせる
                a->OnCollision(b);
                b->OnCollision(a);
            }
        }
    }
}

bool CollisionManager::CanCollide(Collider::Layer a, Collider::Layer b) const
{
    // 当たり判定を取りたい組み合わせだけ true にする
    if (a == Collider::Layer::Player && b == Collider::Layer::Enemy) return true;
    if (a == Collider::Layer::Enemy && b == Collider::Layer::Player) return true;
	if (a == Collider::Layer::Player && b == Collider::Layer::Enemy2) return true;
	if (a == Collider::Layer::Enemy2 && b == Collider::Layer::Player) return true;

    if (a == Collider::Layer::Player && b == Collider::Layer::EnemyBullet) return true;
    if (a == Collider::Layer::EnemyBullet && b == Collider::Layer::Player) return true;


    if (a == Collider::Layer::PlayerBullet && b == Collider::Layer::Enemy) return true;
    if (a == Collider::Layer::Enemy && b == Collider::Layer::PlayerBullet) return true;
	if (a == Collider::Layer::PlayerBullet && b == Collider::Layer::Enemy2) return true;
	if (a == Collider::Layer::Enemy2 && b == Collider::Layer::PlayerBullet) return true;


    return false;
}

void CollisionManager::Clear() {
    
	colliders_.clear();
}
