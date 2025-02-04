#include "PlayerBullet.h"
#include "TextureManager.h"

void PlayerBullet::Initialize(Object3D* obj, const Vector3& potition, const Vector3& velocity, MapChipField* mapChipField) {

	// マップチップフィールドのポインタを保持

	mapChipField_ = mapChipField;
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

	// ★ レイキャストを実行し、マップチップ番号を取得
	if (GetRayMapChipNumber(mapChipField_) == 1) {
		OnCollison();  // 壁 (1) に当たったら削除
	}

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

Vector3 PlayerBullet::GetRayEndPosition() {
	// 弾の現在位置
	Vector3 currentPosition = GetWorldPosition();

	// レイの長さ
	float rayLength = 0.5f; // 弾が進む小さい距離でチェック

	// 移動方向を正規化
	Vector3 normalizedVelocity = velocity_;
	if (normalizedVelocity.Length() > 0) {
		normalizedVelocity.Normalize();
	}

	// レイの終点を計算
	Vector3 rayEnd = currentPosition + normalizedVelocity* rayLength;
	return rayEnd;
}

int PlayerBullet::GetRayMapChipNumber(MapChipField* mapChipField) {
	// レイの終点座標を取得
	Vector3 rayEndPosition = GetRayEndPosition();

	// マップチップのインデックスを取得
	IndexSet index = mapChipField->GetMapChipIndexSetByPosition(rayEndPosition);

	// マップチップの種類を取得
	MapChipType chipType = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	// マップチップ番号を返す
	return static_cast<int>(chipType);
}


