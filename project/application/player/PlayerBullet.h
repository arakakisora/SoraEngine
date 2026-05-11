#pragma once
#include "Model.h"
#include "Object3D.h"
#include <MapChipField.h>
#include "Collider.h"
#include <memory>

enum class WeaponType {
	Gatling,
	Cannon,
};

/// <summary>
/// プレイヤーの弾クラス
/// </summary>
class PlayerBullet : public Collider {

public:
	PlayerBullet() : Collider(Layer::PlayerBullet) {}
	~PlayerBullet() = default;  // unique_ptr により自動解放

	AABB GetAABB() const override { return aabb_; }
	AABB GetBulletAABB();

	/// <summary>
	/// 初期化（Object3D の所有権を受け取る）
	/// </summary>
	void Initialize(std::unique_ptr<Object3D> obj, const Vector3& potition, const Vector3& velocity, MapChipField* mapChipField);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	// 当たったときコールバック
	void OnCollision(Collider* other) override;
	/// </summary>

	//Getter
	/// <summary>
	/// 死亡しているかどうかを取得
	/// </summary>
	bool GetIsDead() const { return isDead_; }

	/// <summary>
	/// warld座標を取得します
	/// </summary>
	/// <returns></returns>
	Vector3 GetWorldPosition();

	/// <summary>
	/// AABBを取得します
	/// </summary>
	
	/// <summary>
	// レイを出してマップチップ番号を取得
	/// </summary>
	Vector3 GetRayEndPosition();
	/// <summary>
	// レイの先のマップチップ番号を取得
	/// </summary>
	int GetRayMapChipNumber(MapChipField* mapChipField);
	/// <summary>
	/// 弾の威力を取得します
	/// </summary>
	/// <returns></returns>
	int GetPower() const { return power_; }
	/// <summary>
	/// 弾の威力を設定します	
	/// </summary>
	void SetPower(int p) { power_ = p; }

	/// <summary>
	//プレイヤーの状態を取得
	/// </summary>
	WeaponType GetWeaponType() const { return weaponType_; }


	// Object3D の生ポインタ参照を返す（所有は PlayerBullet）
	Object3D* Getobject3DBullet_() { return object3D_.get(); }

	void SetWeaponType(WeaponType type) { weaponType_ = type; }

private:
    /// <summary>
	// ブロックに当たったかどうかをレイで判定
    /// </summary>
    /// <returns></returns>
	bool HitBlockSwept(const Vector3& from, const Vector3& to);

private:
	WeaponType weaponType_;
	AABB aabb_;
	std::unique_ptr<Object3D> object3D_; // 所有（unique_ptr）
	Vector3 prevPos_{};
	//textureHandle
	//uint32_t textureHandle_ = 0u;
	//速度
	Vector3 velocity_;
	//寿命
	static const int32_t kLifeTime = 60 * 5;
	int32_t deathTimer_ = kLifeTime;//デスタイマー
	bool isDead_ = false;			//デスフラグ
	MapChipField* mapChipField_ = nullptr;
	int power_ = 1; // 弾の威力

	// 定数化（マジックナンバー削減）
	//static inline constexpr float kAABBHalf = 0.1f; // AABB の半幅
	static inline constexpr float kRayLength = 0.5f; // レイ長さ
};
