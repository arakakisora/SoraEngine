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
#include "ParticleManager.h"
#include "PlayerpaticleBehavior.h"
#include "ChargeBehavior.h"
#include "CollisionManager.h"
#include "LineCommon.h" 
#include <memory>      
#include "Easing.h"
#include <array> 
#include "CameraManager.h"
#include <UIeditor.h>

void Player::Initialize(const Vector3& position) {

	// 初期配置と Object3D の作成（unique_ptr 所有）
	playerPosition_ = position;
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player");
	object3D_->SetScale(Vector3{ 0.5f,0.5f,0.5f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f,-1.82f,-4.77f });
	// プレイヤーの初期位置
	object3D_->SetTranslate(position);
	object3D_->SetRotate({ 0, std::numbers::pi_v<float> / 2.0f , 0 });

	// ゴーストの作成
	ghostObject_ = std::make_unique<Object3D>();
	ghostObject_->Initialize(Object3DCommon::GetInstance());
	ghostObject_->SetModel("player");
	ghostObject_->SetScale(Vector3{ 0.5f, 0.5f, 0.5f });
	ghostObject_->SetColor({ 0.2f, 0.8f, 1.0f, 0.35f });
	// ゴーストは少し暗くしたいのでライトOFF
	ghostObject_->SetLighting(false);

	//ParticleManager::GetInstance()->CreateParticleGroup(
	//	"dash_smoke",
	//	"Resources/smoke.png", // 使いたいテクスチャ
	//	VerticesType::Quad,
	//	std::make_unique<ExhaustGasBehavior>()
	//);
	exhaustEmitter_ = std::make_unique<ParticleEmitter>(
		EulerTransform{ position, {0,0,0}, {1,1,1} },
		1.0f, // lifetime
		0.0f, // currentTime
		100,   // count
		"dash_smoke"
	);

	ParticleManager::GetInstance()->CreateParticleGroup("enemydeath", "Resources/ParticleTexture/honoo.png", VerticesType::Quad, std::make_unique<ExplosionBehavior>());
	deathEffect_ = std::make_unique<ParticleEmitter>(
		EulerTransform{ position, {0,0,0}, {1,1,1} },
		1.0f, // lifetime
		0.0f, // currentTime
		100,   // count
		"enemydeath"
	);;


	line_ = std::make_unique<Line>();

}

void Player::OnCollision(Collider* other)
{

	constexpr std::size_t LAYER_COUNT = 5;
	static constexpr std::array<bool, LAYER_COUNT> lethalWhenSticky{ {
			/* Player       */ false,
			/* Enemy        */ true,
			/* Enemy2       */ true,
			/* PlayerBullet */ false,
			/* EnemyBullet  */ false
		} };

	const auto idx = static_cast<std::size_t>(other->GetLayer());
	if (idx < LAYER_COUNT && lethalWhenSticky[idx]) {
		if (playerState_ == PlayerState::sticky) {
			SetIsDead(true);
		}
	}
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
	ImGui::Begin("PlayerDebug");
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
	ImGui::End();
#endif // DEBUG_

	if (portalCooldown_ > 0) {
		portalCooldown_--;
	}

	PlayerMove();// 自機の動き
	aabb_ = GetPlayerAABB();// AABB 更新
	//Attack();// 攻撃
	PlayerLine();// プレイヤーの移動ライン描画
	DrawPredictLine();

	// 衝突判定を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	// マップ衝突チェック
	MapCollision(collisionMapInfo);// マップとの衝突判定と情報の取得
	Reflect(collisionMapInfo);// 反射処理
	PlayerCondition(collisionMapInfo);// プレイヤーの状態更新	

	if (collisionMapInfo.hitDamageBlock) {
		SetIsDead(true);
	}
	if (collisionMapInfo.hasBreakBlock && playerState_ == PlayerState::hard) {
		const uint32_t bx = collisionMapInfo.breakBlockX;
		const uint32_t by = collisionMapInfo.breakBlockY;

		int beforeHp = mapChipField_->GetMapChipHPByIndex(bx, by);
		if (beforeHp > 0) {
			mapChipField_->DamageMapChipByIndex(bx, by, 1);

			int afterHp = mapChipField_->GetMapChipHPByIndex(bx, by);

			// 壊れた瞬間だけエフェクト
			if (afterHp <= 0 && deathEffect_) {
				Rect r = mapChipField_->GetRectByIndex(bx, by);

				Vector3 breakPos{};
				breakPos.x = (r.left + r.right) * 0.5f;
				breakPos.y = (r.top + r.bottom) * 0.5f;
				breakPos.z = object3D_->GetTransform().translate.z;

				deathEffect_->SetPosition(breakPos);
				deathEffect_->Emit();
				//カメラシェイク
				StartCameraShake(0.08f, 0.15f);


			}
		}
	}

	PlayerShotAnimation();

	PlayerCollisionMove(collisionMapInfo);// プレイヤーの移動処理

	// ポータル通過
	{
		Vector3 pos = object3D_->GetTransform().translate;

		if (TryPortalWarp(pos, velocity_,true)) {
			object3D_->SetTranslate(pos);

			shotVel_ = velocity_;
			hasShotVel_ = true;

			// ポータルは壁接触扱いにしない
			wasTouching_ = false;
		}
	}

	CeilingCollisionMove(collisionMapInfo);// 天井衝突時の移動処理
	LandingCollisionMove(collisionMapInfo);// 着地時の移動処理
	HitWallCollisionMove(collisionMapInfo);// 壁衝突時の移動処理
	PlayerDirection(collisionMapInfo);// プレイヤーの振り向き

	// プレイヤーのワールド位置を更新
	object3D_->Update();
	// パーティクルの更新
	PlayerParticle();
	// 死亡条件の判定
	PlayerDeathTerms();

	UpdateCameraShake();

}

