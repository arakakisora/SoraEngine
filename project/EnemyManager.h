#pragma once
#include "Enemy.h"
#include "Enemy2.h"
#include "EnemyBase.h"

class MapChipField;
class EnemyManager
{
public:
	~EnemyManager();
	//初期化
	void Initialize(MapChipField* map);
	//更新
	void Update();
	//描画
	void Draw();

	//Getter
	const std::vector<EnemyBase*>& GetEnemies() const { return enemies_; }
private:
	//std::vector<int> Enemynumber;
	////Enemy
	//std::list<Enemy*> enemies_;
	////Enemy2
	//std::list<Enemy2*> enemies2_;

	//MapChipField* map_ = nullptr;

	std::vector<EnemyBase*> enemies_;
	MapChipField* map_ = nullptr;
	
};

