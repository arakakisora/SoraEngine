#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <memory>

class Object3D;
class ParticleEmitter;

class HitDeathComponent {
public:
	
	void Initialize(Object3D* obj, int initialHP, ParticleEmitter* emitter);

	// 被弾処理（弾の攻撃力、ノックバックを与える方向）
	void OnHit(int damage, const Vector3& knockback = { 0.0f, 0.0f, 0.0f });

	// 毎フレーム更新（Object3D に対する見た目変更等を行う）
	void Update(Object3D* obj, float dt);

	bool IsDead() const { return isDead_; }
	bool IsPendingRemove() const { return pendingRemove_; }

	// 外部で死亡モーションを担当する場合は true にする（内部での移動/回転/フェードは行わない）
	void SetUseExternalDeathMotion(bool v) { useExternalDeathMotion_ = v; }

private:
	// 内部ヘルパー（死亡開始時の一度だけ処理）
	void StartDeath(Object3D* obj);

	int hp_ = 0;
	bool isDead_ = false;
	bool pendingRemove_ = false;

	// ダメージ表示
	float damageTimer_ = 0.0f;
	static inline const float kDamageDisplayTime = 0.2f;

	// 死亡演出（内部モード用）
	float deathTimer_ = 0.0f;
	static inline const float kDeathDuration = 1.0f;
	// deathVelocity_ は速度 (units/sec) として扱う (Update 内で dt を掛けて積分する)
	Vector3 deathVelocity_ = { 0.0f, 0.0f, 0.0f };
	// 重力 (units/sec^2)
	static inline constexpr float kDeathGravity = 9.8f;

	// 回転（角速度：ラジアン/秒程度の想定）
	float deathAngularVelocity_ = 0.0f;
	static inline const float kDefaultDeathAngularVelocity = 6.0f; // 見た目調整用

	// 色保持
	Vector4 defaultColor_ = { 1.0f,1.0f,1.0f,1.0f };

	ParticleEmitter * emitter_ = nullptr;
	bool emitted_ = false;
			
	// 外部モーションを使うか（外部が移動/回転/フェードを担当する）
	bool useExternalDeathMotion_ = false;
};