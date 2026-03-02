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
#include "LineCommon.h" 
#include <memory>      

void Player::Initialize(const Vector3& position) {

	// 初期配置と Object3D の作成（unique_ptr 所有）
	playerPosition_ = position;
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player.obj");
	object3D_->SetScale(Vector3{ 0.125f,0.125f,0.125f });
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
	aabb.min = { worldPos.x - parameter_.kWidth / 2.0f, worldPos.y - parameter_.kHeight / 2.0f, worldPos.z - parameter_.kWidth / 2.0f };
	aabb.max = { worldPos.x + parameter_.kWidth / 2.0f, worldPos.y + parameter_.kHeight / 2.0f, worldPos.z + parameter_.kWidth / 2.0f };

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



	PlayerMove();// 自機の動き
	aabb_ = GetPlayerAABB();// AABB 更新
	//Attack();// 攻撃
	Playerline();// プレイヤーの移動ライン描画

	BulletUpdate();// 弾の更新

	// 衝突判定を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	// マップ衝突チェック
	MapCollision(collisionMapInfo);


	const bool touchingNow =
		collisionMapInfo.ceiling || collisionMapInfo.landing || collisionMapInfo.hitWall;

	// 非接触→接触になった瞬間だけカウント
	if (touchingNow && !wasTouching_) {
		hitCount++;
		if (hitCount >= 2) {
			playerState_ = PlayerState::sticky;
			velocity_ = { 0,0,0 }; // くっついた瞬間止めるなら
		}
	}
	wasTouching_ = touchingNow;


	Reflect(collisionMapInfo);
	PlayerCollisionMove(collisionMapInfo);// プレイヤーの移動処理
	CeilingCollisionMove(collisionMapInfo);// 天井衝突時の移動処理
	LandingCollisionMove(collisionMapInfo);// 着地時の移動処理
	HitWallCollisionMove(collisionMapInfo);// 壁衝突時の移動処理
	PlayerTurn();
	object3D_->Update();
	PlayerParticle();

	// 落下による死亡判定
	if (object3D_->GetTransform().translate.y < deathHeight_) {
		isDead_ = true;
	}

}

void Player::Draw() {
	if (object3D_) object3D_->Draw();

	for (auto& bullet : bullets_) {
		if (bullet) bullet->Draw();
	}
}