void Player::Draw() {

	DrawGhost();

	if (object3D_) object3D_->Draw();


}

void Player::PlayerMove() {


	// 上キーで仰角を増やし、下キーで仰角を減らす。連続入力に対応。
	if (Input::GetInstance()->PushKey(DIK_UP)) {
		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "UP");
		cannonAngleDeg_ += kCannonAngleStepDeg;
	}
	if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "down");
		cannonAngleDeg_ -= kCannonAngleStepDeg;
	}

	// マウスのホイール差分を取得して角度に反映

	
	auto mouseMove = Input::GetInstance()->GetMouseMove();
	if (mouseMove.lZ != 0) {
		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "mausu");
		// WHEEL_DELTA(=120) ごとに1ノッチ。floatで扱うことで細かい差分にも対応。
		const float wheelNotches = static_cast<float>(mouseMove.lZ) / static_cast<float>(WHEEL_DELTA);
		// 1ノッチを kCannonAngleStepDeg として適用。感度を変えたい場合は係数を掛ける。
		cannonAngleDeg_ += wheelNotches * kCannonAngleStepDeg;
	}



	// 角度更新はそのまま（UP/DOWN + ホイール）
	cannonAngleDeg_ = std::clamp(cannonAngleDeg_, kAimMinDeg, kAimMaxDeg);

	const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
	const float spd = parameter_.kLimitRunSpeed;

	Vector3 localDir{};
	localDir.y = std::sinf(rad);
	localDir.z = std::cosf(rad);
	localDir.x = 0.0f;

	// ワールド変換（回転を考慮）
	Vector3 worldDir = MyMath::TransformNormal(
		localDir,
		object3D_->GetWorldMatrix()
	);

	Vector3 v = worldDir.Normalize() * spd;

	//スペースで発射
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "SPACE");

		const bool canShoot =
			(playerState_ == PlayerState::sticky) || isStopped_;

		if (canShoot) {

			playerState_ = PlayerState::hard;
			isBarrierActive_ = true;


			// 発射アニメ開始
			isShotAnimation_ = true;
			shotAnimationTimer_ = 0.0f;

			Vector3 v = MakeShotVelocity();

			velocity_ = v;
			shotVel_ = v;
			hasShotVel_ = true;

			hitCount = 0;
			wasTouching_ = false;
		}
	}

}


void Player::PlayerCondition(const CollisionMapInfo& info)
{
	// 停止判定
	isStopped_ =
		std::abs(info.move.x) < kStopEps &&
		std::abs(info.move.y) < kStopEps;

	const bool touchingNow =
		info.ceiling || info.landing || info.hitWall;

	// 接触になった瞬間だけカウント
	if (touchingNow && !wasTouching_) {
		hitCount++;

		// 1回目に壁・床・天井へ触れた瞬間
		if (hitCount == 1 && playerState_ == PlayerState::hard) {
			isBarrierActive_ = false;

			if (playerState_ == PlayerState::hard) {
				ParticleManager::GetInstance()->Emit("BarrierBreak", object3D_->GetTransform());
			}
		}

		if (hitCount >= 2) {


			playerState_ = PlayerState::sticky;
			velocity_ = { 0,0,0 };
		}
	}
	wasTouching_ = touchingNow;


}

Vector3 Player::NormalFromType(CollisionType type)
{
	//CollisionType の順序 (Top, Bottom, Right, Left) に合わせたルックアップテーブル
	static const std::array<Vector3, 4> normals{ {
			/* Top    */ Vector3{ 0.0f, -1.0f, 0.0f },
			/* Bottom */ Vector3{ 0.0f,  1.0f, 0.0f },
			/* Left   */ Vector3{ 1.0f,  0.0f, 0.0f },
			/* Right  */ Vector3{ -1.0f, 0.0f, 0.0f }
		} };

	const auto idx = static_cast<std::size_t>(type);
	if (idx < normals.size()) return normals[idx];
	return { 0.0f, 0.0f, 0.0f };
}

