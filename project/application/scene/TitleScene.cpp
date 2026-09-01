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
#include <ParticleMnager.h>
#include <ChargeBehabiaor.h>
#include <UIeditor.h>

// 初期化：モデル読み込み、オブジェクト生成、フェード開始など
void TitleScene::Initialize() {
	// モデル読み込み（ModelManager は単一インスタンス）
	ModelManager::GetInstance()->LoadModel("player");
	ModelManager::GetInstance()->LoadModel("title1"); // タイトルモデル

	// フェードの初期化（白）
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	// 2Dゲート遷移（IN）
	gate_ = std::make_unique<GateInOut>();
	gate_->Initialize(SpriteCommon::GetInstance(),
		"Resources/block.png", "Resources/block.png");
	gate_->SetScreenSize(1280.0f, 720.0f); // WinAppから取れるならそれで
	gateOutRequested_ = false;
	fadeOutRequested_ = false;

	UIeditor::GetInstance()->SetScene("Title");

	// プレイヤー Object3D を生成してセットアップ
	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player");
	object3D_->SetScale(Vector3{ 0.75f, 0.75f, 0.75f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f, -1.82f, -4.77f });
	object3D_->SetRotate({ 0, kYawFront, 0 });

	// タイトルモデル生成・配置
	titleObj_ = std::make_unique<Object3D>();
	titleObj_->Initialize(Object3DCommon::GetInstance());
	titleObj_->SetModel("title1");
	titleObj_->SetLighting(false);
	titleObj_->SetRotate({ 0, kYawFront, 0 });

	{
		// モデルのトランスフォーム初期化
		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		t.scale = { titleModelScaleW_, titleModelScaleH_, 1.0f };
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

	
	titleObj_.reset();
	object3D_.reset();
	camera.reset();
}

// 更新処理：カメラ更新、アニメ状態遷移、入力検出、フェード遷移
void TitleScene::Update() {
	// カメラ更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();

	float dt = 1.0f / 60.0f;
	if (gate_) gate_->Update(dt);
	fadeManager_.Update();

	// オブジェクト更新（存在チェック）
	if (object3D_) object3D_->Update();
	if (titleObj_) titleObj_->Update();
	//if (titleSprite_) titleSprite_->Update();

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

			lrDirection_ = (dir > 0) ? LRDirecion::kright : LRDirecion::kLeft;


			EulerTransform tr = object3D_->GetTransform();
			tr.translate.x = playerX_;
			tr.translate.y = playerTargetUnderTitleY_;
			tr.rotate.y = rotateY;
			object3D_->SetTransform(tr);
		}
		else {
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

		idleTimer_ += 1.0f / 30.0f;

		if (!loopFadePlaying_ && idleTimer_ > loopWaitSec_) {
			// まず暗転(or白転)開始して“リセットの瞬間”を隠す
			loopFadePlaying_ = true;
			loopDoReset_ = true;
			fadeManager_.StartFadeOut(0.5f);
		}

		// フェードアウトが終わった瞬間にリセットを実行
		if (loopFadePlaying_ && loopDoReset_ && fadeManager_.IsFadeOutFinished()) {
			ResetTitleAnimation();           // 瞬間移動しても見えない
			fadeManager_.StartFadeIn();      // そのままフェードインで復帰
		}
	
	} break;
	}


	if (!gateOutRequested_ &&
		(!gate_ || !gate_->IsPlaying()) &&
		Input::GetInstance()->TriggerMouse(0))
	{
		UIeditor::GetInstance()->PlayPressAnimation("Title", "click");
		gateOutRequested_ = true;
		if (gate_) gate_->StartOut(0.6f);
	}

	if (gateOutRequested_ && !fadeOutRequested_ && gate_ && gate_->IsFinished()) {
		fadeOutRequested_ = true;

		gate_->HoldClosed(true);

		fadeManager_.StartFadeOut();
	}

	if (fadeOutRequested_ && fadeManager_.IsFadeOutFinished()) {

		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}

	ImguiDraw();
	PlayerParticle();
}

// 描画
void TitleScene::Draw() {
	Object3DCommon::GetInstance()->CommonDraw();

	if (titleObj_) titleObj_->Draw();
	if (object3D_) object3D_->Draw();

	ParticleManager::GetInstance()->Draw();

	SpriteCommon::GetInstance()->CommonDraw();
	// ControlGuide をここで描画すると UI レイヤーで最前面に来ます
	UIeditor::GetInstance()->Render();

	//if (titleSprite_) titleSprite_->Draw();
	if (gate_) gate_->Draw2D();
	fadeManager_.Draw();
}

void TitleScene::PlayerParticle()
{


	const float dt = 1.0f / 60.0f;


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
		ParticleManager::GetInstance()->Emit("dash_smoke", smokeTransform);
	}

}

// アンカー／初期化を揃える（1周分リセット）
void TitleScene::ResetTitleAnimation()
{

	// フェード用フラグも戻す
	loopFadePlaying_ = false;
	loopDoReset_ = false;

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

	if (titleObj_) {
		ImGui::SeparatorText("Title Object");

		Vector2 titleScale={ titleModelScaleW_, titleModelScaleH_ };
		ImGui::DragFloat("Title ScaleW",&titleModelScaleW_, 0.01f, 0.01f, 20.0f);
		ImGui::DragFloat("Title ScaleH", &titleModelScaleH_, 0.01f, 0.01f, 20.0f);

		EulerTransform t = titleObj_->GetTransform();
		t.scale = { titleModelScaleW_, titleModelScaleH_,1.0};
		titleObj_->SetTransform(t);
	}

	ImGui::End();
#endif // _DEBUG
}

