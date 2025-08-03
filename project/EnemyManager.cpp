#include "EnemyManager.h"
#include "MapChipField.h"
#include "Object3DCommon.h"

EnemyManager::~EnemyManager()
{
	for (auto enemy : enemies_) {
		
		delete enemy;
	}
	enemies_.clear();

}

void EnemyManager::Initialize(MapChipField* map) {

	map_ = map;

	// 敵の位置・タイプを取得
	std::vector<Vector3> enemyPositions = map_->GetEnemyPositions();
	std::vector<int> enemyTypes = map_->GetEnemyNumbers(); // 1 = Enemy, 2 = Enemy2

	for (int i = 0; i < enemyPositions.size(); ++i) {
		const Vector3& pos = enemyPositions[i];
		int type = enemyTypes[i];

		Object3D* obj = new Object3D();
		obj->Initialize(Object3DCommon::GetInstance());

		// モデルの切り替え（必要に応じて）
		if (type == 1) {
			obj->SetModel("enemy.obj");
		}
		else if (type == 2) {
			obj->SetModel("enemy2.obj");
		}

		EnemyBase* newEnemy = nullptr;
		if (type == 1) {
			newEnemy = new Enemy();
		}
		else if (type == 2) {
			newEnemy = new Enemy2();
		}

		if (newEnemy) {
			newEnemy->Initialize(obj, pos);
			enemies_.push_back(newEnemy);
		}
	}

}

void EnemyManager::Update() {


	for (auto it = enemies_.begin(); it != enemies_.end(); ) {
		EnemyBase* enemy = *it;
		enemy->Update(map_);
		if (enemy->IsDead()) {
			delete enemy;
			it = enemies_.erase(it);
		}
		else {
			++it;
		}
	}
}


void EnemyManager::Draw() {
	//Enemyの描画

	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

