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
	int HP = 3;
	hitDeath_.Initialize(object3D_, HP, deatheEffect);

	aabb_ = GetEnemyAABB();
	line = std::make_unique<Line>();
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
	// 位置の更新（現在位置 + 速度を計算）
	Vector3 position = object3D_->GetTransform().translate;
	position += velocity_;

	// 移動を適用する前に「目の前のタイル」をチェックする（1 タイル先を既定）
	if (IsTileAheadSolid(mapChipField, 0)) {
		// 壁がある → 進行方向を反転（移動は適用しない）
		velocity_.x *= -1.0f;

		// 回転方向も反転
		if (velocity_.x > 0) {
			rotateY = std::numbers::pi_v<float> / 2.0f;
		}
		else {
			rotateY = -std::numbers::pi_v<float> / 2.0f;
		}
	}
	else {
		// 通路なら位置を確定
		object3D_->SetTranslate(position);
	}
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
	// object3D の現在設定されているトランスフォーム位置を使う（SetTranslate の直後でも反映される）
	Vector3 currentPosition = object3D_->GetTransform().translate;

	// レイの長さ（必要なら MapChipField::kBlockWidth などに合わせて調整）
	const float rayLength = 3.0f;

	// 方向ベクトル：速度ベース、速度がほぼ0なら rotateY から作成
	Vector3 direction = velocity_;
	if (direction.Length() > 1e-6f) {
		direction.Normalize();
	}
	else {
		direction.x = std::cos(rotateY);
		direction.y = 0.0f;
		direction.z = std::sin(rotateY);
		// 念のため正規化
		if (direction.Length() > 1e-6f) direction.Normalize();
	}

	// レイ終点を計算（高さは currentPosition.y を使用）
	Vector3 rayEnd;
	rayEnd.x = currentPosition.x + direction.x * rayLength;
	rayEnd.y = currentPosition.y;
	rayEnd.z = currentPosition.z + direction.z * rayLength;

	// デバッグライン表示
	line->Draw(currentPosition, rayEnd, { 1.0f, 0.0f, 0.0f, 1.0f });

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

// 指定タイル先（デフォルト1タイル）にあるチップの種類を返す
int Enemy::GetTileAheadType(MapChipField* map, int lookAheadTiles /*= 1*/)
{
    // 現在位置（SetTranslate直後でも反映されるTransform位置を使用）
    Vector3 pos = object3D_->GetTransform().translate;

    // 進行方向（X軸左右移動前提）。速度優先、無ければ rotateY で決定
    int dir = 0;
    if (velocity_.x > 1e-6f) dir = 1;
    else if (velocity_.x < -1e-6f) dir = -1;
    else dir = (rotateY >= 0.0f) ? 1 : -1;

    // チェックするワールド座標（エネミー前端＋タイル数分）
    float offset = (kEnemyWidth * 0.5f) + (lookAheadTiles * 1.0f);
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
bool Enemy::IsTileAheadSolid(MapChipField* map, int lookAheadTiles /*= 1*/)
{
    int type = GetTileAheadType(map, lookAheadTiles);
    // Map の仕様により「1 がブロック」な既存コードに合わせる（必要なら修正）
    IndexSet aheadIndex = map->GetMapChipIndexSetByPosition(
        Vector3{ object3D_->GetTransform().translate.x + ((velocity_.x > 0) ? 1.0f : -1.0f) * ((kEnemyWidth*0.5f) + lookAheadTiles * 1.0f),
                 object3D_->GetTransform().translate.y,
                 object3D_->GetTransform().translate.z });
    return map->IsSolid(aheadIndex.xIndex, aheadIndex.yIndex);
}