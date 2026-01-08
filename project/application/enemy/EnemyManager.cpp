#include "EnemyManager.h"
#include "MapChipField.h"
#include "Object3DCommon.h"
#include "CollisionManager.h"
#include "Enemy.h"
#include "Enemy2.h"
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI

EnemyManager::EnemyManager()
{
	// デフォルトの敵タイプを登録
	RegisterFactory(1, [](const Vector3& pos, MapChipField* map) -> EnemyBase* {
		EnemyBase* e = new Enemy();
		e->SetMapChipField(map); // Initialize が map を参照する可能性があるため先にセット
		e->SetPosition(pos);
		e->Initialize();
		return e;
	});

	RegisterFactory(2, [](const Vector3& pos, MapChipField* map) -> EnemyBase* {
		EnemyBase* e = new Enemy2();
		e->SetMapChipField(map);
		e->SetPosition(pos);
		e->Initialize();
		return e;
	});
}

EnemyManager::~EnemyManager()
{
	// Enemyの解放
	for (auto& enemy : enemies_) {
		if (enemy) {
			delete enemy; // Enemy自体を解放
		}
	}
	enemies_.clear();
}

void EnemyManager::RegisterFactory(int id, Factory factory)
{
	factories_[id] = std::move(factory);
}

void EnemyManager::Initialize(MapChipField* map) {

	map_ = map;
	// CSVから敵の位置を取得
	std::vector<Vector3> enemyPositions = map_->GetEnemyPositions();
	// 敵の番号を取得
	Enemynumber = map_->GetEnemyNumbers(); //敵の番号を取得

	// 配列長の不整合に備えて最小長でループ
	size_t count = std::min(enemyPositions.size(), Enemynumber.size());
	for (size_t i = 0; i < count; ++i) {
		const Vector3& enemyPos = enemyPositions[i];
		// 敵IDを取得
		int id = Enemynumber[i];
		// ファクトリを使って敵を生成
		auto it = factories_.find(id);
		if (it != factories_.end()) {
			// 敵生成
			EnemyBase* newEnemy = it->second(enemyPos, map_);
			if (newEnemy) enemies_.push_back(newEnemy);
		}
		else {
			assert(false && "未登録の敵IDです");
		}
	}
}

void EnemyManager::Update() {

	//敵の更新
	for (EnemyBase* enemy : enemies_) {
		if (enemy != nullptr) {
			enemy->Update();
		}
	}

	//死んだ敵を削除（演出完了で回収するフラグを見る）
	enemies_.remove_if([](EnemyBase* enemy) {
		if (enemy && enemy->IsPendingRemove()) {
			delete enemy;
			return true;
		}
		return false;
	});
}

void EnemyManager::Draw() {
	//Enemyの描画
	for (EnemyBase* enemy : enemies_) {
		if (enemy) {
			enemy->Draw();
		}
	}
}

void EnemyManager::EnemyObjectUpdate()
{
	for (EnemyBase* enemy : enemies_) {
		if (enemy) {
			if (auto* obj = enemy->GetObject3D()) {
				obj->Update();
			}
		}
	}
}

void EnemyManager::RegisterColliders()
{
	// CollisionManager シングルトンに自分の敵を登録する
	auto* cm = CollisionManager::GetInstance();
	for (EnemyBase* e : enemies_) {
		if (e) cm->AddCollider(e);
	}
}

