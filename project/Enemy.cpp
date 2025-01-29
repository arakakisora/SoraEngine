#include "Enemy.h"
#include "imgui.h"


void Enemy::Initialize(Object3D* obj, const Vector3& position) {


	// texthureHandle_ = textureHandle;
	object3D_ = obj;
	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });
	velocity_ = { -kWalkSpeed, 0, 0 }; // 速度
	walkTimer_ = 0.0f;

	rotateY = std::numbers::pi_v<float> / 2.0f;

}

void Enemy::Update(MapChipField* mapChipField) {

	walkTimer_ += 1.0f / 60.0f;

	float param = std::sinf(std::numbers::pi_v<float> *2.0f * walkTimer_ / kWalkMotionTime);
	float radian = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	//worldTransform_.rotation_.x = fLerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, radian);
	object3D_->SetRotate({ MyMath::fLerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, radian) ,rotateY ,0});
	
	
	Vector3 position = object3D_->GetTransform().translate;
	position+= velocity_;

	object3D_->SetTranslate(position);

	// レイの先のマップチップを取得
	int rayChipNumber = GetRayMapChipNumber(mapChipField);
	ImGui::Text("rayChipNumber:%d", rayChipNumber);

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

	if (ImGui::CollapsingHeader("enemyModel", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Transform transform = object3D_->GetTransform();
		//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
		ImGui::DragFloat3("*ModelRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*ModelTransrate", &transform.translate.x, 0.01f);
		object3D_->SetTransform(transform);
	}

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


int Enemy::GetRayMapChipNumber( MapChipField* mapChipField)
{
	// レイの終点座標を取得
	Vector3 rayEndPosition = GetRayEndPosition();

	// レイの終点があるマップチップのインデックスを取得
	IndexSet index = mapChipField->GetMapChipIndexSetByPosition(rayEndPosition);

	// マップチップの種類を取得
	MapChipType chipType = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	// マップチップ番号を返す
	return static_cast<int>(chipType);
}



void Enemy::OnCollision(const PlayerBullet* bullet)
{
	if (bullet) {
		// 弾との衝突の場合
		isDead_ = true; // 敵を死亡させる
	}

}



