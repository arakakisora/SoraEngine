#include "Enemy.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif 

#include "ParticleMnager.h"
#include "ChargeBehabiaor.h"
#include "Object3DCommon.h"




Enemy::~Enemy()
{
	
}

void Enemy::Initialize() {

	//Enemy
			// Object3Dの生成と初期化
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("enemy");
	//エネミーの初期位置
	object3D_->SetTranslate(position_);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });
	scale = 1.0f;
	object3D_->SetScale({ scale,scale,scale });
	object3D_->SetLighting(false);
	velocity_ = { -kWalkSpeed, 0, 0 }; // 速度
	walkTimer_ = 0.0f;
	rotateY = std::numbers::pi_v<float> / 2.0f;
	defaultColor_ = object3D_->GetColor(); // 初期色を保存


	//deatheffect
	//ParticleManager::GetInstance()->CreateParticleGroup("enemydeath", "Resources/honoo.png", VerticesType::Quad, std::make_unique<ExplosionBehavior>());
	lifeTime = 1.0f;//パーティクルの寿命
	currentTime = 1.0f;//現在の時間
	maxParticles = 100;//最大パーティクル数
	deatheEffect =std::make_unique< ParticleEmitter>(effectPosition_, lifeTime, currentTime, maxParticles, "enemydeath");
	// Hit/Death コンポーネント初期化（初期HP = 3）
	int HP = 3;
	hitDeath_.Initialize(object3D_.get(), HP, deatheEffect.get());

	aabb_ = GetEnemyAABB();
	
}	

void Enemy::Update() {
	 aabb_ = GetEnemyAABB();

	// 固定フレームとして dt を使う（現状のフレーム依存コードに合わせる）
	const float dt = 1.0f / 60.0f;

	// HitDeathComponent の更新を先に行う
	hitDeath_.Update(object3D_.get(), dt);

	// 死亡状態なら通常の動作はスキップして演出のみ表示する
	if (hitDeath_.IsDead()) {
		// 当たり判定を無効化してプレイヤーへの衝突を防ぐ
		SetCollisionEnabled(false);

		isDead_ = true;
		// 演出完了で回収フラグを立てる
		if (hitDeath_.IsPendingRemove()) {
			pendingRemove_ = true;
		}
		// object3D の GPU 更新（演出で位置・色を変更しているので反映は必要）
		object3D_->Update();
		// 死亡中はそれ以上の更新を行わない
		return;
	}

	// 通常の挙動（死亡していなければ実行）
	// 歩行タイマーの更新
	walkTimer_ += dt;
	// 歩行モーションの計算
	float param = std::sinf(std::numbers::pi_v<float> *2.0f * walkTimer_ / kWalkMotionTime);
	float radian = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	object3D_->SetRotate({ MyMath::fLerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, radian) ,rotateY ,0 });
	// 位置の更新（現在位置 + 速度を計算）
	Vector3 position = object3D_->GetTransform().translate;
	position += velocity_;

	// 移動を適用する前に「目の前のタイル」をチェックする（1 タイル先を既定）
	if (IsTileAheadSolid(mapChipField_, 0)) {
		// 壁がある → 進行方向を反転（移動は適用しない）
		velocity_.x *= -1.0f;

		// 回転方向も反転
		if (velocity_.x > 0) {
			rotateY = std::numbers::pi_v<float> / 2.0f;
		} else {
			rotateY = -std::numbers::pi_v<float> / 2.0f;
		}
	} else {
		// 通路なら位置を確定
		object3D_->SetTranslate(position);
	}

	// ダメージ表示タイマーの更新 （旧ロジックを残す：色戻し）
	if (damageTimer_ > 0.0f) {
		damageTimer_ -= dt;
		if (damageTimer_ <= 0.0f) {
			object3D_->SetColor(defaultColor_); // 元の色に戻す
		}
	}

	object3D_->Update();
	effectPosition_.translate = object3D_->GetTransform().translate;
	deatheEffect->SetPosition(effectPosition_.translate);
	

#ifdef _DEBUG
	
#endif // _DEBUG

}

void Enemy::Draw() {
	object3D_->Draw();


}

void Enemy::ReceiveDamage(int damage)
{



}






