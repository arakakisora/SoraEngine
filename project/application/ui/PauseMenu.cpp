#include "PauseMenu.h"
#include <SceneManager.h>
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG
#include <ModelManager.h>
#include <CameraManager.h>
#include <algorithm>
#include <UIeditor.h>

double easeInOutCirc(double x) {
	return (x < 0.5)
		? (1 - std::sqrt(1 - std::pow(2 * x, 2))) / 2
		: (std::sqrt(1 - std::pow(-2 * x + 2, 2)) + 1) / 2;
}

//初期化
void PauseMenu::Initialize(Object3DCommon* object3dcommon, PauseType type) {

	//カメラの生成	
	pouseCamera_ = std::make_unique<Camera>();
	pouseCamera_->SetRotate({ 0,0,0, });
	pouseCamera_->SetTranslate({1.0f,11.41f,-22.700f, });
	CameraManager::GetInstance()->AddCamera("pouseCamera", pouseCamera_.get());

	object3dcommon_ = object3dcommon;
	pauseType_ = type;

	// ベースオブジェクト準備
	ModelManager::GetInstance()->LoadModel("Pause");
	ModelManager::GetInstance()->LoadModel("return");
	ModelManager::GetInstance()->LoadModel("StageSelect");
	ModelManager::GetInstance()->LoadModel("titlemenu");

	transform.translate = { -2.6f,1.5f,0.0f };
	object = std::make_unique<Object3D>();
	object->Initialize(object3dcommon_);
	object->SetModel("Pause");
	object->SetScale({ 1.0f,1.0f,1.0f });
	input = Input::GetInstance();

	// メニュー項目をデータで構築
	menuItems_.clear();
	if (pauseType_ == PauseType::GamePlayScene) {
		menuItems_.push_back({
			"return",
			// 戻るアクション
			[this]() { isPaused_ = false; },
			1.0f,   // maxScale
			1.5f,   // selectedScale
			1.8f    // offsetY
		});
		menuItems_.push_back({
			"StageSelect",
			// ステージセレクトへ遷移
			[this]() { SceneManager::GetInstance()->ChangeScene("STAGESELECT"); },
			0.6f,   // maxScale (非選択での上限)
			0.9f,   // selectedScale (選択時)
			1.8f    // offsetY
		});
	}
	else { // StageSelectScene
		menuItems_.push_back({
			"return",
			[this]() { isPaused_ = false; },
			1.0f,
			0.9f,
			1.3f
		});
		menuItems_.push_back({
			"titlemenu",
			[this]() { SceneManager::GetInstance()->ChangeScene("TITELE"); },
			0.6f,
			0.9f,
			1.3f
		});
	}

	// TextObjects を menuItems_ に合わせて作成
	TextObjects.clear();
	for (size_t i = 0; i < menuItems_.size(); ++i) {
		std::unique_ptr<Object3D> newObject = std::make_unique<Object3D>();
		newObject->Initialize(Object3DCommon::GetInstance());
		newObject->SetModel(menuItems_[i].modelPath);
		texttransform[i] = { transform.translate };
		texttransform[i].translate.y = 0.0f;
		newObject->SetTranslate(texttransform[i].translate);
		newObject->SetScale(Vector3(0.0f, 0.0f, 0.0f));
		newObject->SetLighting(false);
		TextObjects.push_back(std::move(newObject));
	}
}

