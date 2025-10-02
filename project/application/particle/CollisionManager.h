#pragma once

class Player;
class EnemyManager;
class CollisionManager
{
public:

	//初期化
	void Initialize(Player* player, EnemyManager* enemy);
	//更新
	void Update();
	
private:
	Player* player = nullptr;
	EnemyManager* enemy = nullptr;

};

