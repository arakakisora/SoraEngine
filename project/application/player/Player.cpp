#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include <cassert>
#include "Input.h"
#include "Logger.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI

#include "Object3DCommon.h"
#include "ParticleMnager.h"
#include "PlayerpaticleBehavior.h"
#include "ChargeBehabiaor.h"
#include "CollisionManager.h"
#include "LineCommon.h" // 追加: ライン描画
#include <memory>      // std::make_unique を明示

void Player::Initialize(const Vector3& position) {

	// 初期配置と Object3D の作成（unique_ptr 所有）
	playerPosition_ = position;
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player.obj");
	object3D_->SetScale(Vector3{ 0.25f,0.25f,0.25f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f,-1.82f,-4.77f });

	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });

	ParticleMnager::GetInstance()->CreateParticleGroup(
		"dash_smoke",
		"Resources/smoke.png", // 使いたいテクスチャ
		VerticesType::Quad,
		std::make_unique<ExhaustGasBehavior>()
	);

	line_ = std::make_unique<Line>();

}

AABB Player::GetPlayerAABB()
{
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;

}

void Player::Update() {

#ifdef _DEBUG

	if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		EulerTransform transform = object3D_->GetTransform();


		ImGui::DragFloat3("*PlayerRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*PlayerTranslate", &transform.translate.x, 0.01f);
		object3D_->SetTransform(transform);

		//ライティング
		Vector3 lightDirection = object3D_->GetDirectionalLight().direction;
		ImGui::DragFloat3("*LightDirection", &lightDirection.x, 0.01f);
		object3D_->SetDirectionalLightDirection(lightDirection);

		// デバッグで砲台角度表示
		ImGui::Text("CannonAngle: %.1f deg", cannonAngleDeg_);

	}
#endif // DEBUG_



	PrayerMove();

	aabb_ = GetPlayerAABB();

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		currentWeaponType_ = WeaponType::Gatling;

	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		currentWeaponType_ = WeaponType::Cannon;
		//弾のサイズを大きくする

	}

	Attack();

	for (auto& bullet : bullets_) {
		if (bullet) bullet->Update();
	}

	// 弾削除: unique_ptr を使うので delete は不要
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return bullet->GetIsDead();
		});

	// 衝突判定を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	// マップ衝突チェック
	MapCollision(collisionMapInfo);
	// 移動
	CeilingCollisionMove(collisionMapInfo);// 天井衝突時の移動処理
	PlayerCollisionMove(collisionMapInfo);// プレイヤーの移動処理
	OnGroundSwitching(collisionMapInfo);// 着地時の移動処理
	HitWallCollisionMove(collisionMapInfo);// 壁衝突時の移動処理
	PrayerTurn();
	object3D_->Update();
	PlayerParticle();

	// 落下による死亡判定
	if (object3D_->GetTransform().translate.y < deathHeight_) {
		isDead_ = true;
	}

	// 上キーで仰角を増やし、下キーで仰角を減らす。連続入力に対応。
	if (Input::GetInstance()->PushKey(DIK_UP)) {
		cannonAngleDeg_ += kCannonAngleStepDeg;
	}
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		cannonAngleDeg_ -= kCannonAngleStepDeg;
	}

	// マウスのホイール差分を取得して角度に反映
	{
		auto mouseMove = Input::GetInstance()->GetMouseMove();
		if (mouseMove.lZ != 0) {
			// WHEEL_DELTA(=120) ごとに1ノッチ。floatで扱うことで細かい差分にも対応。
			const float wheelNotches = static_cast<float>(mouseMove.lZ) / static_cast<float>(WHEEL_DELTA);
			// 1ノッチを kCannonAngleStepDeg として適用。感度を変えたい場合は係数を掛ける。
			cannonAngleDeg_ += wheelNotches * kCannonAngleStepDeg;
		}
	}

	// 過度な角度にならないように制限（-90〜+90度）
	cannonAngleDeg_ = std::clamp(cannonAngleDeg_, -90.0f, 90.0f);

	// 始点はプレイヤーのワールド位置（発射位置）
	Vector3 start = GetWorldPosition();
	start.y += 0.5f; // 少し上から発射するイメージ
	// 仰角をラジアンに変換してローカル方向を作成（ローカル座標系: +Z 前方, +Y 上）
	const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
	Vector3 localDir = { 0.0f, std::sinf(rad), std::cosf(rad) };

	// ローカル→ワールド変換（回転の影響を受ける）
	Vector3 worldDir = MyMath::TransformNormal(localDir, object3D_->GetWorldMatrix());
	Vector3 dirNorm = worldDir.Normalize();

	// 表示長さ
	const float length = 1.0f;
	Vector3 end = start + dirNorm * length;

	// 線の色(ao）
	Vector4 color = { 0.0f, 0.0f, 1.0f, 1.0f };

	// メインライン
	if (line_) line_->Draw(start, end, color);

	// 矢印の頭を描画（小さな左右の線）
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	// 右ベクトル（dir × up）
	Vector3 right = dirNorm.Cross(up).Normalize();
	// 矢印ベース
	Vector3 arrowBase = end - dirNorm * 0.2f;
	Vector3 arrow1 = arrowBase + right * 0.12f;
	Vector3 arrow2 = arrowBase - right * 0.12f;
	if (line_) {
		line_->Draw(end, arrow1, color);
		line_->Draw(end, arrow2, color);
	}

}

