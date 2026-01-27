#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include <CameraManager.h>
#include <ModelManager.h>
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include <memory>

// 初期化：モデル読み込み、オブジェクト生成、フェード開始など
void TitleScene::Initialize() {
	// モデル読み込み（ModelManager は単一インスタンス）
	ModelManager::GetInstance()->LoadModel("player.obj");
	ModelManager::GetInstance()->LoadModel("title.obj"); // タイトルモデル

	// フェードの初期化（白）
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	// タイトルスプライトを作成して初期パラメータ設定
	titleSprite_ = std::make_unique<Sprite>();
	titleSprite_->Initialize(SpriteCommon::GetInstance(), "Resources/space.png");
	titleSprite_->SetPosition({ kTitleSpritePosX, kTitleSpritePosY });
	titleSprite_->SetSize({ kTitleSpriteW, kTitleSpriteH });

	// プレイヤー Object3D を生成してセットアップ
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player.obj");
	object3D_->SetScale(Vector3{ 0.25f, 0.25f, 0.25f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f, -1.82f, -4.77f });
	object3D_->SetRotate({ 0, kYawFront, 0 });

	// タイトルモデル生成・配置
	titleObj_ = std::make_unique<Object3D>();
	titleObj_->Initialize(Object3DCommon::GetInstance());
	titleObj_->SetModel("title.obj");
	titleObj_->SetLighting(false);
	titleObj_->SetRotate({ 0, kYawFront, 0 });

	{
		// モデルのトランスフォーム初期化
		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		t.scale = { titleModelScale_, titleModelScale_, titleModelScale_ };
		titleObj_->SetTransform(t);
	}

	// カメラを生成して CameraManager に登録
	camera = std::make_unique<Camera>();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera.get());
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	// 状態初期化
	state_ = TitleAnimState::IntroRun;
	ropeAttached_ = false;
}

// 終了処理：CameraManager から削除し、unique_ptr が破棄してメモリ解放
void TitleScene::Finalize() {
	CameraManager::GetInstance()->RemoveCamera("maincam");

	// unique_ptr の破棄に任せる（明示的な delete は不要）
	titleSprite_.reset();
	titleObj_.reset();
	object3D_.reset();
	camera.reset();
}