void Player::Reflect(const CollisionMapInfo& info)
{
	if (playerState_ == PlayerState::sticky) return;
	if (!info.hasNormal) return;

	Vector3 n{};

	const bool hitX = info.hitWall;
	const bool hitY = (info.ceiling || info.landing);

	if (hitX && hitY) {
		if (info.penX >= info.penY) {
			// 壁で反射
			n = { (velocity_.x > 0.0f) ? -1.0f : 1.0f, 0.0f, 0.0f };
		}
		else {
			// 天井/床で反射
			n = { 0.0f, (velocity_.y > 0.0f) ? -1.0f : 1.0f, 0.0f };
		}
	}
	else {
		// 合成法線
		n = MyMath::Normalize(info.normal);
	}

	if (n.x == 0 && n.y == 0 && n.z == 0) return;

	const float vn = MyMath::Dot(velocity_, n);
	if (vn >= 0.0f) return;

	velocity_ = velocity_ - n * (2.0f * vn);


	shotVel_ = velocity_;
	hasShotVel_ = true;

}

void Player::PlayerLine()
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

void Player::DrawPredictLine()
{
	if (!line_) return;
	if (!mapChipField_) return;

	// ゴーストは予測線と同時に表示する
	isGhostVisible_ = false;

	// 速度0のとき（発射前）だけ予測線を見せたいならこれ
	if (!(velocity_.x == 0.0f && velocity_.y == 0.0f)) return;

	// 発射ベクトル（予測用）
	Vector3 simVel = MakeShotVelocity();
	Vector3 simPos = object3D_->GetTransform().translate;

	// 2回触れたらsticky
	int hitCountSim = 0;
	bool wasTouchingSim = false;

	// 色（好きに）
	Vector4 color = { 1.0f, 1.0f, 0.0f, 1.0f };

	// 何ステップ先まで描くか
	const int kMaxSteps = 240; // 30fps想定なら8秒
	const float epsPush = parameter_.kCollisionEpsilon * 2.0f;

	Vector3 prev = simPos;

	for (int step = 0; step < kMaxSteps; ++step) {

		Vector3 moveDirBeforeHit = simVel.Normalize();

		CollisionMapInfo info{};
		info.move = simVel;
		info.normal = { 0,0,0 };
		info.hasNormal = false;

		MapCollisionAt(simPos, info, false);

		const bool touchingNow = info.ceiling || info.landing || info.hitWall;

		if (touchingNow && !wasTouchingSim) {
			hitCountSim++;
		}

		wasTouchingSim = touchingNow;

		simPos = simPos + info.move;

		line_->Draw(prev, simPos, color);
		prev = simPos;

		Vector3 beforePortalPos = simPos;

		if (TryPortalWarp(simPos, simVel, false)) {
			Vector4 portalColor = { 0.0f, 1.0f, 1.0f, 1.0f };

			// 入口から出口へつながる線
			line_->Draw(beforePortalPos, simPos, portalColor);

			prev = simPos;

			// ポータルは壁接触扱いにしない
			wasTouchingSim = false;
		}

		if (hitCountSim >= 2) {
			Vector3 nextFacingDir = GetFacingDirFromCollisionInfo(info);

			SetGhostPreview(simPos, nextFacingDir);// ゴーストの位置と向きをセットして表示
			DrawGhostAimPreview(simPos, nextFacingDir);// ゴーストの射出可能方向を表示	

			break;
		}

		// 反射
		if (info.hasNormal) {
			Vector3 n{};
			const bool hitX = info.hitWall;
			const bool hitY = (info.ceiling || info.landing);

			if (hitX && hitY) {
				if (info.penX >= info.penY) {
					n = { (simVel.x > 0.0f) ? -1.0f : 1.0f, 0.0f, 0.0f };
				}
				else {
					n = { 0.0f, (simVel.y > 0.0f) ? -1.0f : 1.0f, 0.0f };
				}
			}
			else {
				n = MyMath::Normalize(info.normal);
			}

			ReflectVelocity(simVel, n);
		}

		// 速度がほぼゼロなら終了
		if (std::abs(simVel.x) < 1e-6f && std::abs(simVel.y) < 1e-6f) {
			break;
		}
	}

}

