#include "EnemyManager.h"
#include "MapChipField.h"
#include "Object3DCommon.h"
#include "CollisionManager.h"
#include "Enemy.h"
#include "Enemy2.h"
#include <algorithm>
#include <cassert>
#include <memory>

#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI

// コンストラクタ: デフォルトの敵ファクトリを登録
EnemyManager::EnemyManager()
{
	// Factory: 通常の Enemy（ID 1）
	RegisterFactory(kFactoryIdDefaultEnemy, [](const Vector3& pos, MapChipField* map) -> std::unique_ptr<EnemyBase> {
		// unique_ptr を用いて所有権を明確にする
		auto e = std::make_unique<Enemy>();
		e->SetMapChipField(map); // Initialize が map を参照する可能性があるため先にセット
		e->SetPosition(pos);
		e->Initialize();
		return e;
	});

	// Factory: Enemy2（ID 2）
	RegisterFactory(kFactoryIdEnemy2, [](const Vector3& pos, MapChipField* map) -> std::unique_ptr<EnemyBase> {
		auto e = std::make_unique<Enemy2>();
		e->SetMapChipField(map);
		e->SetPosition(pos);
		e->Initialize();
		return e;
	});
}

// RegisterFactory: ファクトリを登録（ID -> factory）
void EnemyManager::RegisterFactory(int id, Factory factory)
{
	factories_[id] = std::move(factory);
}

// Initialize: マップから敵情報を読み取り、factory で生成して所有リストに追加
void EnemyManager::Initialize(MapChipField* map) {
	map_ = map;
	// マップから敵の位置リストとタイプリストを取得
	std::vector<Vector3> enemyPositions = map_->GetEnemyPositions();
	Enemynumber = map_->GetEnemyNumbers();

	// 配列長の不整合に備えて最小長でループ
	size_t count = std::min(enemyPositions.size(), Enemynumber.size());
	for (size_t i = 0; i < count; ++i) {
		const Vector3& enemyPos = enemyPositions[i];
		int id = Enemynumber[i];
		// ファクトリから敵を生成（存在しないIDならアサート）
		auto it = factories_.find(id);
		if (it != factories_.end()) {
			std::unique_ptr<EnemyBase> newEnemy = it->second(enemyPos, map_);
			if (newEnemy) enemies_.push_back(std::move(newEnemy));
		}
		else {
			assert(false && "未登録の敵IDです");
		}
	}
}

// Update: 全敵を更新し、演出完了で消す
void EnemyManager::Update() {

	// 敵の Update 呼び出し
	for (auto& enemy : enemies_) {
		if (enemy) {
			enemy->Update();
		}
	}

	// 死亡して演出完了フラグが立っている敵をリストから削除（unique_ptr のデストラクタで解放される）
	enemies_.remove_if([](const std::unique_ptr<EnemyBase>& enemy) {
		return enemy && enemy->IsPendingRemove();
	});
}

// Draw: 全敵の描画を呼ぶ
void EnemyManager::Draw() {
	for (const auto& enemy : enemies_) {
		if (enemy) {
			enemy->Draw();
		}
	}
}

// EnemyObjectUpdate: 内部の Object3D の更新を行う（描画用の GPU 更新など）
void EnemyManager::EnemyObjectUpdate()
{
	for (const auto& enemy : enemies_) {
		if (enemy) {
			if (auto* obj = enemy->GetObject3D()) {
				obj->Update();
			}
		}
	}
}

// RegisterColliders: CollisionManager に自分の敵をコライダー登録する
void EnemyManager::RegisterColliders()
{
	auto* cm = CollisionManager::GetInstance();
	for (const auto& e : enemies_) {
		if (e) cm->AddCollider(e.get()); // Collider* を渡す（unique_ptr の所有はここに残る）
	}
}

