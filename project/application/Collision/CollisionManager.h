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
	// コンストラクタは private（外部から new できない）
	/// <summary>
	/// コンストラクタ
	/// </summary>
	CollisionManager() = default;
	/// <summary>
	/// デストラクタ
	/// </summary>
	~CollisionManager() = default;
	friend struct std::default_delete<CollisionManager>;
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	// 終了時に呼んでインスタンスを解放
	static void Finalize();

	/// <summary>
	/// コライダーを登録
	/// </summary>
	/// <param name="collider"></param>
	void AddCollider(Collider* collider);
	/// <summary>
	/// 登録されたコライダーを全てクリア
	/// </summary>
	void Clear();
	/// <summary>
	/// 当たり判定処理
	/// </summary>
	void Update();

  private:
	/// <summary>
	/// 当たり判定を取るべきか
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>n
	/// <returns></returns>
	bool CanCollide(Collider::Layer a, Collider::Layer b) const;
	std::vector<Collider*> colliders_;

	// シングルトン実体
	static std::unique_ptr<CollisionManager> instance_;
};
