#include "Enemy.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif 
#include "PlayerBullet.h"
#include "ParticleMnager.h"
#include "ChargeBehabiaor.h"




Enemy::~Enemy()
{
	if (object3D_) {
		delete object3D_;
		object3D_ = nullptr;
	}
}

void Enemy::Initialize(Object3D* obj, const Vector3& position) {



	object3D_ = obj;
	//エネミーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });
	object3D_->SetScale({ 1.0f,1.0f,1.0f });
	object3D_->SetLighting(false);
	velocity_ = { -kWalkSpeed, 0, 0 }; // 速度
	walkTimer_ = 0.0f;
	rotateY = std::numbers::pi_v<float> / 2.0f;
	defaultColor_ = object3D_->GetColor(); // 初期色を保存


	//deatheffect
	ParticleMnager::GetInstance()->CreateParticleGroup("enemydeath", "Resources/honoo.png", VerticesType::Quad, std::make_unique<ExplosionBehavior>());
	deatheEffect = new ParticleEmitter(effectPosition_, 1.0f, 1.0f, 100, "enemydeath");

	// Hit/Death コンポーネント初期化（初期HP = 3）
	hitDeath_.Initialize(object3D_, 3, deatheEffect);

	aabb_ = GetEnemyAABB();

}	

void Enemy::Update(MapChipField* mapChipField) {
	 aabb_ = GetEnemyAABB();

	// 固定フレームとして dt を使う（現状のフレーム依存コードに合わせる）
	const float dt = 1.0f / 60.0f;

	// HitDeathComponent の更新を先に行う
	hitDeath_.Update(object3D_, dt);

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
	// 位置の更新
	Vector3 position = object3D_->GetTransform().translate;
	position += velocity_;

	object3D_->SetTranslate(position);
	// レイの先のマップチップを取得
	int rayChipNumber = GetRayMapChipNumber(mapChipField);


	// レイの先にブロックがある場合、反転
	if (rayChipNumber == 1)
	{
		velocity_.x *= -1.0f; // 方向を反転

		// 回転方向も反転
		if (velocity_.x > 0) {
			//object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f, 0 });  // 右向き
			rotateY = std::numbers::pi_v<float> / 2.0f;
		}
		else {
			//object3D_->SetRotate({ 0, -std::numbers::pi_v<float> / 2.0f, 0 }); // 左向き
			rotateY = -std::numbers::pi_v<float> / 2.0f;
		}
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
	//deatheEffect->Update();

#ifdef _DEBUG
	ImGui::Text("HP: %d", HP);
#endif // _DEBUG

}

void Enemy::Draw() {
	object3D_->Draw();


}

Vector3 Enemy::GetWorldPosition() {

	Vector3 worldPos;
	/// ワールド座標を取得
	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}


void Enemy::OnCollision(Collider* other)
{
	// 死亡中は当たり判定無視
	if (hitDeath_.IsDead()) return;

	// --- 修正点: 引数の other を使う ---
	if (other->GetLayer() == Layer::PlayerBullet) {
		PlayerBullet* hitBullet = static_cast<PlayerBullet*>(other);
		// 赤くしてノックバックを渡す
		object3D_->SetColor({ 1, 0, 0, 1 });
		damageTimer_ = kDamageDisplayTime;

		// 修正: knockback を速度 (units/sec) として渡し、HitDeathComponent 側で dt を掛けて位置を積分する
		// カメラに映る程度の動きに抑えるため控えめな値にする
		const float horizontalKnock = 1.0f; // 水平方向速度 (units/sec) - 調整可
		const float verticalKnock = 3.0f;   // 上方向初速度 (units/sec) - 調整可
		float dir = (velocity_.x > 0.0f) ? -1.0f : 1.0f; // 移動方向の逆（後方へ飛ばす）
		Vector3 knock = { dir * horizontalKnock, verticalKnock, 0.0f };

		// 内部モーションを使うようにしてノックバックを渡す
		hitDeath_.SetUseExternalDeathMotion(false);
		hitDeath_.OnHit(hitBullet->GetPower(), knock);
		// コンポーネント側で isDead_ を立てるので Update の次回で演出が始まる
	}
}

AABB Enemy::GetEnemyAABB()
{
	// エネミーのワールド座標を取得
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	// AABBの最小点と最大点を計算
	aabb.min = { worldPos.x - kEnemyWidth / 2.0f, worldPos.y - kEnemyHeight / 2.0f, worldPos.z - kEnemyWidth / 2.0f };
	aabb.max = { worldPos.x + kEnemyWidth / 2.0f, worldPos.y + kEnemyHeight / 2.0f, worldPos.z + kEnemyWidth / 2.0f };

	return aabb;
}


Vector3 Enemy::GetRayEndPosition()
{
	// エネミーの現在位置
	Vector3 currentPosition = GetWorldPosition();

	// レイの長さ（3）
	float rayLength = 3.0f;

	// 移動方向を正規化してレイの終点を計算
	Vector3 normalizedVelocity = velocity_;
	if (normalizedVelocity.Length() > 0) {
		normalizedVelocity.Normalize();  // 速度を正規化
	}

	// 向きに応じたレイの終点座標を計算
	Vector3 rayEnd;
	rayEnd.x = currentPosition.x + rayLength * normalizedVelocity.x;
	rayEnd.y = currentPosition.y;
	rayEnd.z = currentPosition.z + rayLength * normalizedVelocity.z;

	return rayEnd;
}


int Enemy::GetRayMapChipNumber(MapChipField* mapChipField)
{
	// レイの終点座標を取得
	Vector3 rayEndPosition = GetRayEndPosition();

	// レイの終点があるマップチップのインデックスを取得
	IndexSet index = mapChipField->GetMapChipIndexSetByPosition(rayEndPosition);

	// マップチップの種類を取得
	int chipType = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	// マップチップ番号を返す
	return static_cast<int>(chipType);
}