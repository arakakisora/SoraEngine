#pragma once
#include "Enemy2.h"
#include "EnemyBase.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include <list>
#include <vector>

struct Vector3;
class MapChipField;
/// <summary>
/// EnemyManagerクラス
/// </summary>
class EnemyManager
{
public:
	EnemyManager();
	~EnemyManager() = default; 

	/// <summary>
	// 初期化（マップ参照を保持し、CSV から敵を生成）
	/// </summary>
	void Initialize(MapChipField* map);
	/// <summary>
	// 更新（敵の Update と生存チェック）
	/// </summary>
	void Update();
	/// <summary>
	// 描画
	/// </summary>
	void Draw();

	// コライダを CollisionManager に登録する
	void RegisterColliders();

	// Getter: 敵一覧（所有権はこのクラスが持つ）
	inline const std::list<std::unique_ptr<EnemyBase>>& GetEnemies() const { return enemies_; }

	void EnemyObjectUpdate();

	// 敵ファクトリ登録（id -> factory）
	// Factory は生成済みの std::unique_ptr を返す
	using Factory = std::function<std::unique_ptr<EnemyBase>(const Vector3& pos, MapChipField* map)>;
	// 登録関数
	void RegisterFactory(int id, Factory factory);

private:
	
	static inline constexpr int kFactoryIdDefaultEnemy = 1;
	static inline constexpr int kFactoryIdEnemy2 = 2;

	// マップから読み取った敵タイプ列（CSV由来）
	std::vector<int> Enemynumber;

	// 敵の所有コンテナ（unique_ptr で所有）
	std::list<std::unique_ptr<EnemyBase>> enemies_;
	MapChipField* map_ = nullptr;

	// id -> factory マップ
	std::unordered_map<int, Factory> factories_;
};