void Player::Draw() {
	if (object3D_) object3D_->Draw();

	for (auto& bullet : bullets_) {
		if (bullet) bullet->Draw();
	}
}

void Player::PrayerMove() {

	if (onGround_) {
		// 移動入力 を WASD に変更（A/D 左右、W ジャンプ）
		if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_D)) {

				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				if (lrDirection_ != LRDirecion::kright) {
					lrDirection_ = LRDirecion::kright;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = kTimeTurn;
				}
				// 右移動
				acceleration.x += kAcceleration;
				playerMoveRight_ = true;
				playerMoveLeft = false;

			}
			else if (Input::GetInstance()->PushKey(DIK_A)) {

				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirecion::kLeft) {
					lrDirection_ = LRDirecion::kLeft;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = kTimeTurn;
				}
				// 左移動
				acceleration.x -= kAcceleration;
				playerMoveLeft = true;
				playerMoveRight_ = false;

			}
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;
			velocity_.z += acceleration.z;

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		}
		else {

			velocity_.x *= (1.0f - kAttenuation);
			velocity_.y *= (1.0f - kAttenuation);
			velocity_.z *= (1.0f - kAttenuation);
			// スティックが真ん中なら両方falseにする
			playerMoveRight_ = false;
			playerMoveLeft = false;
		}


		// ジャンプを W に変更
		if (Input::GetInstance()->PushKey(DIK_W)) {

			velocity_.x += 0;
			velocity_.y += kJumpAcceleration;
			velocity_.z += 0;

		}

	}
	else {
		// 落下速度
		velocity_.x += 0;
		velocity_.y += -kGravityAcceleration;
		velocity_.z += 0;
		// 落下速度制限

		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::PrayerTurn() {
	if (turnTimer_ > 0.0f) {
		// 固定フレーム前提の dt（現状コードの多くが 1/60 を使っているため合わせる）
		const float dt = 1.0f / 30.0f;
		turnTimer_ -= dt;

		// 左右の角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,
			std::numbers::pi_v<float> * 3.0f / 2.0f,
		};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// turnTimer_ を kTimeTurn に対して正規化して進行率 t を得る（0..1）
		const float total = kTimeTurn;
		float t = 1.0f - (turnTimer_ / total);
		t = std::clamp(t, 0.0f, 1.0f);

		// イージング（正しい EaseOutSine）
		float eased = EaseOutSine(t);

		// 開始角度から目標角度へ補間
		float startY = turnFirstRotationY_;
		float newY = startY + (destinationRotationY - startY) * eased;
		object3D_->SetRotate({ 0.0f, newY, 0.0f });

		// 終了時に角度を厳密に合わせる
		if (turnTimer_ <= 0.0f) {
			object3D_->SetRotate({ 0.0f, destinationRotationY, 0.0f });
			turnTimer_ = 0.0f;
		}
	}
}

void Player::MapCollision(CollisionMapInfo& info) {

	CollisionMapInfoTop(info);
	CollisionMapInfoBottom(info);
	CollisionMapInfoRight(info);
	CollisionMapInfoLeft(info);
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offseetTable[kNumCorner] = {

		{+kWidth / 2.0f, -kHeight / 2.0f, 0},
		{-kWidth / 2.0f, -kHeight / 2.0f, 0},
		{+kWidth / 2.0f, +kHeight / 2.0f, 0},
		{-kWidth / 2.0f, +kHeight / 2.0f, 0}
	};

	return center + offseetTable[static_cast<uint32_t>(corner)];

}

