#define NOMINMAX
#include "Player.h"
#include "MapChipField.h"
#include <cassert>
#include "Input.h"
#include "Logger.h"
#include <imgui.h>

#include "Object3DCommon.h"



void Player::Initialize(Object3D* object3D, const Vector3& position) {

	// モデルの初期化
	object3D_ = object3D;
	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });



}

Player::~Player()
{
	// プレイヤーの弾を削除
	for (PlayerBullet* bullet : bullets_) {

		delete bullet;
	}

	delete object3DBullet_;
	
		
		
}

void Player::Update() {

#ifdef DEBUG_

	if (ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Transform transform = object3D_->GetTransform();

		//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
		ImGui::DragFloat3("*PlayerRotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("*PlayerTransrate", &transform.translate.x, 0.01f);
		object3D_->SetTransform(transform);

	}
#endif // DEBUG_


	PrayerMove();
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
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			// 左右加速
			Vector3 accceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				if (lrDirection_ != LRDirecion::kright) {
					lrDirection_ = LRDirecion::kright;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = kLimitRunSpeed;
				}

				accceleration.x += kAccleration;

			}
			else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirecion::kLeft) {
					lrDirection_ = LRDirecion::kLeft;
					turnFirstRotationY_ = object3D_->GetTransform().rotate.y;
					turnTimer_ = kLimitRunSpeed;
				}

				accceleration.x -= kAccleration;
			}
			velocity_.x += accceleration.x;
			velocity_.y += accceleration.y;
			velocity_.z += accceleration.z;

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		}
		else {

			velocity_.x *= (1.0f - kAttenuation);
			velocity_.y *= (1.0f - kAttenuation);
			velocity_.z *= (1.0f - kAttenuation);
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {

			velocity_.x += 0;
			velocity_.y += kJampAcceleration;
			velocity_.z += 0;
		}

	}
	else {
		// 落下速度
		velocity_.x += 0;
		velocity_.y += -kGravityAccleration;
		velocity_.z += 0;
		// 落下速度制限

		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
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


//void Player::PrayerTurn() {
//    if (turnTimer_ > 0.0f) {
//        turnTimer_ -= 1.0f / 30.0f;
//
//        // 左右の目標角度テーブル
//        float destinationRotationYTable[] = {
//            std::numbers::pi_v<float> / 2.0f,           // 右回転
//            3.0f * std::numbers::pi_v<float> / 2.0f    // 左回転（ここを変更）
//        };
//
//        // 現在の回転角度
//        float currentRotationY = turnFirstRotationY_;
//
//        // 目標角度を取得
//        float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
//
//        // 差分を計算し、範囲を 0 ～ 2π に正規化
//        float angleDifference = destinationRotationY - currentRotationY;
//        if (angleDifference > std::numbers::pi_v<float>) {
//            angleDifference -= 2.0f * std::numbers::pi_v<float>;
//        } else if (angleDifference < -std::numbers::pi_v<float>) {
//            angleDifference += 2.0f * std::numbers::pi_v<float>;
//        }
//
//        // EaseOutSine を用いて補間
//        float easedRotationY = currentRotationY + angleDifference * EaseOutSine(1.0f - turnTimer_);
//
//        // 回転角度を設定
//        object3D_->SetRotate({0, easedRotationY, 0});
//    }
//}


void Player::MapCollision(CollisionMapInfo& info) {

	CollisionMapInfoTop(info);
	CollisionMapInfoBootm(info);
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

	/*if (corner == kRightBottom) {
		return center + Vector3{+kWidth / 2.0f, -kHeight / 2.0f, 0};
	} else if (corner == kLeftBottom){
		return center + Vector3{-kWidth / 2.0f, -kHeight / 2.0f, 0};
	} else if (corner == kRightTop) {
		return center + Vector3{+kWidth / 2.0f, +kHeight / 2.0f, 0};
	} else {
		return center + Vector3{-kWidth / 2.0f, +kHeight / 2.0f, 0};
	}*/
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
			indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kCollisionsmallnumber, 0));
			mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右点の判定
			indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kCollisionsmallnumber, 0));
			mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			if (!hit) {

				onGround_ = false;
			}
		}

	}
	else {

		if (info.landing) {

			Logger::Log("hit landing\n");
			velocity_.x *= (1.0f - kAttenuationLanding);
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

		//positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真上の当たり判定
	bool hit = false;
	// 左点の判定
	IndexSet indexSet;
	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右点の判定
	//   左点の判定

	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// hit
	if (hit) {

		Vector3 position = object3D_->GetTransform().translate;

		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFild_->GetMapChipIndexSetByPosition(position + Vector3(0, -kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipFild_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::max(0.0f, rect.bottom - position.y - (kHeight / 2.0f + kBlank));
		// 天井に当たったらことを記録する
		info.ceiling = true;
	}
}

void Player::CollisionMapInfoBootm(CollisionMapInfo& info) {
	if (info.move.y >= 0) {
		return;
	}
	// 移動後4つの計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {

		Vector3 position = object3D_->GetTransform().translate;
		position += info.move;
		positionsNew[i] = CornerPosition(position, static_cast<Corner>(i));
		//positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	MapChipType mapChipType;
	// 真下の当たり判定
	bool hit = false;

	// 左点の判定
	IndexSet indexSet;
	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右点の判定
	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// hit
	if (hit) {

		Vector3 position = object3D_->GetTransform().translate;
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFild_->GetMapChipIndexSetByPosition(position + Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipFild_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::min(0.0f, rect.bottom - position.y + (kHeight / 2.0f + kBlank));
		// 地面に当たったらことを記録する
		info.landing = true;
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

		//positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真上の当たり判定
	bool hit = false;
	// 右上点の判定
	IndexSet indexSet;
	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightTop] + Vector3(+kCollisionsmallnumber, 0, 0));
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定

	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(+kCollisionsmallnumber, 0, 0));
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// hit
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		//DebugText::GetInstance()->ConsolePrintf("hit hitwall\n");

		Logger::Log("hit hitwall\n");

		Vector3 position = object3D_->GetTransform().translate;
		indexSet = mapChipFild_->GetMapChipIndexSetByPosition(position + Vector3(-kWidth, 0 / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipFild_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
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
		//positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真上の当たり判定
	bool hit = false;
	// hidari上点の判定
	IndexSet indexSet;
	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop] + Vector3(-kCollisionsmallnumber, 0, 0));
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// hidari下点の判定

	indexSet = mapChipFild_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(-kCollisionsmallnumber, 0, 0));
	mapChipType = mapChipFild_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// hit
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		//DebugText::GetInstance()->ConsolePrintf("hit hitwall\n");
		Vector3 position = object3D_->GetTransform().translate;
		Logger::Log("hit hitwall\n");
		indexSet = mapChipFild_->GetMapChipIndexSetByPosition(position + Vector3(+kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		Rect rect = mapChipFild_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
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

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {

	(void)enemy;


	isDead_ = true;

}

void Player::Attack()
{
	// 回転中は弾を発射しない
	if (turnTimer_ > 0.0f) {
		return;
	}
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {

		//弾の速度
		const float kBlletSpeed = 1.0f;
		Vector3 velocity(0, 0, kBlletSpeed);

		//自機の向きに合わせて回転させる
		velocity = MyMath::TransformNormal(velocity, object3D_->GetWorldMatrix());


		object3DBullet_ = new Object3D();
		object3DBullet_->Initialize(Object3DCommon::GetInstance());
		object3DBullet_->SetModel("bullet.obj");
		object3DBullet_->SetScale({ 0.4f,0.4f,0.4f });

		

		//弾生成、初期化
		PlayerBullet* newbBullet = new PlayerBullet();
		newbBullet->Initialize(object3DBullet_, GetWorldPosition(), velocity,mapChipFild_);


		//弾を登録
		bullets_.push_back(newbBullet);



	}


}

float Player::EaseOutSine(float x) { return cosf((x * std::numbers::pi_v<float>) / 2); }