void Player::DrawGhostAimPreview(const Vector3& landingPos, const Vector3& nextFacingDir)
{
	if (!line_) return;

	Vector3 baseDir = nextFacingDir.Normalize();
	if (std::abs(baseDir.x) < 1e-6f && std::abs(baseDir.y) < 1e-6f) {
		return;
	}

	// -45〜+45度
	const float maxAngleRad = 45.0f * (std::numbers::pi_v<float> / 180.0f);

	// 時間で矢印を往復させる
	// sinなので端で少しゆっくりになって見た目が自然
	static float previewTimer = 0.0f;
	previewTimer += 1.0f / 60.0f;

	const float swing = std::sinf(previewTimer * 2.0f);
	const float previewRad = swing * maxAngleRad;

	Vector3 previewDir = RotateVectorZ(baseDir, previewRad).Normalize();

	// 表示位置を少し手前に浮かせる
	Vector3 start = landingPos;
	start.z -= 0.05f;

	const float arrowLength = 1.0f;
	Vector3 end = start + previewDir * arrowLength;

	// ゴーストのスイング矢印色
	Vector4 arrowColor = { 0.0f, 1.0f, 1.0f, 1.0f };

	line_->Draw(start, end, arrowColor);

	// 矢印の頭
	Vector3 zAxis = { 0.0f, 0.0f, 1.0f };
	Vector3 right = previewDir.Cross(zAxis).Normalize();

	const float headLength = 0.2f;
	const float headWidth = 0.12f;

	Vector3 arrowBase = end - previewDir * headLength;
	Vector3 arrow1 = arrowBase + right * headWidth;
	Vector3 arrow2 = arrowBase - right * headWidth;

	line_->Draw(end, arrow1, arrowColor);
	line_->Draw(end, arrow2, arrowColor);

	// 範囲の端も薄く表示する
	Vector3 minDir = RotateVectorZ(baseDir, -maxAngleRad).Normalize();
	Vector3 maxDir = RotateVectorZ(baseDir, maxAngleRad).Normalize();

	Vector4 rangeColor = { 0.0f, 1.0f, 1.0f, 0.4f };

	line_->Draw(start, start + minDir * 0.8f, rangeColor);
	line_->Draw(start, start + maxDir * 0.8f, rangeColor);

}

Vector3 Player::RotateVectorZ(const Vector3& v, float rad)
{
	const float c = std::cosf(rad);
	const float s = std::sinf(rad);

	Vector3 result{};
	result.x = v.x * c - v.y * s;
	result.y = v.x * s + v.y * c;
	result.z = v.z;

	return result;
}

Vector3 Player::GetFacingDirFromCollisionInfo(const CollisionMapInfo& info)
{
	// 壁と床/天井に同時ヒットした場合は、めり込み量が大きい方を優先
	if (info.hitWall && (info.ceiling || info.landing)) {

		if (info.penX >= info.penY) {
			if (info.normal.x > 0.0f) {
				return { 1.0f, 0.0f, 0.0f };
			}
			if (info.normal.x < 0.0f) {
				return { -1.0f, 0.0f, 0.0f };
			}
		}
		else {
			if (info.normal.y > 0.0f) {
				return { 0.0f, 1.0f, 0.0f };
			}
			if (info.normal.y < 0.0f) {
				return { 0.0f, -1.0f, 0.0f };
			}
		}
	}

	// 壁
	if (info.hitWall) {
		if (info.normal.x > 0.0f) {
			return { 1.0f, 0.0f, 0.0f };
		}
		if (info.normal.x < 0.0f) {
			return { -1.0f, 0.0f, 0.0f };
		}
	}

	// 床 or 天井
	if (info.ceiling || info.landing) {
		if (info.normal.y > 0.0f) {
			return { 0.0f, 1.0f, 0.0f };
		}
		if (info.normal.y < 0.0f) {
			return { 0.0f, -1.0f, 0.0f };
		}
	}

	// 保険
	return { 1.0f, 0.0f, 0.0f };
}

void Player::ReflectVelocity(Vector3& v, const Vector3& normal)
{
	Vector3 n = MyMath::Normalize(normal);
	if (n.x == 0 && n.y == 0 && n.z == 0) return;

	const float vn = MyMath::Dot(v, n);
	if (vn >= 0.0f) return; // 面から離れてるなら反射しない

	v = v - n * (2.0f * vn);
}

Vector3 Player::MakeShotVelocity()
{
	// 仰角をラジアンに変換してローカル方向を作成（ローカル座標系: +Z 前方, +Y 上）
	const float rad = cannonAngleDeg_ * (3.14159265f / 180.0f);
	const float spd = parameter_.kLimitRunSpeed;
	// ローカル→ワールド変換（回転の影響を受ける）
	Vector3 localDir{};
	localDir.y = std::sinf(rad);
	localDir.z = std::cosf(rad);
	localDir.x = 0.0f;
	// ワールド変換（回転を考慮）
	Vector3 worldDir = MyMath::TransformNormal(
		localDir,
		object3D_->GetWorldMatrix()
	);
	// 正規化して速度を掛ける
	return worldDir.Normalize() * spd;
}