void Player::PlayerCollisionMove(const CollisionMapInfo& info) {
	// 移動
	Vector3 position = object3D_->GetTransform().translate;
	position.x += info.move.x;
	position.y += info.move.y;
	position.z += info.move.z;
	object3D_->SetTranslate(position);


}

// 天井当たった？
void Player::CeilingCollisionMove(const CollisionMapInfo& info) {

	if (info.ceiling) {

		Logger::Log("hit ceiling\n");
		velocity_.y *= (1.0f - kAttenuationLanding);
	}
}

void Player::OnGroundSwitching(const CollisionMapInfo& info) {

	if (onGround_) {
		if (velocity_.y > 0.0f) {

			onGround_ = false;

		}
		else {
			// 移動後4つの計算
			std::array<Vector3, kNumCorner> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {

				Vector3 position = object3D_->GetTransform().translate;
				position += info.move;
				positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));
			}
			int mapChipType;
			// 真下の当たり判定
			bool hit = false;

			// 左点の判定
			IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kCollisionEpsilon, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == 1 || mapChipType == 6) {
				hit = true;
			}
			else if (mapChipType == 4) {
				goal_ = true;
			}
			// 右点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kCollisionEpsilon, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == 1 || mapChipType == 6) {
				hit = true;
			}
			else if (mapChipType == 4) {
				goal_ = true;
			}

			if (!hit) {

				onGround_ = false;
			}
		}

	}
	else {

		if (info.landing) {

			Logger::Log("hit landing\n");
			// 着地時に下向きの残留速度をキャンセルして次のジャンプを一定にする
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}
}

void Player::HitWallCollisionMove(const CollisionMapInfo& info) {

	if (info.hitWall) {

		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::CollisionMapInfoTop(CollisionMapInfo& info) {

	if (info.move.y <= 0) {
		return;
	}
	// 移動後4つの計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		Vector3 position = object3D_->GetTransform().translate;
		position += info.move;
		positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));

	}

	int mapChipType;
	// 真上のあたり判定
	bool hit = false;
	// 左点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}
	// 右点の判定
	//   左点の判定

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}
	// hit
	if (hit) {

		Vector3 position = object3D_->GetTransform().translate;

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(position + Vector3(0, -kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::max(0.0f, rect.bottom - position.y - (kHeight / 2.0f + kBlank));
		// 天井に当たったらことを記録する
		info.ceiling = true;
	}
}

void Player::CollisionMapInfoBottom(CollisionMapInfo& info) {
	if (info.move.y >= 0) {
		return;
	}
	// 移動後4つの計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		Vector3 position = object3D_->GetTransform().translate;
		position += info.move;
		positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));

	}
	int mapChipType;
	// 真下のあたり判定
	bool hit = false;

	// 左点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	// 右点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}

	// hit
	if (hit) {
		// 地面に当たったらことを記録する
		info.landing = true;
		Vector3 position = object3D_->GetTransform().translate;
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(position + Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::min(0.0f, rect.bottom - position.y + (kHeight / 2.0f + kBlank));

	}
}

void Player::CollisionMapInfoRight(CollisionMapInfo& info) {

	if (info.move.x <= 0) {
		return;
	}
	// 移動後4つの計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 position = object3D_->GetTransform().translate;
		position += info.move;
		positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));

	}

	int mapChipType;
	// 真上のあたり判定
	bool hit = false;
	// 右上点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop] + Vector3(+kCollisionEpsilon, 0, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}

	// 右下点の判定

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(+kCollisionEpsilon, 0, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}
	// hit
	if (hit) {
		// めり込みを排除する方向に移動量を設定する

		Logger::Log("hit hit wall\n");

		Vector3 position = object3D_->GetTransform().translate;
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(position + Vector3(-kWidth, 0 / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::max(0.0f, rect.right - position.x - (kWidth / 2.0f + kBlank));
		// 壁に当たったらことを記録する
		info.hitWall = true;
	}
}

void Player::CollisionMapInfoLeft(CollisionMapInfo& info) {
	if (info.move.x >= 0) {
		return;
	}
	// 移動後4つの計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		Vector3 position = object3D_->GetTransform().translate;
		position += info.move;
		positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));

	}

	int mapChipType;
	// 真上のあたり判定
	bool hit = false;
	// 左上点の判定
	IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop] + Vector3(-kCollisionEpsilon, 0, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}

	// 左下点の判定

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(-kCollisionEpsilon, 0, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == 1 || mapChipType == 6) {
		hit = true;
	}
	else if (mapChipType == 4) {
		goal_ = true;
	}
	// hit
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		Vector3 position = object3D_->GetTransform().translate;
		Logger::Log("hit wall\n");
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(position + Vector3(+kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::min(0.0f, rect.left - position.x + (kWidth / 2.0f + kBlank));
		// 壁に当たったらことを記録する
		info.hitWall = true;
	}
}

