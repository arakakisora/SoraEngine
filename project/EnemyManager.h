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
	inline const std::list<EnemyBase*>& GetEnemies() const { return enemies_; }
	
private:
	std::list<EnemyBase*> enemies_;
	MapChipField* map_ = nullptr;
	
};

