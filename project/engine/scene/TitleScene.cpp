#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include <CameraManager.h>
#include <ModelManager.h>
#include <imgui.h>

void TitleScene::Initialize() {
	// モデル読み込み
	ModelManager::GetInstans()->LoadModel("player.obj");
	ModelManager::GetInstans()->LoadModel("title.obj"); // タイトルモデル

	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	titleSprite = new Sprite();
	titleSprite->Initialize(SpriteCommon::GetInstance(),"Resources/space.png");
	titleSprite->SetPosition({ 490.0f,600.0f });
	titleSprite->SetSize({ 300.0f,100.0f });




	// プレイヤー生成
	object3D_ = new Object3D();
	object3D_->Initialize(Object3DCommon::GetInstance());
	object3D_->SetModel("player.obj");
	object3D_->SetScale(Vector3{ 0.25f, 0.25f, 0.25f });
	object3D_->SetLighting(true);
	object3D_->SetDirectionalLightEnable(true);
	object3D_->SetDirectionalLightDirection({ -1.3f, -1.82f, -4.77f });
	object3D_->SetRotate({ 0, 3.1f, 0 });

	// タイトル生成
	titleObj_ = new Object3D();
	titleObj_->Initialize(Object3DCommon::GetInstance());
	titleObj_->SetModel("title.obj");
	titleObj_->SetLighting(false);
	titleObj_->SetRotate({ 0, 3.1f, 0 });


	{
		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		t.scale = { titleModelScale_, titleModelScale_, titleModelScale_ };
		titleObj_->SetTransform(t);
	}

	// カメラ
	camera = new Camera();
	camera->SetRotate({ 0, 0, 0 });
	camera->SetTranslate({ 0, 0, -10 });
	CameraManager::GetInstance()->AddCamera("maincam", camera);
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	state_ = TitleAnimState::IntroRun;
	ropeAttached_ = false;
}

void TitleScene::Finalize() {
	delete titleObj_;
	delete object3D_;
	delete camera;
	delete titleSprite;
	
	CameraManager::GetInstance()->RemoveCamera("maincam");
}