void Player::PlayerMove() {


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


	// 角度更新はそのまま（UP/DOWN + ホイール）
	cannonAngleDeg_ = std::clamp(cannonAngleDeg_, kAimMinDeg, kAimMaxDeg);

	const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
	const float spd = parameter_.kLimitRunSpeed;

	// XY平面で作る（横移動 = X、仰角 = Y、Zは常に0）
	Vector3 v{};
	Vector3 v2{};
	v.y = std::sinf(rad) * spd;
	v.x = std::cosf(rad) * spd;
	v.z = 0.0f;


	if (Input::GetInstance()->TriggerKey(DIK_D)) {
		if (lrDirection_ != LRDirecion::kright) {
			lrDirection_ = LRDirecion::kright;
			turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
			turnTimer_ = parameter_.kTimeTurn;
		}
	}

	if (Input::GetInstance()->TriggerKey(DIK_A)) {
		if (lrDirection_ != LRDirecion::kLeft) {
			lrDirection_ = LRDirecion::kLeft;
			turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
			turnTimer_ = parameter_.kTimeTurn;
		}
	}


	if (Input::GetInstance()->PushKey(DIK_SPACE)) {

		// XY平面だけ
		Vector3 v{};
		v.y = std::sinf(rad) * spd;
		v.x = std::cosf(rad) * spd;

		// 左向きならX反転（右=+、左=-）
		if (lrDirection_ == LRDirecion::kLeft) {
			v.x = -v.x;
		}

		v.z = 0.0f;
		if (velocity_.x == 0 && velocity_.y == 0) {
		}
			playerState_ = PlayerState::hard;
			velocity_ = v;
			shotVel_ = v;
			hasShotVel_ = true;
			hitCount = 0;
			wasTouching_ = false;

	}


	//if (onGround_) {
	//	// 移動入力 を WASD に変更（A/D 左右、W ジャンプ）
	//	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
	//		// 左右加速
	//		Vector3 acceleration = {};
	//		if (Input::GetInstance()->PushKey(DIK_D)) {

	//			if (velocity_.x < 0.0f) {
	//				velocity_.x *= (1.0f - parameter_.kAttenuation);
	//			}

	//			if (lrDirection_ != LRDirecion::kright) {
	//				lrDirection_ = LRDirecion::kright;
	//				turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
	//				turnTimer_ = parameter_.kTimeTurn;
	//			}
	//			// 右移動
	//			acceleration.x += parameter_.kAcceleration;
	//			playerMoveRight_ = true;
	//			playerMoveLeft = false;

	//		}
	//		else if (Input::GetInstance()->PushKey(DIK_A)) {

	//			if (velocity_.x > 0.0f) {
	//				velocity_.x *= (1.0f - parameter_.kAttenuation);
	//			}
	//			if (lrDirection_ != LRDirecion::kLeft) {
	//				lrDirection_ = LRDirecion::kLeft;
	//				turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
	//				turnTimer_ = parameter_.kTimeTurn;
	//			}
	//			// 左移動
	//			acceleration.x -= parameter_.kAcceleration;
	//			playerMoveLeft = true;
	//			playerMoveRight_ = false;

	//		}
	//		velocity_.x += acceleration.x;
	//		velocity_.y += acceleration.y;
	//		velocity_.z += acceleration.z;

	//		velocity_.x = std::clamp(velocity_.x, -parameter_.kLimitRunSpeed, parameter_.kLimitRunSpeed);

	//	}
	//	else {

	//		velocity_.x *= (1.0f -  parameter_.kAttenuation);
	//		velocity_.y *= (1.0f -  parameter_.kAttenuation);
	//		velocity_.z *= (1.0f -  parameter_.kAttenuation);
	//		// スティックが真ん中なら両方falseにする
	//		playerMoveRight_ = false;
	//		playerMoveLeft = false;
	//	}


	//	// ジャンプを W に変更
	//	if (Input::GetInstance()->PushKey(DIK_W)) {

	//		velocity_.x += 0;
	//		velocity_.y += parameter_.kJumpAcceleration;
	//		velocity_.z += 0;

	//	}

	//}
	//else {
	//	// 落下速度
	//	velocity_.x += 0;
	//	velocity_.y += -parameter_.kGravityAcceleration;
	//	velocity_.z += 0;
	//	// 落下速度制限

	//	velocity_.y = std::max(velocity_.y, -parameter_.kLimitFallSpeed);
	//}
}

Vector3 Player::NormalFromType(CollisionType type)
{
	switch (type) {
	case CollisionType::Top:    return { 0, -1, 0 };
	case CollisionType::Bottom: return { 0,  1, 0 };
	case CollisionType::Right:  return { -1, 0, 0 };
	case CollisionType::Left:   return { 1, 0, 0 };
	default: return { 0,0,0 };
	}
}

void Player::Reflect(const CollisionMapInfo& info)
{
	if (!hasShotVel_) return;
	if (playerState_ == PlayerState::sticky) return;

	const bool reflectX = info.hitWall;
	const bool reflectY = (info.ceiling || info.landing);

	if (!reflectX && !reflectY) return;

	if (reflectX) shotVel_.x = -shotVel_.x;
	if (reflectY) shotVel_.y = -shotVel_.y;

	velocity_ = shotVel_;
	velocity_ = shotVel_; 

}

void Player::Playerline()
{

	// 始点はプレイヤーのワールド位置（発射位置）
	Vector3 start = GetWorldPosition();
	start.y += 0; // 少し上から発射するイメージ
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

void Player::PlayerTurn() {
	if (turnTimer_ > 0.0f) {
		// 固定フレーム前提の dt（現状コードの多くが 1/60 を使っているため合わせる）
		const float dt = 1.0f / 30.0f;
		turnTimer_ -= dt;

		// 左右の角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,
			std::numbers::pi_v<float> *3.0f / 2.0f,
		};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// turnTimer_ を kTimeTurn に対して正規化して進行率 t を得る（0..1）
		const float total = parameter_.kTimeTurn;
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

void Player::BulletUpdate()
{

	for (auto& bullet : bullets_) {
		if (bullet) bullet->Update();
	}

	// 弾削除: unique_ptr を使うので delete は不要
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return bullet->GetIsDead();
		});

}