void Player::DrawGhost()
{

	if (!isGhostVisible_) return;
	if (!ghostObject_) return;

	ghostObject_->SetTransform(ghostTransform_);
	ghostObject_->Update();
	ghostObject_->Draw();

}

void Player::SetGhostPreview(const Vector3& landingPos, const Vector3& nextFacingDir)
{
	if (!ghostObject_) return;

	isGhostVisible_ = true;

	ghostTransform_ = object3D_->GetTransform();

	// 着地点に置く
	ghostTransform_.translate = landingPos;

	// 本体と完全に重なるとチラつく場合があるので少し手前/奥にずらす
	ghostTransform_.translate.z -= 0.08f;

	// 向き設定
	ghostTransform_.rotate = GetGhostRotateFromFacingDir(nextFacingDir);

	// サイズは本体と同じ
	ghostTransform_.scale = { 0.5f, 0.5f, 0.5f };

}

Vector3 Player::GetGhostRotateFromFacingDir(const Vector3& facingDir)
{
	Vector3 rotate{ 0.0f, 0.0f, 0.0f };

	// 右向き
	if (std::abs(facingDir.x) > std::abs(facingDir.y)) {
		if (facingDir.x >= 0.0f) {
			rotate.y = std::numbers::pi_v<float> / 2.0f;
			rotate.z = 0.0f;
		}
		// 左向き
		else {
			rotate.y = std::numbers::pi_v<float> *3.0f / 2.0f;
			rotate.z = 0.0f;
		}
	}
	else {
		// 上向き
		if (facingDir.y >= 0.0f) {
			rotate.y = std::numbers::pi_v<float> / 2.0f;
			rotate.z = std::numbers::pi_v<float> / 2.0f;
		}
		// 下向き
		else {
			rotate.y = std::numbers::pi_v<float> / 2.0f;
			rotate.z = -std::numbers::pi_v<float> / 2.0f;
		}
	}

	return rotate;
}

void Player::PlayerDirection(const CollisionMapInfo& info) {
	LRTBDirection targetDir = direction_;

	if (info.hasNormal) {

		if (info.normal.x < 0.0f) {
			targetDir = LRTBDirection::kLeft;
		}
		else if (info.normal.x > 0.0f) {
			targetDir = LRTBDirection::kRight;
		}

		if (info.normal.y > 0.0f) {
			targetDir = LRTBDirection::kTop;
		}
		else if (info.normal.y < 0.0f) {
			targetDir = LRTBDirection::kBottom;
		}
	}

	if (targetDir != direction_) {
		direction_ = targetDir;
		turnTimer_ = parameter_.kTimeTurn;
	}

	EulerTransform tr = object3D_->GetTransform();

	// 回転角決定
	float targetY = tr.rotate.y;
	float targetZ = tr.rotate.z;

	// 方向ごとの目標角度
	switch (direction_) {

	case LRTBDirection::kRight:
		targetY = std::numbers::pi_v<float> / 2.0f;
		targetZ = 0.0f;
		break;

	case LRTBDirection::kLeft:
		targetY = std::numbers::pi_v<float> *3.0f / 2.0f;
		targetZ = 0.0f;
		break;

	case LRTBDirection::kTop:
		targetY = std::numbers::pi_v<float> / 2.0f;
		targetZ = std::numbers::pi_v<float> / 2.0f;
		break;

	case LRTBDirection::kBottom:
		targetY = std::numbers::pi_v<float> / 2.0f;
		targetZ = -std::numbers::pi_v<float> / 2.0f;
		break;
	}

	// 補間
	if (turnTimer_ > 0.0f) {

		const float dt = 1.0f / 30.0f;
		turnTimer_ -= dt;

		float t = 1.0f - (turnTimer_ / parameter_.kTimeTurn);
		t = std::clamp(t, 0.0f, 1.0f);
		// ライブラリの関数を使う
		float eased = Easing::EaseOutSine(t);

		tr = object3D_->GetTransform();

		tr.rotate.y += (targetY - tr.rotate.y) * eased;
		tr.rotate.z += (targetZ - tr.rotate.z) * eased;
		tr.rotate.x = shotAnimationRotate_.x;

		object3D_->SetRotate(tr.rotate);

		if (turnTimer_ <= 0.0f) {
			turnTimer_ = 0.0f;

			tr.rotate.y = targetY;
			tr.rotate.z = targetZ;
			tr.rotate.x = shotAnimationRotate_.x;
			object3D_->SetRotate(tr.rotate);
		}
	}

	// turnTimer_ が 0 の時も発射アニメ回転を反映する
	if (turnTimer_ <= 0.0f) {
		tr.rotate.y = targetY;
		tr.rotate.z = targetZ;
		tr.rotate.x = shotAnimationRotate_.x;

		object3D_->SetRotate(tr.rotate);
	}


}

