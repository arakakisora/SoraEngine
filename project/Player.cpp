#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include <cassert>
#include "Input.h"
#include "Logger.h"
#include <imgui.h>

#include "Object3DCommon.h"
#include "ParticleMnager.h"
#include "plyerpaticleBehavior.h"



void Player::Initialize(const Vector3& position) {

	
	

	object3D_ = new Object3D();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player.obj");
	object3D_->SetScale(Vector3{ 0.25f,0.25f,0.25f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f,-1.82f,-4.77f });
	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });

	////パーティクル
	//ParticleMnager::GetInstance()->CreateParticleGroup("dash", "Resources/uvChecker_png.png", VerticesType::Quad, std::make_unique<ExhaustGasBehavior>());
	//dashparticleEmitter_ = new ParticleEmitter(object3D_->GetTransform(), 1.5f, 0.0f, 1, "dash");
	//
}

Player::~Player()
{
	// プレイヤーの弾を削除
	for (PlayerBullet* bullet : bullets_) {

		delete bullet;
	}

	delete object3D_;



}

void Player::Update() {

#ifdef _DEBUG
	ImGui::Begin("Player Debug");

	if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		EulerTransform transform = object3D_->GetTransform();

		//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
		ImGui::DragFloat3("*PlayerRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*PlayerTransrate", &transform.translate.x, 0.01f);
		object3D_->SetTransform(transform);

		//ライティング
		Vector3 lightDirection = object3D_->GetDirectionalLight().direction;
		ImGui::DragFloat3("*LightDirection", &lightDirection.x, 0.01f);
		object3D_->SetDirectionalLightDirection(lightDirection);

		//体力とレベルとダメージクールタイム
		ImGui::Text("Player HP: %d", hp_);
		ImGui::Text("Player Level: %d", level_);
		ImGui::Text("Player Damage Cool Timer: %d", damageCoolTimer_);
		

	}
	ImGui::End();
#endif // DEBUG_

	if (damageCoolTimer_ > 0) {
		--damageCoolTimer_;
	};

	PrayerMove();

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		currentWeaponType_ = WeaponType::Gatling;

	}
	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		currentWeaponType_ = WeaponType::Cannon;
		//弾のサイズを大きくする
		
	}

	Attack();

	for (PlayerBullet* bullet : bullets_) {

		bullet->Update();

	}

	//bulletデス
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->GetIsDead()) {
			delete bullet;

			return true;

		}
		return false;
		});




	// 衝突判定を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;
	// マップ衝突チェック
	MapCollision(collisionMapInfo);
	// 移動
	CeilingCollisionMove(collisionMapInfo);
	PlayerCollisionMove(collisionMapInfo);
	OnGroundSwitching(collisionMapInfo);
	HitWallCollisionMove(collisionMapInfo);
	PrayerTurn();
	object3D_->Update();

	// 落下による死亡判定
	if (object3D_->GetTransform().translate.y < deathHeight_) {
		isDead_ = true;
	}

}

void Player::Draw() {
	object3D_->Draw();

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw();
	}
}

void Player::PrayerMove() {

		
	if (onGround_) {
		// 移動入力
		// 左右移動操作
		/*dashparticleEmitter_->SetPosition(object3D_->GetTransform().translate);
		dashparticleEmitter_->Update();*/
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			// 左右加速
			Vector3 accceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - playerParameter_.kAttenuation);
				}

				if (lrDirection_ != LRDirecion::kright) {
					lrDirection_ = LRDirecion::kright;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = playerParameter_.kLimitRunSpeed;
				}

				accceleration.x += playerParameter_.kAccleration;
				playermoveright = true;
				playermoveleft = false;
				
			}
			else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - playerParameter_.kAttenuation);
				}
				if (lrDirection_ != LRDirecion::kLeft) {
					lrDirection_ = LRDirecion::kLeft;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = playerParameter_.kLimitRunSpeed;
				}

				accceleration.x -= playerParameter_.kAccleration;
				playermoveleft = true;
				playermoveright = false;
				
			}
			velocity_.x += accceleration.x;
			velocity_.y += accceleration.y;
			velocity_.z += accceleration.z;

			velocity_.x = std::clamp(velocity_.x, -playerParameter_.kLimitRunSpeed, playerParameter_.kLimitRunSpeed);

		}
		else {

			velocity_.x *= (1.0f - playerParameter_.kAttenuation);
			velocity_.y *= (1.0f - playerParameter_.kAttenuation);
			velocity_.z *= (1.0f - playerParameter_.kAttenuation);
			// スティックが真ん中なら両方falseにする
			playermoveright = false;
			playermoveleft = false;
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {

			velocity_.x += 0;
			velocity_.y += playerParameter_.kJampAcceleration;
			velocity_.z += 0;
		}

	}
	else {
		// 落下速度
		velocity_.x += 0;
		velocity_.y += -playerParameter_.kGravityAccleration;
		velocity_.z += 0;
		// 落下速度制限

		velocity_.y = std::max(velocity_.y, -playerParameter_.kLimitFallSpeed);
	}
}