//更新
void PauseMenu::Update() {

	object->SetTranslate(transform.translate);
	object->Update();

#ifdef _DEBUG
	ImGui::Begin("Textindex");
	ImGui::Text("check: %d", textindex);
	ImGui::End();
#endif // _DEBUG

	for (std::unique_ptr<Object3D>& text : TextObjects) {
		text->SetScale(Vector3(0.0f, 0.0f, 0.0f));
		text->Update();
	}

	//遷移速度
	const float easeSpeed = 0.04f;

	if (isPaused_) {
		easeTimer_ += easeSpeed;
		if (easeTimer_ > 1.0f) easeTimer_ = 1.0f;
	}
	else {
		easeTimer_ -= easeSpeed;
		if (easeTimer_ < 0.0f) easeTimer_ = 0.0f;
	}

	// 汎用化されたポーズ描画処理（タイプ別分岐は Initialize 時に解決済み）
	if (isPaused_ || easeTimer_ > 0.0f) {
		// イージングの値（0.0〜1.0）の計算（※逆再生に対応）
		float easedValue = float(easeInOutCirc(easeTimer_));
		CameraManager::GetInstance()->SetActiveCamera("pouseCamera");

		Vector3 cameraPos = CameraManager::GetInstance()->GetActiveCamera()->GetTransform().translate;
		transform.translate.x = cameraPos.x;
		transform.translate.y = cameraPos.y;
		transform.translate.z = 5.0f + (-7.0f * easedValue);

		// ベース描画更新
		float minScale = 0.0f;
		float currentScale = (easedValue > minScale) ? easedValue : minScale;

		// 各項目を menuItems_ のデータに従って配置・スケール制御
		Vector3 basePos = object->GetTransform().translate;
		for (size_t i = 0; i < TextObjects.size(); ++i) {
			const auto& item = menuItems_[i];
			Vector3 followPos = basePos;
			followPos.z -= 5.0f;
			followPos.y += 1.0f + (item.offsetY * -static_cast<float>(i));
			TextObjects[i]->SetTranslate(followPos);

			// 選択時は選択用スケール（ease 完了時）を優先、未選択は easedValue を項目の maxScale でクリップ
			if (i == textindex && easeTimer_ == 1.0f) {
				TextObjects[i]->SetScale(Vector3(item.selectedScale, item.selectedScale, item.selectedScale));
			}
			else {
				float limited = std::min(currentScale, item.maxScale);
				TextObjects[i]->SetScale(Vector3(limited, limited, limited));
			}
			TextObjects[i]->Update();
		}
	}

	// 入力処理
	ControllerUpdate();

	if (!isPaused_) {
		CameraManager::GetInstance()->SetActiveCamera("maincam");
	}
}

//描画
void PauseMenu::Draw() {

	if (isPaused_) {
		//ポーズ画面
		object->Draw();
	}
	if (easeTimer_ >= 0.1f) {
		for (std::unique_ptr<Object3D>& text : TextObjects) {
			text->Draw();
		}
	}
}

//フラグのゲッター
bool PauseMenu::IsPaused()const {
	// 「ポーズ中」または「イージングがまだ戻っている最中」なら true
	return isPaused_ || easeTimer_ > 0.0f;
}

void PauseMenu::ControllerUpdate() {

	// アニメーションが完了しているか
	bool canInput = (easeTimer_ >= 1.0f);

	// =========================
	// ポーズ ON / OFF
	// =========================
	if ((canInput || easeTimer_ <= 0.0f) && Input::GetInstance()->TriggerGamePadButton(XINPUT_GAMEPAD_START) || Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = !isPaused_;
		UIeditor::GetInstance()->PlayPressAnimation("GamePlay", "back");
		UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "esc");
	}

	// =========================
	// 入力まとめ
	// =========================
	bool upPressed =
		Input::GetInstance()->TriggerKey(DIK_W) ||
		Input::GetInstance()->TriggerKey(DIK_UP) ||
		Input::GetInstance()->GetGamePadStickY() < -0.5f;

	bool downPressed =
		Input::GetInstance()->TriggerKey(DIK_S) ||
		Input::GetInstance()->TriggerKey(DIK_DOWN) ||
		Input::GetInstance()->GetGamePadStickY() > 0.5f;

	bool okPressed =
		Input::GetInstance()->TriggerKey(DIK_SPACE) ||
		Input::GetInstance()->TriggerGamePadButton(XINPUT_GAMEPAD_A);

	// =========================
	// 選択移動
	// =========================
	if (isPaused_ && canInput) {

		if (upPressed && textindex > 0) {
			textindex--;
		}

		if (downPressed && textindex < TextObjects.size() - 1) {
			textindex++;
		}

		// =========================
		// 決定（データ駆動）
		// =========================
		if (okPressed) {
			if (textindex < menuItems_.size()) {
				// メニュー項目に紐づくアクションを呼ぶ
				menuItems_[textindex].action();
			}
		}

		// 色変更（選択可視化）
		for (size_t i = 0; i < TextObjects.size(); ++i) {
			TextObjects[i]->SetColor(
				(i == textindex)
				? Vector4{ 1,0,0,1 }
				: Vector4{ 1,1,1,1 }
			);
		}
	}
	else {
		textindex = 0;
		for (auto& t : TextObjects) {
			t->SetColor({ 1,1,1,1 });
		}
	}
}
