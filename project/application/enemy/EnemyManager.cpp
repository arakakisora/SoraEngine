#include "EnemyManager.h"
#include "MapChipField.h"
#include "Object3DCommon.h"

EnemyManager::~EnemyManager()
{
	for (auto& enemy : enemies_) {
		if (enemy) {
			//enemy->ReleaseObject3D();
			delete enemy; // Enemy自体を解放
		}
	}

}

void EnemyManager::Initialize(MapChipField* map) {

	map_ = map;

	// マップから敵タイプごとの全座標を取る
	auto posE1 = map_->GetPositionsByType(MapChipType::kEnemy);   // 敵1の全座標:contentReference[oaicite:3]{index=3}
	auto posE2 = map_->GetPositionsByType(MapChipType::kEnemy2);  // 敵2の全座標:contentReference[oaicite:4]{index=4}
	std::vector<std::vector<int>> enemynum = map_->GetAllNumbers();

		for (const auto& pos : positions) {
			// 各個体の Object3D を作る
			auto* obj = new Object3D();
			obj->Initialize(Object3DCommon::GetInstance());
			obj->SetModel("enemy.obj");

			EnemyBase* e = nullptr;
			if (t == MapChipType::kEnemy)      e = new Enemy();
			else if (t == MapChipType::kEnemy2) e = new Enemy2();

			if (e) {
				// Initialize は (Object3D*, pos) が必須:contentReference[oaicite:5]{index=5}:contentReference[oaicite:6]{index=6}
				e->Initialize(obj, pos);
				enemies_.push_back(e);
			} else {
				delete obj; // 生成失敗時のリーク防止
			}
		}

}

void EnemyManager::Update() {


	//敵の更新
	for (Enemy* enemy : enemies_) {

		if (enemy != nullptr) {
			enemy->Update(map_);
		}
	}

	//死んだ敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
		});

	//敵2の更新
	for (Enemy2* enemy2 : enemies2_) {
		if (enemy2 != nullptr) {
			enemy2->Update(map_);
		}
	}
	//死んだ敵2を削除
	enemies2_.remove_if([](Enemy2* enemy2) {
		if (enemy2->IsDead()) {
			delete enemy2;
			return true;
		}
		return false;
		});
}

void EnemyManager::Draw() {
	//Enemyの描画

	for (Enemy* enemy : enemies_) {
		if (!nullptr) {
			enemy->Draw();
		}
	}
	//Enemy2の描画
	for (Enemy2* enemy2 : enemies2_) {
		if (!nullptr) {
			enemy2->Draw();
		}
	}
}

