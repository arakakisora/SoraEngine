#include "EnemyManager.h"
#include "MapChipField.h"
#include "Object3DCommon.h"

EnemyManager::~EnemyManager()
{
	// Enemyの解放
	///enemy
	for (auto& enemy : enemies_) {
		if (enemy) {
			
			delete enemy; // Enemy自体を解放
		}
	}
	//enemy2
	for (auto& enemy2 : enemies2_) {
		if (enemy2) {
			
			delete enemy2; // Enemy2自体を解放
		}
	}

}

void EnemyManager::Initialize(MapChipField* map) {

	map_ = map;
	// CSVから敵の位置を取得
	std::vector<Vector3> enemyPositions = map_->GetEnemyPositions();
	// 敵の番号を取得
	Enemynumber = map_->GetEnemyNumbers(); //敵の番号を取得
	for (int i = 0; const Vector3 & enemyPos : enemyPositions) {

		if (Enemynumber[i] == 1) {
			//Enemy
			// Object3Dの生成と初期化
			Object3D* object3DEnemy = new Object3D();
			object3DEnemy->Initialize(Object3DCommon::GetInstance());
			object3DEnemy->SetModel("enemy.obj");
			// Enemyの生成と初期化
			Enemy* newEnemy = new Enemy();
			newEnemy->Initialize(object3DEnemy, enemyPos); 
			enemies_.push_back(newEnemy);

		}
		else if (Enemynumber[i] == 2) {
			//Enemy2]
			// Object3Dの生成と初期化
			Object3D* object3DEnemy2 = new Object3D();
			object3DEnemy2->Initialize(Object3DCommon::GetInstance());
			object3DEnemy2->SetModel("enemy.obj");
			// Enemy2の生成と初期化
			Enemy2* newEnemy2 = new Enemy2();
			newEnemy2->Initialize(object3DEnemy2, enemyPos); 
			enemies2_.push_back(newEnemy2);
		}


		i++;
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

void EnemyManager::EnemyObjectUpdate()
{

	
	for (Enemy* enemy : enemies_) {
		if (!nullptr) {
			enemy->GetObject3D()->Update();
		}
	}
	//Enemy2の描画
	for (Enemy2* enemy2 : enemies2_) {
		if (!nullptr) {
			enemy2->GetObject3D()->Update();
		}
	}

}

