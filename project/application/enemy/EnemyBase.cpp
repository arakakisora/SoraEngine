#include "EnemyBase.h"

void EnemyBase::OnCollision(Collider* other)
{
	// 死亡中は当たり判定無視
	if (hitDeath_.IsDead()) return;


	if (other->GetLayer() == Layer::PlayerBullet) {
		//PlayerBullet* hitBullet = static_cast<PlayerBullet*>(other);
		// 赤くしてノックバックを渡す
		object3D_->SetColor({ 1, 0, 0, 1 });
		damageTimer_ = kDamageDisplayTime;

	
		// カメラに映る程度の動きに抑えるため控えめな値にする
		const float horizontalKnock = 1.0f; // 水平方向速度 (units/sec) - 調整可
		const float verticalKnock = 3.0f;   // 上方向初速度 (units/sec) - 調整可
		float dir = (velocity_.x > 0.0f) ? -1.0f : 1.0f; // 移動方向の逆（後方へ飛ばす）
		Vector3 knock = { dir * horizontalKnock, verticalKnock, 0.0f };

		// 内部モーションを使うようにしてノックバックを渡す
		hitDeath_.SetUseExternalDeathMotion(false);
		int hitPower = 1;
		hitDeath_.OnHit(hitPower, knock);
		// コンポーネント側で isDead_ を立てるので Update の次回で演出が始まる
	}

}

Vector3 EnemyBase::GetWorldPosition() {

	Vector3 worldPos;
	/// ワールド座標を取得
	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}


AABB EnemyBase::GetEnemyAABB()
{
	// エネミーのワールド座標を取得
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	// AABBの最小点と最大点を計算
	aabb.min = { worldPos.x - kEnemyWidth / 2.0f, worldPos.y - kEnemyHeight / 2.0f, worldPos.z - kEnemyWidth / 2.0f };
	aabb.max = { worldPos.x + kEnemyWidth / 2.0f, worldPos.y + kEnemyHeight / 2.0f, worldPos.z + kEnemyWidth / 2.0f };

	return aabb;
}

Vector3 EnemyBase::GetRayEndPosition()
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

int EnemyBase::GetRayMapChipNumber(MapChipField* mapChipField)
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