Vector3 Player::GetWorldPosition() {

	Vector3 worldPos;

	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}

void Player::Attack()
{

	if (turnTimer_ > 0.0f) return;

	// 武器ごとの設定（マジックナンバーはローカル定数に）
	int32_t kFireInterval = 0;
	int damage = 1;

	switch (currentWeaponType_) {
	case WeaponType::Gatling:
		kFireInterval = 10; // 高速連射
		damage = 1;
		for (auto& bullet : bullets_) {
			if (bullet && bullet->Getobject3DBullet_()) bullet->Getobject3DBullet_()->SetScale(Vector3{ 0.4f,0.4f,0.4f });
		}
		break;
	case WeaponType::Cannon:
		kFireInterval = 60; // リロード長い
		damage = 3;
		for (auto& bullet : bullets_) {
			if (bullet && bullet->Getobject3DBullet_()) bullet->Getobject3DBullet_()->SetScale(Vector3{ 1.5f,1.5f,1.5f });
		}
		break;
	}
#ifdef _DEBUG
	//リロードを表示
	ImGui::Text("FireInterval: %d", fireTimer);
	//モード切替
	ImGui::Text("CurrentWeaponType: %d", static_cast<int>(currentWeaponType_));
#endif // _DEBUG


	// クールタイム中は待つ
	if (fireTimer > 0) {
		--fireTimer;
	}

	if (Input::GetInstance()->PushKey(DIK_SPACE) && fireTimer <= 0) {

		fireTimer = kFireInterval;

		// 弾の速度（定数化）
		const float kBulletSpeed = 1.0f;

		Vector3 localVelocity(0, 0, kBulletSpeed);


		const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
		localVelocity.y = std::sinf(rad) * kBulletSpeed;
		localVelocity.z = std::cosf(rad) * kBulletSpeed;


		// ローカル→ワールド変換
		Vector3 velocity = MyMath::TransformNormal(localVelocity, object3D_->GetWorldMatrix());

		// Object3D を弾用に作成しOwnershipを移す
		auto obj = std::make_unique<Object3D>();
		obj->Initialize(Object3DCommon::GetInstance());
		obj->SetModel("bullet.obj");
		obj->SetScale({ 0.4f,0.4f,0.4f });

		// PlayerBullet を unique_ptr で作成し、Object3D の所有権を移す
		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(std::move(obj), GetWorldPosition(), velocity, mapChipField_);
		newBullet->SetPower(damage);

		// 正しく一度だけインスタンスを取得して登録する
		auto* cm = CollisionManager::GetInstance();
		cm->AddCollider(newBullet.get());

		bullets_.push_back(std::move(newBullet));
	}
}

void Player::PlayerParticle()
{
	// 地面にいて、左右どちらかに動いているときだけ排気ガス
	bool isMoving = onGround_ && (playerMoveRight_ || playerMoveLeft);

	const float dt = 1.0f / 60.0f; // 固定フレーム前提ならこれでOK

	if (isMoving) {
		exhaustTimer_ += dt;

		// 一定間隔ごとにだけ煙を出す
		if (exhaustTimer_ >= kExhaustInterval) {
			exhaustTimer_ = 0.0f;

			EulerTransform smokeTransform{};
			smokeTransform.translate = object3D_->GetTransform().translate;

			// 進行方向のちょい後ろに出すと“排気”感が出る
			if (lrDirection_ == LRDirecion::kright) {
				smokeTransform.translate.x -= 0.15f;
			}
			else {
				smokeTransform.translate.x += 0.15f;
			}

			// 1回に2粒くらい
			ParticleMnager::GetInstance()->Emit("dash_smoke", smokeTransform, 100, 0.8f);
		}
	}
	else {
		// 止まったらタイマーリセット
		exhaustTimer_ = 0.0f;
	}
}

float Player::EaseOutSine(float x) { return cosf((x * std::numbers::pi_v<float>) / 2); }

void Player::RegisterColliders()
{
	auto* cm = CollisionManager::GetInstance();
	// 自身を登録
	cm->AddCollider(this);
	// 弾をすべて登録
	for (auto& b : bullets_) {
		if (b) cm->AddCollider(b.get());
	}
}