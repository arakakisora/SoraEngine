#pragma once

class Player;
class EnemyManager;

/// <summary>
/// 当たり判定管理クラス
/// </summary>
class CollisionManager
{
public:
	/// <summary>
	//初期化
	/// </summary>
	///<param name="player"></param> 
	void Initialize(Player* player, EnemyManager* enemyValue);
	/// <summary>
	//更新
	/// </summary>
	void Update();
	
private:
	Player* player = nullptr;
	EnemyManager* enemy = nullptr;

};

