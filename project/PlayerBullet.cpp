#include "PlayerBullet.h"
#include "TextureManager.h"

void PlayerBullet::Initialize(Object3D* obj, const Vector3& potition, const Vector3& velocity) {

	// ポインタ参照
	object3D_ = obj;

	Vector3 pos = potition;
	pos.y += 0.5f;

	// プレイヤーの初期位置
	object3D_->SetTranslate(pos);
	

	// 速度
	velocity_ = velocity;
}

void PlayerBullet::Update() {

	Vector3 position = object3D_->GetTransform().translate;
	position += velocity_;
	object3D_->SetTranslate(position);

	

	if (--deathTimer_ <= 0) {

		isDead_ = true;
	}

	object3D_->Update();
}

void PlayerBullet::Draw() { object3D_->Draw(); }

void PlayerBullet::OnCollison() { isDead_ = true; }

Vector3 PlayerBullet::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}

AABB PlayerBullet::GetAABB()
{
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - 0.1f, worldPos.y - 0.1f, worldPos.z - 0.1f };
	aabb.max = { worldPos.x + 0.1f, worldPos.y + 0.1f, worldPos.z + 0.1f };
	return aabb;
}