void Player::PrayerTurn() {
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 30.0f;

		// 左右の角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,
			std::numbers::pi_v<float> *3.0f / 2.0f,
		};
		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		object3D_->SetRotate({ 0,destinationRotationY * EaseOutSine(turnTimer_) ,0 });



	}
}





void Player::MapCollision(CollisionMapInfo& info) {

	CollisionMapInfoDirection(
		info,
		CollisionType::Right,
		{ kRightTop, kRightBottom },
		Vector3(playerParameter_.kCollisionsmallnumber, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x > 0; }
	);

	CollisionMapInfoDirection(
		info,
		CollisionType::Left,
		{ kLeftTop, kLeftBottom },
		Vector3(-playerParameter_.kCollisionsmallnumber, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x < 0; }
	);

	CollisionMapInfoDirection(
		info,
		CollisionType::Top,
		{ kLeftTop, kRightTop },
		Vector3(0, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.y > 0; }
	);

	CollisionMapInfoDirection(
		info,
		CollisionType::Bottom,
		{ kLeftBottom, kRightBottom },
		Vector3(0, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.y < 0; }
	);
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offseetTable[kNumCorner] = {

		{+playerParameter_.kWidth / 2.0f, -playerParameter_.kHeight / 2.0f, 0},
		{-playerParameter_.kWidth / 2.0f, -playerParameter_.kHeight / 2.0f, 0},
		{+playerParameter_.kWidth / 2.0f, +playerParameter_.kHeight / 2.0f, 0},
		{-playerParameter_.kWidth / 2.0f, +playerParameter_.kHeight / 2.0f, 0}
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
		velocity_.y = 0.0f;
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
			MapChipType mapChipType;
			// 真下の当たり判定
			bool hit = false;

			// 左点の判定
			IndexSet indexSet;
			indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -playerParameter_.kCollisionsmallnumber, 0));
			mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			else if (mapChipType == MapChipType::kGoal) {
				goal_ = true;
			}
			// 右点の判定
			indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -playerParameter_.kCollisionsmallnumber, 0));
			mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			else if (mapChipType == MapChipType::kGoal) {
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
			velocity_.x *= (1.0f - playerParameter_.kAttenuationLanding);
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}
}

void Player::HitWallCollisionMove(const CollisionMapInfo& info) {

	if (info.hitWall) {

		velocity_.x *= (1.0f - playerParameter_.kAttenuationWall);
	}
}




Vector3 Player::GetWorldPosition() {

	Vector3 worldPos;

	worldPos.x = object3D_->GetWorldMatrix().m[3][0];;
	worldPos.y = object3D_->GetWorldMatrix().m[3][1];;
	worldPos.z = object3D_->GetWorldMatrix().m[3][2];;
	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - playerParameter_.kWidth / 2.0f, worldPos.y - playerParameter_.kHeight / 2.0f, worldPos.z - playerParameter_.kWidth / 2.0f };
	aabb.max = { worldPos.x + playerParameter_.kWidth / 2.0f, worldPos.y + playerParameter_.kHeight / 2.0f, worldPos.z + playerParameter_.kWidth / 2.0f };

	return aabb;
}

void Player::TakeDamage(int damage)
{
	if (damageCoolTimer_ > 0) return; // クールタイム中は無視

	hp_ -= damage;
	damageCoolTimer_ = kDamageCoolTime;

	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
	}
}

bool Player::CanTakeDamage() const
{
	return damageCoolTimer_ <= 0;
}