void Player::MapCollision(CollisionMapInfo& info) {

	const Vector3 position = object3D_->GetTransform().translate;
	MapCollisionAt(position, info);
}

void Player::MapCollisionAt(const Vector3& position, CollisionMapInfo& info) 
{
	CollisionMapInfoDirection(
		position, info, CollisionType::Right,
		{ kRightTop, kRightBottom },
		Vector3(parameter_.kCollisionEpsilon, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x > 0; }
	);

	CollisionMapInfoDirection(
		position, info, CollisionType::Left,
		{ kLeftTop, kLeftBottom },
		Vector3(-parameter_.kCollisionEpsilon, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x < 0; }
	);

	CollisionMapInfoDirection(
		position, info, CollisionType::Top,
		{ kLeftTop, kRightTop },
		Vector3(0, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.y > 0; }
	);

	CollisionMapInfoDirection(
		position, info, CollisionType::Bottom,
		{ kLeftBottom, kRightBottom },
		Vector3(0, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.y < 0; }
	);






}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner)  {

	Vector3 offseetTable[kNumCorner] = {

		{+parameter_.kWidth / 2.0f, -parameter_.kHeight / 2.0f, 0},
		{-parameter_.kWidth / 2.0f, -parameter_.kHeight / 2.0f, 0},
		{+parameter_.kWidth / 2.0f, +parameter_.kHeight / 2.0f, 0},
		{-parameter_.kWidth / 2.0f, +parameter_.kHeight / 2.0f, 0}
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

		if (playerState_ == PlayerState::sticky) {

			velocity_.x = 0.0f;
			velocity_.y = 0.0f;
			velocity_.z = 0.0f;
		} else {

			Logger::Log("hard\n");

		}
	}
}

void Player::LandingCollisionMove(const CollisionMapInfo& info) {

	//// 上向き速度が出たら地上扱い解除（ジャンプ開始）
	//if (onGround_) {
	//	if (velocity_.y > 0.0f) {
	//		onGround_ = false;
	//		return;
	//	}

	//	// 移動後の中心座標
	//	Vector3 movedPos = object3D_->GetTransform().translate + info.move;

	//	// 足元に床が無ければ落下開始
	//	if (!HasGroundBelow(movedPos)) {
	//		onGround_ = false;
	//	}
	//	return;
	//}
	// 着地フラグが立ったら着地
	if (info.landing) {

		if (playerState_ == PlayerState::sticky) {

			velocity_.x = 0.0f;
			velocity_.y = 0.0f;
			velocity_.z = 0.0f;
		} else {

			Logger::Log("hard\n");

		}
	}
}



void Player::HitWallCollisionMove(const CollisionMapInfo& info) {

	if (info.hitWall) {

		if (playerState_ == PlayerState::sticky) {

			velocity_.x = 0.0f;
			velocity_.y = 0.0f;
			velocity_.z = 0.0f;
		} else {

			Logger::Log("hard\n");

		}
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
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		currentWeaponType_ = WeaponType::Gatling;

	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		currentWeaponType_ = WeaponType::Cannon;
		//弾のサイズを大きくする

	}

	if (turnTimer_ > 0.0f) return;

	int32_t fireInterval = 0;
	int damage = 1;
	Vector3 bulletScale{ 0.4f, 0.4f, 0.4f };

	switch (currentWeaponType_) {
	case WeaponType::Gatling:
		fireInterval = 10;
		damage = 1;
		bulletScale = { 0.4f, 0.4f, 0.4f };
		break;

	case WeaponType::Cannon:
		fireInterval = 120;
		damage = 3;
		bulletScale = { 1.5f, 1.5f, 1.5f };
		break;
	}

	if (fireTimer > 0) --fireTimer;

	if (Input::GetInstance()->PushKey(DIK_SPACE) && fireTimer <= 0) {
		fireTimer = fireInterval;

		const float bulletSpeed = 1.0f;

		Vector3 localVelocity(0, 0, bulletSpeed);
		const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
		localVelocity.y = std::sinf(rad) * bulletSpeed;
		localVelocity.z = std::cosf(rad) * bulletSpeed;

		Vector3 velocity = MyMath::TransformNormal(localVelocity, object3D_->GetWorldMatrix());

		auto obj = std::make_unique<Object3D>();
		obj->Initialize(Object3DCommon::GetInstance());
		obj->SetModel("bullet.obj");
		obj->SetScale(bulletScale);

		auto newBullet = std::make_unique<PlayerBullet>();
		newBullet->Initialize(std::move(obj), GetWorldPosition(), velocity, mapChipField_);

		newBullet->SetWeaponType(currentWeaponType_);
		newBullet->SetPower(damage);

		CollisionManager::GetInstance()->AddCollider(newBullet.get());
		bullets_.push_back(std::move(newBullet));
	}
}

