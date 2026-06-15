#include "CollisionManager.h"
#include "Player.h"
#include "EnemyManager.h"

#include "MyMath.h"
#include <assert.h>
#include <array>


std::unique_ptr<CollisionManager> CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance()
{
	// シングルトン実体がなければ生成して返す
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
	// コライダーを登録
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
	// レイヤー同士の当たり判定を定義するテーブル
    constexpr std::size_t LAYER_COUNT = 5;
    static constexpr std::array<std::array<bool, LAYER_COUNT>, LAYER_COUNT> table{{
        /* Player      */ std::array<bool, LAYER_COUNT>{ false, true,  true,  false, true  },
        /* Enemy       */ std::array<bool, LAYER_COUNT>{ true,  false, false, true,  false },
        /* Enemy2      */ std::array<bool, LAYER_COUNT>{ true,  false, false, true,  false },
        /* PlayerBullet*/ std::array<bool, LAYER_COUNT>{ false, true,  true,  false, false },
        /* EnemyBullet */ std::array<bool, LAYER_COUNT>{ true,  false, false, false, false },
    }};

    const auto ia = static_cast<std::size_t>(a);
    const auto ib = static_cast<std::size_t>(b);

    if (ia >= LAYER_COUNT || ib >= LAYER_COUNT) {
        // 未知のレイヤーは false を返す（安全側）
        return false;
    }

    return table[ia][ib];
}

void CollisionManager::Clear() {
    
	colliders_.clear();
}