// 更新処理：カメラ更新、アニメ状態遷移、入力検出、フェード遷移
void TitleScene::Update() {
	// カメラ更新・フェード更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();

	// オブジェクト更新（存在チェック）
	if (object3D_) object3D_->Update();
	if (titleObj_) titleObj_->Update();
	if (titleSprite_) titleSprite_->Update();

	// プレイヤー走行アニメーション（Intro / Drag / Settle 時）
	if (state_ == TitleAnimState::IntroRun ||
		state_ == TitleAnimState::DragTitle ||
		state_ == TitleAnimState::Settle) {

		// 移動（フレーム固定Δ想定）
		playerX_ += playerSpeed_;
		if (playerX_ > kPlayerMaxX) playerX_ = kPlayerMaxX;

		// 向き選定：正面/右/左
		float rotateY = kYawFront;             // デフォは正面
		if (playerSpeed_ > 0.0f) rotateY = kYawRight; // 右へ移動中
		if (playerSpeed_ < 0.0f) rotateY = kYawLeft;  // 左へ移動中

		// Transform を適用
		EulerTransform tr = object3D_->GetTransform();
		tr.translate.x = playerX_;
		tr.translate.y = playerY_;
		tr.rotate.y = rotateY;
		object3D_->SetTransform(tr);
	}

	// フックが掛かる位置に到達したらフラグを立てる
	if (!ropeAttached_ && playerX_ >= hookAtX_) {
		ropeAttached_ = true;
		state_ = TitleAnimState::HookTitle;
	}

	// カメラ位置に追従する目標 X（現在のカメラ座標を参照）
	const EulerTransform camTr = CameraManager::GetInstance()->GetActiveCamera()->GetTransform();
	float targetX = camTr.translate.x;

	// 状態ごとの処理（スプリングモデルで引っ張る / 収束 / 帰還など）
	switch (state_) {
	case TitleAnimState::IntroRun:
		// Intro は単に走らせるのみ
		break;

	case TitleAnimState::HookTitle:
		// フックが掛かったら Drag に遷移（ワンショット）
		state_ = TitleAnimState::DragTitle;
		break;

	case TitleAnimState::DragTitle: {
		// スプリング運動でタイトルをカメラ位置へ近づける
		float ax = (targetX - springX_) * springK_;
		springV_ = springV_ * springD_ + ax;
		springX_ += springV_;

		titlePos3_.x = springX_;
		titlePos3_.y = camTr.translate.y + titleYOffset_;
		titlePos3_.z = 0.0f;

		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		titleObj_->SetTransform(t);

		// 近接かつ速度が小さい → 収束状態へ
		if (std::abs(targetX - springX_) < kSpringCloseDist1 && std::abs(springV_) < kSpringVelThresh1)
			state_ = TitleAnimState::Settle;
	} break;

	case TitleAnimState::Settle: {
		// 収束時は閾値を厳しくして完全停止を待つ
		float ax = (targetX - springX_) * springK_;
		springV_ = springV_ * springD_ + ax;
		springX_ += springV_;
		titlePos3_.x = springX_;
		titlePos3_.y = camTr.translate.y + titleYOffset_;

		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		titleObj_->SetTransform(t);

		if (std::abs(targetX - springX_) < kSpringCloseDist2 && std::abs(springV_) < kSpringVelThresh2)
			state_ = TitleAnimState::ReturnUnderTitle;
	} break;

	case TitleAnimState::ReturnUnderTitle: {
		// プレイヤーをタイトル下へ戻すアニメーション
		const EulerTransform provisionalcamTr =
			CameraManager::GetInstance()->GetActiveCamera()->GetTransform();
		const float provisionaltargetX = provisionalcamTr.translate.x;

		if (std::abs(playerX_ - provisionaltargetX) > kPlayerReturnEpsilon) {
			float dir = (playerX_ < provisionaltargetX) ? 1.0f : -1.0f; // 右へ=+1, 左へ=-1
			playerX_ += dir * playerReturnSpeed_;
			if ((dir > 0 && playerX_ > provisionaltargetX) || (dir < 0 && playerX_ < provisionaltargetX)) {
				playerX_ = provisionaltargetX;
			}

			float rotateY = (dir > 0) ? kYawRight : kYawLeft;

			EulerTransform tr = object3D_->GetTransform();
			tr.translate.x = playerX_;
			tr.translate.y = playerTargetUnderTitleY_;
			tr.rotate.y = rotateY;
			object3D_->SetTransform(tr);
		} else {
			// 中央に到達 → 正面で停止
			EulerTransform tr = object3D_->GetTransform();
			tr.translate.x = playerX_;
			tr.translate.y = playerTargetUnderTitleY_;
			tr.rotate.y = kYawFront;   // 正面
			object3D_->SetTransform(tr);
			state_ = TitleAnimState::Idle;
		}
	} break;

	case TitleAnimState::Idle: {
		// ループ制御：一定時間待ってリセットする
		if (!loop_) break;
		idleTimer_ += 1.0f / 60.0f;        // 固定フレーム想定
		if (idleTimer_ > loopWaitSec_) {
			ResetTitleAnimation();
			// state_ は Reset で IntroRun に戻る
		}
	} break;
	}

	// スペースキーでシーン遷移（フェード開始）
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		fadeManager_.StartFadeOut();
	}
	if (fadeManager_.IsFadeOutFinished()) {
		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}

	ImguiDraw();
}

// 描画：3D -> 2D の順で描画
void TitleScene::Draw() {
	Object3DCommon::GetInstance()->CommonDraw();
	if (titleObj_) titleObj_->Draw();
	if (object3D_) object3D_->Draw();

	SpriteCommon::GetInstance()->CommonDraw();
	if (titleSprite_) titleSprite_->Draw();
	fadeManager_.Draw();
}

// アンカー／初期化を揃える（1周分リセット）
void TitleScene::ResetTitleAnimation()
{
	// 状態とパラメータを初期化して再スタートできる状態へ戻す
	state_ = TitleAnimState::IntroRun;
	ropeAttached_ = false;
	idleTimer_ = 0.0f;

	// プレイヤー初期位置
	playerX_ = -8.0f;
	playerY_ = -1.0f;

	// スプリング初期化
	springX_ = kTitleStartX;
	springV_ = 0.0f;

	// タイトルを左外へ戻す（定数を利用）
	if (titleObj_) {
		EulerTransform t = titleObj_->GetTransform();
		t.translate = { kTitleStartX, 0.0f, 0.0f };
		titleObj_->SetTransform(t);
	}
	// プレイヤーも左へ戻し、正面を向く
	if (object3D_) {
		EulerTransform p = object3D_->GetTransform();
		p.translate = { playerX_, playerY_, 0.0f };
		p.rotate.y = kYawFront;
		object3D_->SetTransform(p);
	}
}

// デバッグ UI（ImGui）
void TitleScene::ImguiDraw()
{
#ifdef _DEBUG

	ImGui::Begin("TitleScene Debug");

	// タイトルスプライト位置/サイズ編集
	if (titleSprite_) {
		Vector2 titlePos2_ = titleSprite_->GetPosition();
		ImGui::DragFloat2("titlePos2_", &titlePos2_.x, 0.1f);
		titleSprite_->SetPosition(titlePos2_);
		Vector2 titleScale2_ = titleSprite_->GetSize();
		ImGui::DragFloat2("titleScale2_", &titleScale2_.x, 0.1f);
		titleSprite_->SetSize(titleScale2_);
	}

	ImGui::End();
#endif // _DEBUG
}

