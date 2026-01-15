#define NOMINMAX 
#include "HitDeathComponent.h"
#include "Object3D.h"
#include "ParticleEmitter.h"
#include <algorithm>

// 初期化：HP と（オプションの）エミッタを受け取る。
// emitter は所有権を受け取る（nullptr 可）
void HitDeathComponent::Initialize(Object3D* obj, int initialHP, ParticleEmitter* emitter)
{
	hp_ = initialHP;
	// 所有権を受け取る（nullptr の場合はエフェクトなし）
	emitter_ = emitter;

	// オブジェクトの初期色を保存
	if (obj) {
		defaultColor_ = obj->GetColor();
	}
	// フラグ初期化
	isDead_ = false;
	pendingRemove_ = false;
	damageTimer_ = 0.0f;
	deathTimer_ = 0.0f;
	emitted_ = false;
	useExternalDeathMotion_ = false;
	deathAngularVelocity_ = 0.0f;
	// deathVelocity_ は既にデフォルト初期化されている
}

// 被弾処理：HP を減らし、ダメージ表示・ノックバック・角速度を設定する
void HitDeathComponent::OnHit(int damage, const Vector3& knockback)
{
	// 既に死亡済みなら無視
	if (isDead_) return;

	// HP 減少とダメージ表示タイマーセット
	hp_ -= damage;
	damageTimer_ = kDamageDisplayTime;

	// ノックバックを速度として受け取る（units/sec）
	deathVelocity_.x = knockback.x;
	deathVelocity_.y = knockback.y;
	deathVelocity_.z = knockback.z;

	// 横ノックバックの方向に応じて「後ろにひっくり返る」角速度を与える
	if (knockback.x > 0.0f) {
		// 右にノックバックなら背面方向に回転（符号はモデルに合わせて調整）
		deathAngularVelocity_ = -kDefaultDeathAngularVelocity;
	}
	else if (knockback.x < 0.0f) {
		deathAngularVelocity_ = kDefaultDeathAngularVelocity;
	}
	else {
		// 横成分が無ければフォールバックで少し回す
		deathAngularVelocity_ = kDefaultDeathAngularVelocity;
	}

	// HP が尽きたら死亡フラグを立てる（Update で演出開始）
	if (hp_ <= 0 && !isDead_) {
		isDead_ = true;
	}
}

// 死亡開始時の一度だけ処理（エフェクト発射等）
void HitDeathComponent::StartDeath(Object3D* obj)
{
	if (emitter_ && !emitted_) {
		emitter_->Emit();
		emitted_ = true;
	}
	// 内部モーションの場合、ここで追加初期化（必要なら実装）
}

// 毎フレーム処理：色戻し、内部死亡モーション（非外部モード）を行う
void HitDeathComponent::Update(Object3D* obj, float dt)
{
	if (!obj) return;

	// ダメージ表示タイマー（色を元に戻す）
	if (damageTimer_ > 0.0f) {
		damageTimer_ -= dt;
		if (damageTimer_ <= 0.0f) {
			obj->SetColor(defaultColor_);
		}
	}

	// 死亡前はここまで
	if (!isDead_) {
		return;
	}

	// 死亡開始処理（初回のみ）
	if (!emitted_) {
		StartDeath(obj);
	}

	// 外部モーションが有効な場合は内部で移動/フェードは行わない（エフェクト位置だけ更新）
	if (useExternalDeathMotion_) {
		if (emitter_) {
			emitter_->SetPosition(obj->GetTransform().translate);
		}
		// pendingRemove_ は外部が管理するためここでは設定しない
		return;
	}

	// --- 以下、内部死亡モーション ---
	deathTimer_ += dt;

	// 重力を適用（速度の y 成分に減衰）
	deathVelocity_.y -= kDeathGravity * dt;

	// 速度を積分して位置を更新（pos += v * dt）
	auto pos = obj->GetTransform().translate;
	pos += deathVelocity_ * dt;
	obj->SetTranslate(pos);

	// 回転（倒れる）：角速度を加算
	auto rot = obj->GetTransform().rotate;
	rot.x += deathAngularVelocity_ * dt;
	obj->SetRotate(rot);

	// フェードアウト：経過比 t を計算してアルファを下げる
	float t = std::clamp(deathTimer_ / kDeathDuration, 0.0f, 1.0f);
	auto col = defaultColor_;
	col.w = std::max(0.0f, 1.0f - t);
	obj->SetColor(col);

	// エフェクト位置をオブジェクトに追従させる
	if (emitter_) {
		emitter_->SetPosition(obj->GetTransform().translate);
	}

	// 演出終了で削除予定フラグを立てる
	if (deathTimer_ >= kDeathDuration) {
		pendingRemove_ = true;
	}
}