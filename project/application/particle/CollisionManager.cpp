#include "CollisionManager.h"
#include "Player.h"
#include "EnemyManager.h"
#include "PlayerBullet.h"
#include "MyMath.h"

void CollisionManager::Initialize(Player* playerValue, EnemyManager* enemyValue)
{
	this->player = playerValue;
	this->enemy = enemyValue;

}

void CollisionManager::Update()
{
	AABB aabb1, aabb2;
	aabb1 = player->GetAABB();
	for (Enemy* provisionalenemy :enemy->GetEnemies()) {

		aabb2 = provisionalenemy->GetAABB();
		
		if (MyMath::IsCollision(aabb1, aabb2)) {

			player->SetIsDead(true); // プレイヤーが死亡する処理を追加
			
		}
	}

	// 弾と敵の衝突
	for (PlayerBullet* bullet : player->GetBullets()) { // GetBullets を追加で実装
		AABB bulletAABB = bullet->GetAABB();
		for (Enemy* provisionalenemy : enemy->GetEnemies()) {
			aabb2 = provisionalenemy->GetAABB();
			if (MyMath::IsCollision(bulletAABB, aabb2)) {
				bullet->OnCollison();  // 弾を削除
				provisionalenemy->OnCollision(bullet); // 敵の処理
				break; // 弾が消滅するので、これ以上判定を行わない
			}
		}
	}

	// 弾と敵2の衝突
	for (PlayerBullet* bullet : player->GetBullets()) { // GetBullets を追加で実装
		AABB bulletAABB = bullet->GetAABB();
		for (Enemy2* enemy2 : enemy->GetEnemies2()) {
			aabb2 = enemy2->GetAABB();
			if (MyMath::IsCollision(bulletAABB, aabb2)) {
				bullet->OnCollison();  // 弾を削除
				enemy2->OnCollision(bullet); // 敵の処理
				break; // 弾が消滅するので、これ以上判定を行わない
			}
		}
	}

	// プレイヤーと敵2の衝突
	for (Enemy2* enemy2 : enemy->GetEnemies2()) {
		aabb2 = enemy2->GetAABB();
		if (MyMath::IsCollision(aabb1, aabb2)) {
			player->SetIsDead(true); // プレイヤーが死亡する処理を追加
		}
	}
}
