#pragma once
#include "Enemy.h"

class MapChipField;
class EnemyManager
{
public:
	//初期化
	void Initialize(MapChipField* map);
	//解放
	void Finalize();
	//更新
	void Update();
	//描画
	void Draw();

	//Getter
	inline const std::list<Enemy*>& GetEnemies() const { return enemies_; }
private:
	//Enemy
	std::list<Enemy*> enemies_;
	MapChipField* map_ = nullptr;
	
};

