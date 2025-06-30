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
	// CSVから敵の位置を取得
	std::vector<Vector3> enemyPositions = map_->GetEnemyPositions();
	for (const Vector3& enemyPos : enemyPositions) {
		Object3D* object3DEnemy = new Object3D();
		object3DEnemy->Initialize(Object3DCommon::GetInstance());
		object3DEnemy->SetModel("enemy.obj");

		Enemy* newEnemy = new Enemy();
		newEnemy->Initialize(object3DEnemy, enemyPos); // ← これだけでOK

		enemies_.push_back(newEnemy);
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


}

void EnemyManager::Draw(){
	//Enemyの描画

	for (Enemy* enemy : enemies_) {
		if (!nullptr) {
			enemy->Draw();
		}
	}
}

