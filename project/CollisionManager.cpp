#include "CollisionManager.h"
#include "Player.h"
#include "EnemyManager.h"
#include "PlayerBullet.h"
#include "MyMath.h"

void CollisionManager::Initialize(Player* player, EnemyManager* enemy)
{
	this->player = player;
	this->enemy = enemy;

}

void CollisionManager::Update()
{
	AABB aabb1, aabb2;
	aabb1 = player->GetAABB();
	for (Enemy* enemy :enemy->GetEnemies()) {

		aabb2 = enemy->GetAABB();

		if (MyMath::IsCollision(aabb1, aabb2)) {

			player->OnCollision(enemy);
			enemy->OnCollision(player);
		}
	}

	// 弾と敵の衝突
	for (PlayerBullet* bullet : player->GetBullets()) { // GetBullets を追加で実装
		AABB bulletAABB = bullet->GetAABB();
		for (Enemy* enemy : enemy->GetEnemies()) {
			aabb2 = enemy->GetAABB();
			if (MyMath::IsCollision(bulletAABB, aabb2)) {
				bullet->OnCollison();  // 弾を削除
				enemy->OnCollision(bullet); // 敵の処理
				break; // 弾が消滅するので、これ以上判定を行わない
			}
		}
	}
}
