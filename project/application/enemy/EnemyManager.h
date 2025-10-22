#pragma once
#include "Enemy.h"
#include "Enemy2.h"

class MapChipField;
/// <summary>
/// EnemyManagerクラス
/// </summary>
class EnemyManager
{
public:
	~EnemyManager();
	/// <summary>
	//初期化
	/// </summary>
	void Initialize(MapChipField* map);
	/// <summary>
	//更新
	/// </summary>
	void Update();
	/// <summary>
	//描画
	/// </summary>
	void Draw();

	//Getter
	/// <summary>
	//Enemyのリストを取得
	/// </summary>
	inline const std::list<Enemy*>& GetEnemies() const { return enemies_; }
	/// <summary>
	///Enemy2のリストを取得
	/// </summary>
	/// <returns></returns>
	inline const std::list<Enemy2*>& GetEnemies2() const { return enemies2_; }
private:
	std::vector<int> Enemynumber;
	//Enemy
	std::list<Enemy*> enemies_;
	//Enemy2
	std::list<Enemy2*> enemies2_;

	MapChipField* map_ = nullptr;
	
};

