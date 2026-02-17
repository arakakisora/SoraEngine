#define NOMINMAX 

#include "PlayerBullet.h"
#include "TextureManager.h"
#include "MapChipField.h" // 追加
#include <unordered_set>


AABB PlayerBullet::GetBulletAABB()
{
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - object3D_->GetTransform().scale.x/2.0f, worldPos.y - object3D_->GetTransform().scale.y/2.0f, worldPos.z - object3D_->GetTransform().scale.z/2.0f};
	aabb.max = { worldPos.x + object3D_->GetTransform().scale.x/2.0f, worldPos.y + object3D_->GetTransform().scale.y/2.0f, worldPos.z + object3D_->GetTransform().scale.z/2.0f};
	return aabb;
}

void PlayerBullet::Initialize(std::unique_ptr<Object3D> obj, const Vector3& potition, const Vector3& velocity, MapChipField* mapChipField) {

	// 所有権を受け取る
	object3D_ = std::move(obj);

	Vector3 pos = potition;
	pos.y += 0.5f;

	// プレイヤーの初期位置
	object3D_->SetTranslate(pos);

	// 速度
	velocity_ = velocity;

	mapChipField_ = mapChipField;

	prevPos_ = object3D_->GetTransform().translate;
}

void PlayerBullet::Update() {

	prevPos_ = object3D_->GetTransform().translate;

	Vector3 position = prevPos_;
	position += velocity_;
	object3D_->SetTranslate(position);

	if (HitBlockSwept(prevPos_, position)) {
		isDead_ = true;
	}

	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	object3D_->Update();
	aabb_ = GetBulletAABB();
}

void PlayerBullet::Draw() { object3D_->Draw(); }

void PlayerBullet::OnCollision(Collider* other)
{
	if (other->GetLayer() == Layer::Enemy ||
		other->GetLayer() == Layer::Enemy2 ||
		other->GetLayer() == Layer::EnemyBullet) {
		isDead_ = true;   // 消えるフラグ
	}

}

Vector3 PlayerBullet::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}

Vector3 PlayerBullet::GetRayEndPosition() {

	// 弾の現在位置
	Vector3 currentPosition = GetWorldPosition();

	// 移動方向を正規化
	Vector3 normalizedVelocity = velocity_;
	if (normalizedVelocity.Length() > 0) {
		normalizedVelocity = normalizedVelocity.Normalize();
	}

	// レイの終点を計算
	Vector3 rayEnd = currentPosition + normalizedVelocity * kRayLength;
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

bool PlayerBullet::HitBlockSwept(const Vector3& from, const Vector3& to)
{
	if (!mapChipField_) return false;

	// 半径（ワールド単位に合わせる）
	// scale/2 はワールド半径じゃないので、係数で合わせるのが安全
	constexpr float kRadiusPerScale = 0.25f; // ここ調整（0.2〜0.4）
	const float r = object3D_->GetTransform().scale.x * kRadiusPerScale;

	// 移動距離に応じてサンプル数を増やす（境界拾い漏れ防止）
	Vector3 d = to - from;
	float dist = d.Length();
	int steps = std::max(1, (int)std::ceil(dist / (r * 0.5f))); // 半径の半分刻み
	Vector3 step = d * (1.0f / float(steps));

	bool hitAny = false;

	// 触れたタイルを重複なく処理したいなら set でまとめてもOK
	for (int i = 0; i <= steps; ++i) {
		Vector3 center = from + step * float(i);

		// 半径ぶんのタイル範囲
		IndexSet minIdx = mapChipField_->GetMapChipIndexSetByPosition(center + Vector3{ -r, -r, 0 });
		IndexSet maxIdx = mapChipField_->GetMapChipIndexSetByPosition(center + Vector3{ +r, +r, 0 });

		int x0 = std::min(minIdx.xIndex, maxIdx.xIndex);
		int x1 = std::max(minIdx.xIndex, maxIdx.xIndex);
		int y0 = std::min(minIdx.yIndex, maxIdx.yIndex);
		int y1 = std::max(minIdx.yIndex, maxIdx.yIndex);

		for (int y = y0; y <= y1; ++y) {
			for (int x = x0; x <= x1; ++x) {

				MapChipType chip = mapChipField_->GetMapChipTypeByIndex(x, y);
				if (chip != MapChipType::Block && chip != MapChipType::UnbreakableBlock) continue;

				Rect rect = mapChipField_->GetRectByIndex(x, y);

				// 円 vs 矩形（触れてるか）
				float cx = std::clamp(center.x, rect.left, rect.right);
				float cy = std::clamp(center.y, rect.bottom, rect.top);

				float dx = center.x - cx;
				float dy = center.y - cy;

				if (dx * dx + dy * dy <= r * r) {
					int hp = mapChipField_->GetMapChipHPByIndex(x, y);
					if (hp > 0) {
						mapChipField_->DamageMapChipByIndex(x, y, power_);
					}
					hitAny = true;
				}
			}
		}
	}

	return hitAny;
}