void Player::Attack()
{
	
	if (turnTimer_ > 0.0f) return;

	// 武器ごとの設定
	int32_t kFireInterval = 0;
	int damage = 1;

	switch (currentWeaponType_) {
	case WeaponType::Gatling:
		kFireInterval = 10; // 高速連射
		damage = 1;
		for (PlayerBullet* bullet : bullets_) {
			object3DBullet_->SetScale(Vector3{ 0.4f,0.4f,0.4f });
		}
		break;
	case WeaponType::Cannon:
		kFireInterval = 60; // リロード長い
		damage = 3;
		for (PlayerBullet* bullet : bullets_) {
			object3DBullet_->SetScale(Vector3{ 1.5f,1.5f,1.5f });
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

		// 弾の速度
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(0, 0, kBulletSpeed);
		velocity = MyMath::TransformNormal(velocity, object3D_->GetWorldMatrix());

		object3DBullet_ = new Object3D();
		object3DBullet_->Initialize(Object3DCommon::GetInstance());
		object3DBullet_->SetModel("bullet.obj");
		object3DBullet_->SetScale({ 0.4f,0.4f,0.4f });

		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(object3DBullet_, GetWorldPosition(), velocity, mapChipFild_);
		newBullet->SetPower(damage); // ← ダメージを設定（次のステップで追加）

		bullets_.push_back(newBullet);



	}


}

float Player::EaseOutSine(float x) { return cosf((x * std::numbers::pi_v<float>) / 2); }

bool Player::CheckCollisionPoints(const std::array<Vector3, 2>& posList, CollisionType type, CollisionMapInfo& info)
{
	bool hit = false;

	for (const auto& pos : posList) {
		IndexSet index = mapChipFild_->GetMapChipIndexSetByPosition(pos);
		MapChipType chip = mapChipFild_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		if (IsHittableBlock(chip)) {
			hit = true;
		}
		else if (chip == MapChipType::kGoal) {
			
		}
	}

	if (hit) {
		Vector3 position = object3D_->GetTransform().translate;
		IndexSet index = mapChipFild_->GetMapChipIndexSetByPosition(position);

		Rect rect = mapChipFild_->GetRectByIndex(index.xIndex, index.yIndex);

		switch (type) {
		case CollisionType::Top:
			info.move.y = std::max(0.0f, rect.bottom - position.y - (playerParameter_.kHeight / 2.0f + playerParameter_.kBlank));
			info.ceiling = true;
			break;
		case CollisionType::Bottom:
			info.move.y = std::min(0.0f, rect.top - position.y + (playerParameter_.kHeight / 2.0f + playerParameter_.kBlank));
			info.landing = true;
			break;
		case CollisionType::Right:
			info.move.x = std::max(0.0f, rect.left - position.x - (playerParameter_.kWidth / 2.0f + playerParameter_.kBlank));
			info.hitWall = true;
			break;
		case CollisionType::Left:
			info.move.x = std::min(0.0f, rect.right - position.x + (playerParameter_.kWidth / 2.0f + playerParameter_.kBlank));
			info.hitWall = true;
			break;
		}
	}

	return hit;
}

void Player::CollisionMapInfoDirection(CollisionMapInfo& info, CollisionType dir, const std::array<Corner, 2>& checkCorners, const Vector3& offset, std::function<bool(const CollisionMapInfo&)> moveCondition)
{
	if (!moveCondition(info)) return;// 移動量が0なら何もしない
	Vector3 position = object3D_->GetTransform().translate + info.move;// 現在の位置に移動量を加算

	// 2つのコーナー位置を計算
	std::array<Vector3, 2> points = {
		CornerPosition(position, checkCorners[0]) + offset,
		CornerPosition(position, checkCorners[1]) + offset
	};
	// 衝突判定を行う
	if (CheckCollisionPoints(points, static_cast<CollisionType>(dir), info)) {
#ifdef _DEBUG
		switch (dir) {
		case CollisionType::Top: Logger::Log("hit ceiling\n"); break;
		case CollisionType::Bottom: Logger::Log("hit landing\n"); break;
		case CollisionType::Left:
		case CollisionType::Right: Logger::Log("hit hitwall\n"); break;
		}
#endif
	}
}

bool Player::IsHittableBlock(MapChipType type)
{
	switch (type) { 
	case MapChipType::kBlock: 
	case MapChipType::kGoal:  
		return true;
	default:
		return false;
	}
}
