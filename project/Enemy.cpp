#include "Enemy.h"


void Enemy::Initialize(Object3D* obj, const Vector3& position) {


	// texthureHandle_ = textureHandle;
	object3D_ = obj;
	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });
	velocity_ = { -kWalkSpeed, 0, 0 }; // 速度
	walkTimer_ = 0.0f;

}

void Enemy::Update() {

	walkTimer_ += 1.0f / 60.0f;

	float param = std::sinf(std::numbers::pi_v<float> *2.0f * walkTimer_ / kWalkMotionTime);
	float radian = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	//worldTransform_.rotation_.x = fLerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, radian);
	object3D_->SetRotate({ MyMath::fLerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, radian) ,std::numbers::pi_v<float> / 2.0f ,0});
	
	Vector3 position = object3D_->GetTransform().translate;
	position+= velocity_;

	object3D_->SetTranslate(position);

	object3D_->Update();
}

void Enemy::Draw() { object3D_->Draw(); }

Vector3 Enemy::GetWorldPosition() {

	Vector3 worldPos;

	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - kEnemyWidth / 2.0f, worldPos.y - kEnemyHeight / 2.0f, worldPos.z - kEnemyWidth / 2.0f };
	aabb.max = { worldPos.x + kEnemyWidth / 2.0f, worldPos.y + kEnemyHeight / 2.0f, worldPos.z + kEnemyWidth / 2.0f };

	return aabb;
}

void Enemy::OnCollision(const Player* player) {

	(void)player;


}

void Enemy::OnCollision(const PlayerBullet* bullet)
{
	if (bullet) {
		// 弾との衝突の場合
		isDead_ = true; // 敵を死亡させる
	}

}
