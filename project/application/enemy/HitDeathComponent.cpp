#define NOMINMAX 
#include "HitDeathComponent.h"
#include "Object3D.h"
#include "ParticleEmitter.h"
#include <algorithm>

void HitDeathComponent::Initialize(Object3D* obj, int initialHP, ParticleEmitter* emitter)
{
	hp_ = initialHP;
	emitter_ = emitter;
	if (obj) {
		defaultColor_ = obj->GetColor();
	}
	isDead_ = false;
	pendingRemove_ = false;
	damageTimer_ = 0.0f;
	deathTimer_ = 0.0f;
	emitted_ = false;
	useExternalDeathMotion_ = false;
	deathAngularVelocity_ = 0.0f;
	// deathVelocity_ は速度として初期化済み
}

void HitDeathComponent::OnHit(int damage, const Vector3& knockback)
{
	if (isDead_) return;
	hp_ -= damage;
	damageTimer_ = kDamageDisplayTime;

	// knockback を速度として受け取る（units/sec）
	deathVelocity_.x = knockback.x;
	deathVelocity_.y = knockback.y;
	deathVelocity_.z = knockback.z;

	// 横ノックバックの方向に応じて「後ろにひっくり返る」角速度を与える
	if (knockback.x > 0.0f) {
		// 右にノックバックなら背面方向に回転（符号はモデルの向きに合わせて調整）
		deathAngularVelocity_ = -kDefaultDeathAngularVelocity;
	}
	else if (knockback.x < 0.0f) {
		deathAngularVelocity_ = kDefaultDeathAngularVelocity;
	}
	else {
		deathAngularVelocity_ = kDefaultDeathAngularVelocity; // 横成分が無くても回転させたい場合のフォールバック
	}

	if (hp_ <= 0 && !isDead_) {
		isDead_ = true;
	}
}

void HitDeathComponent::StartDeath(Object3D* obj)
{
	// 初回のエフェクト発射など
	if (emitter_ && !emitted_) {
		emitter_->Emit();
		emitted_ = true;
	}
	// 内部モーションならここでその他初期処理を行う
}

void HitDeathComponent::Update(Object3D* obj, float dt)
{
	if (!obj) return;

	// ダメージ表示タイマー（色戻し）
	if (damageTimer_ > 0.0f) {
		damageTimer_ -= dt;
		if (damageTimer_ <= 0.0f) {
			obj->SetColor(defaultColor_);
		}
	}

	if (!isDead_) {
		return;
	}

	// 死亡開始処理（初回のみ）
	if (!emitted_) {
		StartDeath(obj);
	}

	// 外部モーションが有効なら、内部での移動・フェードは行わない。
	if (useExternalDeathMotion_) {
		// エフェクト位置だけ更新しておく
		if (emitter_) {
			emitter_->SetPosition(obj->GetTransform().translate);
		}
		// pendingRemove_ は外部が決めるためここでは立てない
		return;
	}

	// 以下は内部死亡モーション
	deathTimer_ += dt;

	// 重力は速度に対して減衰（units/sec^2）
	deathVelocity_.y -= deathGravity_ * dt;

	// 速度を積分して位置更新（pos += v * dt）
	auto pos = obj->GetTransform().translate;
	pos += deathVelocity_ * dt;
	obj->SetTranslate(pos);

	// 回転（倒れる）: 角速度を dt に応じて加算していく（ラジアン/秒）
	auto rot = obj->GetTransform().rotate;
	rot.x += deathAngularVelocity_ * dt;
	obj->SetRotate(rot);

	// フェードアウト
	float t = std::clamp(deathTimer_ / kDeathDuration, 0.0f, 1.0f);
	auto col = defaultColor_;
	col.w = std::max(0.0f, 1.0f - t);
	obj->SetColor(col);

	// エフェクト位置更新
	if (emitter_) {
		emitter_->SetPosition(obj->GetTransform().translate);
	}

	// 演出終了で削除予定（内部モーション時のみ）
	if (deathTimer_ >= kDeathDuration) {
		pendingRemove_ = true;
	}
}