void Player::PlayerDeathTerms()
{

	// 落下による死亡判定
	if (object3D_->GetTransform().translate.y < parameter_.deathHeight.min
		|| object3D_->GetTransform().translate.y>parameter_.deathHeight.max
		|| object3D_->GetTransform().translate.x < parameter_.deathWidth.min
		|| object3D_->GetTransform().translate.x > parameter_.deathWidth.max)
	{
		isDead_ = true;
	}

}

void Player::MapCollision(CollisionMapInfo& info) {

	const Vector3 position = object3D_->GetTransform().translate;
	MapCollisionAt(position, info, true);
}

void Player::MapCollisionAt(const Vector3& position, CollisionMapInfo& info, bool enableGoal)
{
	Vector3 base = position;

	// --------------------
	// X解く
	// --------------------
	CollisionMapInfo xInfo{};
	xInfo.move = { info.move.x, 0.0f, 0.0f };
	xInfo.normal = { 0,0,0 };
	xInfo.hasNormal = false;
	xInfo.penX = 0.0f;
	xInfo.penY = 0.0f;

	CollisionMapInfoDirection(
		base, xInfo, CollisionType::Right,
		{ kRightTop, kRightBottom },
		Vector3(parameter_.kCollisionEpsilon, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x > 0; },
		enableGoal
	);
	CollisionMapInfoDirection(
		base, xInfo, CollisionType::Left,
		{ kLeftTop, kLeftBottom },
		Vector3(-parameter_.kCollisionEpsilon, 0, 0),
		[](const CollisionMapInfo& i) { return i.move.x < 0; },
		enableGoal
	);

	// X反映
	base.x += xInfo.move.x;
	info.move.x = xInfo.move.x;

	if (xInfo.hasNormal) {
		info.normal = info.normal + xInfo.normal;
		info.hasNormal = true;
	}
	info.hitWall = info.hitWall || xInfo.hitWall;
	info.penX = std::max(info.penX, xInfo.penX);
	// ブロック破壊情報反映
	if (xInfo.hasBreakBlock) {
		info.hasBreakBlock = true;
		info.breakBlockX = xInfo.breakBlockX;
		info.breakBlockY = xInfo.breakBlockY;

	}
	if (xInfo.hitDamageBlock) {
		info.hitDamageBlock = true;
	}

	// --------------------
	// Y（X反映後の位置で）
	// --------------------
	CollisionMapInfo yInfo{};
	yInfo.move = { 0.0f, info.move.y, 0.0f };
	yInfo.normal = { 0,0,0 };
	yInfo.hasNormal = false;
	yInfo.penX = 0.0f;
	yInfo.penY = 0.0f;

	CollisionMapInfoDirection(
		base, yInfo, CollisionType::Top,
		{ kLeftTop, kRightTop },
		Vector3(0, parameter_.kCollisionEpsilon, 0),
		[](const CollisionMapInfo& i) { return i.move.y > 0; },
		enableGoal
	);
	CollisionMapInfoDirection(
		base, yInfo, CollisionType::Bottom,
		{ kLeftBottom, kRightBottom },
		Vector3(0, -parameter_.kCollisionEpsilon, 0),
		[](const CollisionMapInfo& i) { return i.move.y < 0; },
		enableGoal
	);

	// Y反映
	base.y += yInfo.move.y;
	info.move.y = yInfo.move.y;

	if (yInfo.hasNormal) {
		info.normal = info.normal + yInfo.normal;
		info.hasNormal = true;
	}
	info.ceiling = info.ceiling || yInfo.ceiling;
	info.landing = info.landing || yInfo.landing;
	info.penY = std::max(info.penY, yInfo.penY);
	// ブロック破壊情報反映
	if (yInfo.hasBreakBlock && !info.hasBreakBlock) {
		info.hasBreakBlock = true;
		info.breakBlockX = yInfo.breakBlockX;
		info.breakBlockY = yInfo.breakBlockY;

	}

	if (yInfo.hitDamageBlock) {
		info.hitDamageBlock = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

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
		}
		else {

			Logger::Log("hard\n");

		}
	}
}

