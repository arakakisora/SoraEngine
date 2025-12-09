#pragma once
#include "Collider.h"	
#include <vector>

/// <summary>
/// 当たり判定管理クラス（シングルトン）
/// </summary>
class CollisionManager
{
public:
	// シングルトン取得
	static CollisionManager* GetInstance();
	// 終了時に呼んでインスタンスを解放
	static void Finalize();

	void AddCollider(Collider* collider);

	void Clear();

	void Update();

private:
	// コンストラクタは private（外部から new できない）
	CollisionManager() = default;
	~CollisionManager() = default;
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	bool CanCollide(Collider::Layer a, Collider::Layer b) const ;
	std::vector<Collider*> colliders_;

	// シングルトン実体
	static CollisionManager* instance_;
};