void TitleScene::Update() {
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	fadeManager_.Update();
	
	object3D_->Update();
	titleObj_->Update();
	titleSprite->Update();

	if (state_ == TitleAnimState::IntroRun ||
		state_ == TitleAnimState::DragTitle ||
		state_ == TitleAnimState::Settle) {

		playerX_ += playerSpeed_;
		if (playerX_ > 12.0f) playerX_ = 12.0f;

		// ← ここ変更：移動方向で向きを切替
		float rotateY = kYawFront;             // デフォは正面
		if (playerSpeed_ > 0.0f) rotateY = kYawRight; // 右へ移動中
		if (playerSpeed_ < 0.0f) rotateY = kYawLeft;  // 左へ移動中

		EulerTransform tr = object3D_->GetTransform();
		tr.translate.x = playerX_;
		tr.translate.y = playerY_;
		tr.rotate.y = rotateY;
		object3D_->SetTransform(tr);
	}


	// フックトリガー
	if (!ropeAttached_ && playerX_ >= hookAtX_) {
		ropeAttached_ = true;
		state_ = TitleAnimState::HookTitle;
	}

	const EulerTransform camTr = CameraManager::GetInstance()->GetActiveCamera()->GetTransform();
	float targetX = camTr.translate.x;

	// 状態ごとの処理
	switch (state_) {
	case TitleAnimState::IntroRun:
		break;

	case TitleAnimState::HookTitle:
		state_ = TitleAnimState::DragTitle;
		break;

	case TitleAnimState::DragTitle: {
		float ax = (targetX - springX_) * springK_;
		springV_ = springV_ * springD_ + ax;
		springX_ += springV_;

		titlePos3_.x = springX_;
		titlePos3_.y = camTr.translate.y + titleYOffset_;
		titlePos3_.z = 0.0f;

		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		titleObj_->SetTransform(t);

		if (std::abs(targetX - springX_) < 0.5f && std::abs(springV_) < 0.05f)
			state_ = TitleAnimState::Settle;
	} break;

	case TitleAnimState::Settle: {
		float ax = (targetX - springX_) * springK_;
		springV_ = springV_ * springD_ + ax;
		springX_ += springV_;
		titlePos3_.x = springX_;
		titlePos3_.y = camTr.translate.y + titleYOffset_;

		EulerTransform t = titleObj_->GetTransform();
		t.translate = titlePos3_;
		titleObj_->SetTransform(t);

		if (std::abs(targetX - springX_) < 0.1f && std::abs(springV_) < 0.02f)
			state_ = TitleAnimState::ReturnUnderTitle;
	} break;

	case TitleAnimState::ReturnUnderTitle: {
		const EulerTransform provisionalcamTr =
			CameraManager::GetInstance()->GetActiveCamera()->GetTransform();
		const float provisionaltargetX = provisionalcamTr.translate.x;

		if (std::abs(playerX_ - provisionaltargetX) > 0.001f) {
			float dir = (playerX_ < provisionaltargetX) ? 1.0f : -1.0f; // 右へ=+1, 左へ=-1
			playerX_ += dir * playerReturnSpeed_;
			if ((dir > 0 && playerX_ > provisionaltargetX) || (dir < 0 && playerX_ < provisionaltargetX)) {
				playerX_ = provisionaltargetX;
			}

			// ← ここ変更：移動方向に応じて左右
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
			tr.rotate.y = kYawFront;   // ← 3.1（正面）
			object3D_->SetTransform(tr);
			state_ = TitleAnimState::Idle;
		}
	} break;

	case TitleAnimState::Idle: {
		if (!loop_) break;                 // ループしないなら何もしない
		idleTimer_ += 1.0f / 60.0f;        // 固定フレーム想定
		if (idleTimer_ > loopWaitSec_) {
			ResetTitleAnimation();         // ← ここで1周分をリセット
			// ここで state_ は IntroRun に戻るので、次フレームからまた走り出す
		}
	} break;

	}

	// スペースキーでシーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		fadeManager_.StartFadeOut();
	}
	if (fadeManager_.IsFadeOutFinished()) {
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
	ImguiDraw();
}

void TitleScene::Draw() {
	Object3DCommon::GetInstance()->CommonDraw();
	if (titleObj_) titleObj_->Draw();
	if (object3D_) object3D_->Draw();

	SpriteCommon::GetInstance()->CommonDraw();
	titleSprite->Draw();
	fadeManager_.Draw();
}

void TitleScene::ResetTitleAnimation()
{

	// 状態
	state_ = TitleAnimState::IntroRun;
	ropeAttached_ = false;
	idleTimer_ = 0.0f;

	// パラメータ初期化（あなたの初期値に合わせる）
	playerX_ = -8.0f;
	playerY_ = -1.0f;

	springX_ = -12.0f;
	springV_ = 0.0f;

	// タイトルを左外へ戻す
	if (titleObj_) {
		EulerTransform t = titleObj_->GetTransform();
		t.translate = { -12.0f, 0.0f, 0.0f };
		titleObj_->SetTransform(t);
	}
	// プレイヤーも左へ戻し、正面を向く（3.1 が正面ならそのまま）
	if (object3D_) {
		EulerTransform p = object3D_->GetTransform();
		p.translate = { playerX_, playerY_, 0.0f };
		p.rotate.y = 3.1f;
		object3D_->SetTransform(p);
	}
}

void TitleScene::ImguiDraw()
{
#ifdef _DEBUG

	ImGui::Begin("TitleScene Debug");


	//タイトルスプライト
	Vector2 titlePos2_ = titleSprite->GetPosition();
	ImGui::DragFloat2("titlePos2_", &titlePos2_.x, 0.1f);
	titleSprite->SetPosition(titlePos2_);
	//サイズ
	Vector2 titleScale2_ = titleSprite->GetSize();
	ImGui::DragFloat2("titleScale2_", &titleScale2_.x, 0.1f);
	titleSprite->SetSize(titleScale2_);	

	


	ImGui::End();
#endif // _DEBUG
}

