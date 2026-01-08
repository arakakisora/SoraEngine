#pragma once
#include "Enemy2.h"
#include "EnemyBase.h"
#include <functional>
#include <unordered_map>

struct Vector3;
class MapChipField;
/// <summary>
/// EnemyManagerクラス
/// </summary>
class EnemyManager
{
public:
	EnemyManager();
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

	//コライダを CollisionManager に登録する
	void RegisterColliders();

	//Getter
	inline const std::list<EnemyBase*>& GetEnemies() const { return enemies_; }
	

	void EnemyObjectUpdate();

	// 敵ファクトリ登録（id -> factory）
	using Factory = std::function<EnemyBase*(const Vector3& pos, MapChipField* map)>;
	//登録関数
	void RegisterFactory(int id, Factory factory);

private:
	std::vector<int> Enemynumber;
	std::list<EnemyBase*> enemies_;
	MapChipField* map_ = nullptr;

	std::unordered_map<int, Factory> factories_;
};

