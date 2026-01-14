// EnemyBase.h
#pragma once
#include "Object3D.h"
#include "MapChipField.h"
#include <ParticleEmitter.h>
#include "HitDeathComponent.h"
#include "Collider.h"


/// <summary>
/// EnemyBaseクラス
/// 敵の基底クラス
/// </summary>
class EnemyBase : public Collider {
public:
	EnemyBase() : Collider(Layer::Enemy) {}

	virtual ~EnemyBase() = default;
	/// <summary>
	//初期化
	//</summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;

	AABB GetAABB() const override { return aabb_; }

	void OnCollision(Collider* other) override;
	
	AABB GetEnemyAABB();

	void SetPosition(const Vector3& pos) {
		position_ = pos;
	}


	/// <summary>
	/// ワールド座標を取得します
	/// </summary>
	/// <returns>vector3、ワールドポジションを返す</returns>
	Vector3 GetWorldPosition();


	/// <summary>
	/// 目の前にブロックがあるかどうか
	/// </summary>
	/// <returns>エネミー目の前にレイを出す向きに応じて代わる</returns>
	Vector3 GetRayEndPosition();

	/// <summary>
	/// レイの先のマップチップ番号を取得
	/// </summary>
	/// <returns>レイに当たってるマップチップ番号</returns>
	int GetRayMapChipNumber(MapChipField* mapChipField);

	/// <summary>
	/// 指定タイル先（デフォルト1タイル）にあるチップの種類を返す
	/// </summary>
	int GetTileAheadType(MapChipField* map, int lookAheadTiles = 1);

	/// <summary>
	/// 指定タイル先が固い（壁）かどうかを返すヘルパ
	/// </summary>
	bool IsTileAheadSolid(MapChipField* map, int lookAheadTiles = 1);

	/// <summary>
	// Object3D解放用のメソッド
	/// </summary>
	void ReleaseObject3D() {
		delete object3D_;
		object3D_ = nullptr;
	}

	/// <summary>
	/// 死亡しているかどうか
	/// </summary>
	/// <returns></returns>
	bool IsDead() const { return isDead_; }

	/// <summary>
	/// マネージャが削除判定に使う（死亡演出完了フラグ）
	/// </summary>
	bool IsPendingRemove() const { return pendingRemove_; }
	/// <summary>
	/// マップチップフィールドセット
	/// </summary>
	/// <param name="mapChipField"></param>
	void SetMapChipField(MapChipField* mapChipField) {
		mapChipField_ = mapChipField;
	}

	Object3D* GetObject3D() { return object3D_; }
	

protected:
	Object3D* object3D_ = nullptr;
	Vector3 position_;
	Vector3 velocity_;
	bool isDead_ = false;
	float ground = 0.0f;
	float kEnemyWidth = 0.8f;
	float kEnemyHeight = 0.8f;
	int HP_ = 3;
	Vector4 defaultColor_ = { 1, 1, 1, 1 }; // 通常時の色
	float damageTimer_ = 0.0f;
	static inline const float kDamageDisplayTime = 0.2f; // 赤くなる時間（秒）
	//撃破effect
	ParticleEmitter* deatheEffect = nullptr; // パーティクルエミッター
	EulerTransform effectPosition_ = { {0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } };
	HitDeathComponent hitDeath_;
	bool pendingRemove_ = false; // マネージャ用
	AABB aabb_;
	MapChipField* mapChipField_ = nullptr;
	//death
	float rotateY = 0.0f;
};