#include "EnemyBase.h"

void EnemyBase::OnCollision(Collider* other)
{
	// 死亡中は当たり判定無視
	if (hitDeath_.IsDead()) return;


	if (other->GetLayer() == Layer::PlayerBullet) {
		//PlayerBullet* hitBullet = static_cast<PlayerBullet*>(other);
		// 赤くしてノックバックを渡す
		object3D_->SetColor(kDamageColor);
		damageTimer_ = kDamageDisplayTime;

	
		// カメラに映る程度の動きに抑えるため控えめな値にする
		const float horizontalKnock = kHorizontalKnock; // 水平方向速度 (units/sec)
		const float verticalKnock = kVerticalKnock;   // 上方向初速度 (units/sec)
		float dir = (velocity_.x > 0.0f) ? -1.0f : 1.0f; // 移動方向の逆（後方へ飛ばす）
		Vector3 knock = { dir * horizontalKnock, verticalKnock, 0.0f };

		// 内部モーションを使うようにしてノックバックを渡す
		hitDeath_.SetUseExternalDeathMotion(false);
		int hitPower = kDefaultHitPower;
		hitDeath_.OnHit(hitPower, knock);
		// コンポーネント側で isDead_ を立てるので Update の次回で演出が始まる
	}

}

Vector3 EnemyBase::GetWorldPosition() {

	Vector3 worldPos;
	/// ワールド座標を取得
	worldPos.x = object3D_->GetWorldMatrix().m[3][0];
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];
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

	// レイの長さ
	const float rayLength = kRayLength;

	// 移動方向を正規化してレイの終点を計算
	Vector3 normalizedVelocity = velocity_;
	if (normalizedVelocity.Length() > 0.0f) {
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
	MapChipType chipType = mapChipField->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	// マップチップ番号を返す
	return static_cast<int>(chipType);
}

// 指定タイル先（デフォルト1タイル）にあるチップの種類を返す
MapChipType EnemyBase::GetTileAheadType(MapChipField* map, int lookAheadTiles /*= 1*/)
{
	// 現在位置（SetTranslate直後でも反映されるTransform位置を使用）
	Vector3 pos = object3D_->GetTransform().translate;

	// 進行方向（X軸左右移動前提）。速度優先、無ければ rotateY で決定
	int dir = 0;
	if (velocity_.x > kVelocityEpsilon) dir = 1;
	else if (velocity_.x < -kVelocityEpsilon) dir = -1;
	else dir = (rotateY >= 0.0f) ? 1 : -1;

	// チェックするワールド座標（エネミー前端＋タイル数分）
	float offset = (kEnemyWidth * 0.5f) + (lookAheadTiles * map->GetBlockWidth());
	Vector3 checkPos = pos;
	checkPos.x += dir * offset;

	// マップのインデックスを取得して範囲内にクランプ
	IndexSet idx = map->GetMapChipIndexSetByPosition(checkPos);
	int x = static_cast<int>(idx.xIndex);
	int y = static_cast<int>(idx.yIndex);
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= static_cast<int>(map->GetNumBlockHorizontal())) x = static_cast<int>(map->GetNumBlockHorizontal()) - 1;
	if (y >= static_cast<int>(map->GetNumBlockVirtical())) y = static_cast<int>(map->GetNumBlockVirtical()) - 1;

	return map->GetMapChipTypeByIndex(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
}

// 指定タイル先が固い（壁）かどうかを返すヘルパ
bool EnemyBase::IsTileAheadSolid(MapChipField* map, int lookAheadTiles /*= 1*/)
{
	MapChipType type = GetTileAheadType(map, lookAheadTiles);
	// Map の仕様により「1 がブロック」な既存コードに合わせる（必要なら修正）
	IndexSet aheadIndex = map->GetMapChipIndexSetByPosition(
		Vector3{ object3D_->GetTransform().translate.x + ((velocity_.x > 0) ? 1.0f : -1.0f) * ((kEnemyWidth * 0.5f) + lookAheadTiles * map->GetBlockWidth()),
				 object3D_->GetTransform().translate.y,
				 object3D_->GetTransform().translate.z });
	return map->IsSolid(aheadIndex.xIndex, aheadIndex.yIndex);
}