void Player::PlayerParticle()
{
	// 地面にいて、左右どちらかに動いているときだけ排気ガス
	bool isMoving = playerMoveRight_ || playerMoveLeft;

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
			} else {
				smokeTransform.translate.x += 0.15f;
			}

			// 1回に2粒くらい
			ParticleMnager::GetInstance()->Emit("dash_smoke", smokeTransform, 100, 0.8f);
		}
	} else {
		// 止まったらタイマーリセット
		exhaustTimer_ = 0.0f;
	}
}



float Player::EaseOutSine(float x) { return std::sinf((x * std::numbers::pi_v<float>) * 0.5f); }

bool Player::IsHittableBlock(MapChipType type)
{
	switch (type) {
	case MapChipType::Block:
	case MapChipType::UnbreakableBlock:
		return true;
	default:
		return false;
	}
}

bool Player::CheckCollisionPoints(const Vector3& basePos, const std::array<Vector3, 2>& posList, CollisionType type, CollisionMapInfo& info)
{
	bool hit = false;

	for (const auto& pos : posList) {
		IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(pos);
		MapChipType chip = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);


		if (IsHittableBlock(chip)) {
			hit = true;
		} else if (chip == MapChipType::Goal) {
			goal_ = true;
		}
	}

	if (hit) {
		
		IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(basePos);
		Rect rect = mapChipField_->GetRectByIndex(index.xIndex, index.yIndex);

		Vector3 n = NormalFromType(type);
		info.normal = info.normal + n;
		info.hasNormal = true;

		switch (type) {
		case CollisionType::Top:
			info.move.y = std::max(0.0f, rect.bottom - basePos.y - (parameter_.kHeight / 2.0f + parameter_.kBlank));
			info.ceiling = true;
			break;
		case CollisionType::Bottom:
			info.move.y = std::min(0.0f, rect.top - basePos.y + (parameter_.kHeight / 2.0f + parameter_.kBlank));
			info.landing = true;
			break;
		case CollisionType::Right:
			info.move.x = std::max(0.0f, rect.left - basePos.x - (parameter_.kWidth / 2.0f + parameter_.kBlank));
			info.hitWall = true;
			
			break;
		case CollisionType::Left:
			info.move.x = std::min(0.0f, rect.right - basePos.x + (parameter_.kWidth / 2.0f + parameter_.kBlank));
			info.hitWall = true;
			break;
		}
	}

	return hit;
}

void Player::CollisionMapInfoDirection(
	const Vector3& basePos, 
	CollisionMapInfo& info, 
	CollisionType dir, 
	const std::array<Corner, 2>& checkCorners, 
	const Vector3& offset, 
	std::function<bool(const CollisionMapInfo&)> moveCondition)
{
	if (!moveCondition(info)) return;

	Vector3 position = basePos + info.move;

	std::array<Vector3, 2> points = {
		CornerPosition(position, checkCorners[0]) + offset,
		CornerPosition(position, checkCorners[1]) + offset
	};

	CheckCollisionPoints(basePos, points, dir, info);
}

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