void Player::LandingCollisionMove(const CollisionMapInfo& info) {


	if (info.landing) {

		if (playerState_ == PlayerState::sticky) {

			velocity_.x = 0.0f;
			velocity_.y = 0.0f;
			velocity_.z = 0.0f;
		}
		else {

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
		}
		else {

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

void Player::PlayerShotAnimation()
{
	if (!isShotAnimation_) {
		shotAnimationRotate_ = { 0,0,0 };
		return;
	}

	const float dt = 1.0f / 60.0f;

	shotAnimationTimer_ += dt;

	float t = shotAnimationTimer_ / 0.4f;

	if (t >= 1.0f) {
		t = 1.0f;
		isShotAnimation_ = false;
	}

	// スピン
	shotAnimationRotate_.x = std::numbers::pi_v<float> *2.0f * t;



}

bool Player::TryPortalWarp(Vector3& position, Vector3& velocity, bool useCooldown)
{
	if (!mapChipField_) return false;

	if (useCooldown && portalCooldown_ > 0) {
		return false;
	}

	IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(position);
	MapChipType type = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	if (type != MapChipType::Portal) {
		return false;
	}

	PortalInfo inPortal;
	if (!mapChipField_->TryGetPortal(index.xIndex, index.yIndex, inPortal)) {
		return false;
	}

	PortalInfo outPortal;
	if (!mapChipField_->TryGetPairPortal(inPortal, outPortal)) {
		return false;
	}

	velocity = RotateVelocityByPortal(
		velocity,
		inPortal.dir,
		outPortal.dir
	);

	Vector3 exitPos = mapChipField_->GetMapChipPositionByIndex(outPortal.x, outPortal.y);

	const float pushOut = 0.8f;
	Vector3 outDir = outPortal.dir.Normalize();
	position = exitPos + outDir * pushOut;

	if (useCooldown) {
		portalCooldown_ = 10;
	}

	return true;
}

Vector3 Player::RotateVelocityByPortal(const Vector3& velocity, const Vector3& inDir, const Vector3& outDir)
{
	float inAngle = std::atan2(inDir.y, inDir.x);
	float outAngle = std::atan2(outDir.y, outDir.x);

	float delta = outAngle - inAngle;

	float c = std::cos(delta);
	float s = std::sin(delta);

	Vector3 result{};
	result.x = velocity.x * c - velocity.y * s;
	result.y = velocity.x * s + velocity.y * c;
	result.z = velocity.z;

	return result;
}

void Player::StartCameraShake(float power, float duration)
{
	cameraShakePower_ = power;
	cameraShakeDuration_ = duration;
	cameraShakeTimer_ = duration;
}

void Player::UpdateCameraShake()
{
	Camera* camera = CameraManager::GetInstance()->GetActiveCamera();
	if (!camera) return;

	const float dt = 1.0f / 60.0f;

	// 前フレームで足したシェイク分を一回戻す
	Vector3 cameraPos = camera->GetTransform().translate;
	cameraPos = cameraPos - cameraShakePrevOffset_;
	cameraShakePrevOffset_ = { 0.0f, 0.0f, 0.0f };

	if (cameraShakeTimer_ <= 0.0f || cameraShakeDuration_ <= 0.0f) {
		camera->SetTranslate(cameraPos);
		return;
	}

	cameraShakeTimer_ -= dt;

	float t = cameraShakeTimer_ / cameraShakeDuration_;
	t = std::clamp(t, 0.0f, 1.0f);

	const float power = cameraShakePower_ * t;

	Vector3 offset{};
	offset.x = std::sinf(cameraShakeTimer_ * 90.0f) * power;
	offset.y = std::cosf(cameraShakeTimer_ * 110.0f) * power;
	offset.z = 0.0f;

	cameraShakePrevOffset_ = offset;
	camera->SetTranslate(cameraPos + offset);

	if (cameraShakeTimer_ <= 0.0f) {
		cameraShakeTimer_ = 0.0f;
		cameraShakePrevOffset_ = { 0.0f, 0.0f, 0.0f };
		camera->SetTranslate(cameraPos);
	}
}

void Player::PlayerParticle()
{
	// 地面にいて、左右どちらかに動いているときだけ排気ガス
	bool isMoving = true;

	const float dt = 1.0f / 60.0f;

	if (playerState_ == PlayerState::hard && isBarrierActive_) {
		ParticleManager::GetInstance()->EmitFollowOne("barriering", object3D_->GetTransform());
	}
	else {
		ParticleManager::GetInstance()->StopFollow("barriering");
	}

	if (isMoving) {
		exhaustTimer_ += dt;

		// 一定間隔ごとにだけ煙を出す
		if (exhaustTimer_ >= kExhaustInterval) {
			exhaustTimer_ = 0.0f;

			EulerTransform smokeTransform{};
			smokeTransform.translate = object3D_->GetTransform().translate;

			// 進行方向のちょい後ろに出すと“排気”感が出る
			if (direction_ == LRTBDirection::kRight) {
				smokeTransform.translate.x -= 0.15f;
			}
			else {
				smokeTransform.translate.x += 0.15f;
			}

			// 1回に2粒くらい
			exhaustEmitter_->SetPosition(smokeTransform.translate);
			exhaustEmitter_->Emit();


		}

	}
	else {
		// 止まったらタイマーリセット
		exhaustTimer_ = 0.0f;
	}
}

bool Player::IsHittableBlock(MapChipType type)
{
	switch (type) {
	case MapChipType::Block:
	case MapChipType::UnbreakableBlock:
	case MapChipType::damageBlock:
		return true;
	default:
		return false;
	}
}

bool Player::CheckCollisionPoints(const Vector3& basePos, const std::array<Vector3, 2>& posList, CollisionType type, CollisionMapInfo& info, bool enableGoal)
{
	const Vector3 movedCenter = basePos + info.move;

	bool hit = false;

	bool hasBest = false;
	Rect bestRect{};
	float bestPen = -1.0f;

	for (const auto& p : posList) {
		IndexSet idx = mapChipField_->GetMapChipIndexSetByPosition(p);
		MapChipType chip = mapChipField_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex);

		Rect r = mapChipField_->GetRectByIndex(idx.xIndex, idx.yIndex);

		float pen = 0.0f;
		switch (type) {
		case CollisionType::Right:
			pen = (movedCenter.x + parameter_.kWidth * 0.5f) - r.left;
			break;
		case CollisionType::Left:
			pen = r.right - (movedCenter.x - parameter_.kWidth * 0.5f);
			break;
		case CollisionType::Top:
			pen = (movedCenter.y + parameter_.kHeight * 0.5f) - r.bottom;
			break;
		case CollisionType::Bottom:
			pen = r.top - (movedCenter.y - parameter_.kHeight * 0.5f);
			break;
		}

		if (chip == MapChipType::Goal) {
			if (enableGoal) {
				if (pen > -parameter_.kCollisionEpsilon) { // 少し甘くする
					goal_ = true;
				}
			}
			continue;
		}

		if (chip == MapChipType::damageBlock) {
			if (pen > 0.0f) {
				info.hitDamageBlock = true;

			}
		}

		if (chip == MapChipType::Block) {
			if (enableGoal && playerState_ == PlayerState::hard) {
				if (pen > 0.0f) {
					if (!info.hasBreakBlock) {
						info.hasBreakBlock = true;
						info.breakBlockX = idx.xIndex;
						info.breakBlockY = idx.yIndex;
					}
				}
			}
		}

		// 壁になるブロック以外は無視
		if (!IsHittableBlock(chip)) continue;

		hit = true;

		// めり込みがある時だけ採用
		if (pen > 0.0f) {
			if (type == CollisionType::Right || type == CollisionType::Left) {
				info.penX = std::max(info.penX, pen);
			}
			else {
				info.penY = std::max(info.penY, pen);
			}

			if (!hasBest || pen > bestPen) {
				hasBest = true;
				bestPen = pen;
				bestRect = r;
			}
		}
	}

	if (!hit || !hasBest) return false;

	// 法線加算
	Vector3 n = NormalFromType(type);
	info.normal = info.normal + n;
	info.hasNormal = true;

	// 押し戻し（bestRect 基準）
	switch (type) {
	case CollisionType::Top:
		// 天井：ブロックの bottom に合わせる
		info.move.y = bestRect.bottom - basePos.y - (parameter_.kHeight * 0.5f + parameter_.kBlank);
		info.ceiling = true;
		break;

	case CollisionType::Bottom:
		// 床：ブロックの top に合わせる
		info.move.y = bestRect.top - basePos.y + (parameter_.kHeight * 0.5f + parameter_.kBlank);
		info.landing = true;
		break;

	case CollisionType::Right:
		// 右壁：ブロックの left に合わせる
		info.move.x = bestRect.left - basePos.x - (parameter_.kWidth * 0.5f + parameter_.kBlank);
		info.hitWall = true;
		break;

	case CollisionType::Left:
		// 左壁：ブロックの right に合わせる
		info.move.x = bestRect.right - basePos.x + (parameter_.kWidth * 0.5f + parameter_.kBlank);
		info.hitWall = true;
		break;
	}

	return true;
}


void Player::CollisionMapInfoDirection(
	const Vector3& basePos,
	CollisionMapInfo& info,
	CollisionType dir,
	const std::array<Corner, 2>& checkCorners,
	const Vector3& offset,
	std::function<bool(const CollisionMapInfo&)> moveCondition,
	bool enableGoal
)
{
	if (!moveCondition(info)) return;

	Vector3 position = basePos + info.move;

	std::array<Vector3, 2> points = {
		CornerPosition(position, checkCorners[0]) + offset,
		CornerPosition(position, checkCorners[1]) + offset
	};

	CheckCollisionPoints(basePos, points, dir, info, enableGoal);
}
