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

	// 新規：コライダを CollisionManager に登録する
	void RegisterColliders();

	//Getter
	inline const std::list<Enemy*>& GetEnemies() const { return enemies_; }
	inline const std::list<Enemy2*>& GetEnemies2() const { return enemies2_; }

	void EnemyObjectUpdate();
private:
	std::vector<int> Enemynumber;
	std::list<Enemy*> enemies_;
	std::list<Enemy2*> enemies2_;
	MapChipField* map_ = nullptr;
	
};

