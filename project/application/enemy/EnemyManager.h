#pragma once
#include "Enemy.h"
#include "Enemy2.h"

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
	inline const std::list<Enemy*>& GetEnemies() const { return enemies_; }
	inline const std::list<Enemy2*>& GetEnemies2() const { return enemies2_; }
private:
	std::vector<int> Enemynumber;
	//Enemy
	std::list<Enemy*> enemies_;
	//Enemy2
	std::list<Enemy2*> enemies2_;

	MapChipField* map_ = nullptr;
	
};

