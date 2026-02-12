#include "PauseMenu.h"
#include <SceneManager.h>
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG
#include <ModelManager.h>
#include <CameraManager.h>

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
	ModelManager::GetInstance()->LoadModel("PauseMenu/Pause.obj");
	ModelManager::GetInstance()->LoadModel("PauseMenu/return.obj");
	ModelManager::GetInstance()->LoadModel("PauseMenu/StageSelect.obj");
	ModelManager::GetInstance()->LoadModel("PauseMenu/title.obj");

	transform.translate = { -2.6f,1.5f,0.0f };
	object = std::make_unique<Object3D>();
	object->Initialize(object3dcommon_);
	object->SetModel("PauseMenu/Pause.obj");
	object->SetScale({ 1.0f,2.0f,1.0f });
	input = Input::GetInstance();

	// 作成してでリストに追加
	for (uint32_t i = 0; i < 2; ++i) {
		std::unique_ptr<Object3D> newObject = std::make_unique<Object3D>();
		newObject->Initialize(Object3DCommon::GetInstance());
		if (i == 0) {
			newObject->SetModel("PauseMenu/return.obj");
		}
		else if (i == 1) {
			if (pauseType_ == PauseType::GamePlayScene) {
				newObject->SetModel("PauseMenu/StageSelect.obj");
			}
			else if (pauseType_ == PauseType::StageSelectScene) {
				newObject->SetModel("PauseMenu/title.obj");
			}
		}

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

	if (pauseType_ == PauseType::GamePlayScene) {
		// ゲームシーンのポーズ画面表示
		PausedStartGamePlay();
	}
	if (pauseType_ == PauseType::StageSelectScene) {
		// ゲームセレクトのポーズ画面表示
		PausedStartStageSelect();
	}
	// コントローラの動き
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

void PauseMenu::PausedStartGamePlay() {
	if (isPaused_ || easeTimer_ > 0.0f) {
		// イージングの値（0.0〜1.0）の計算（※逆再生に対応）
		float easedValue = float(easeInOutCirc(easeTimer_));

		CameraManager::GetInstance()->SetActiveCamera("pouseCamera");

		Vector3 cameraPos = CameraManager::GetInstance()->GetActiveCamera()->GetTransform().translate;
		transform.translate.x = cameraPos.x;
		transform.translate.y = cameraPos.y;
		transform.translate.z = 5.0f + (-7.0f * easedValue);

		float minScale = 0.0f;
		float limitedScale = (easedValue > 1.0f) ? 1.0f :
			(easedValue < minScale) ? minScale : easedValue;

		// ポーズを閉じている途中なら、0.0f までしっかり戻す
		if (easeTimer_ < 1.0f && !isPaused_) {
			limitedScale = (easedValue > 0.0f) ? easedValue : 0.0f;
		}
		else if (easeTimer_ >= 1.0f) {
			limitedScale = 1.0f;
		}

		transform.scale = { limitedScale, limitedScale, limitedScale };

		object->SetTranslate(transform.translate);
		object->SetScale(transform.scale);
		object->Update();

		Vector3 basePos = object->GetTransform().translate;

		float currentScale = (easedValue > minScale) ? easedValue : minScale;
		float offsetY = 1.8f;
		for (size_t i = 0; i < TextObjects.size(); ++i) {
			Vector3 followPos = basePos;
			followPos.z -= 5.0f;
			followPos.y += 1.0f + (offsetY * -static_cast<float>(i));

			TextObjects[i]->SetTranslate(followPos);

			if (i == textindex && easeTimer_ == 1.0f) {
				if (pauseType_ == PauseType::GamePlayScene) {
					TextObjects[i]->SetScale(Vector3(1.5f, 1.5f, 1.5f));
				}
				else if (pauseType_ == PauseType::StageSelectScene) {
					TextObjects[i]->SetScale(Vector3(0.8f, 0.8f, 0.8f));
				}
			}
			else {
				Vector3 limitedScale = { std::min(currentScale, 1.0f), std::min(currentScale, 1.0f), std::min(currentScale, 1.0f) };
				TextObjects[i]->SetScale(limitedScale);
			}
			TextObjects[i]->Update();
		}
	}
}

void PauseMenu::PausedStartStageSelect() {
	if (isPaused_ || easeTimer_ > 0.0f) {
		// イージングの値（0.0〜1.0）の計算（※逆再生に対応）
		float easedValue = float(easeInOutCirc(easeTimer_));


		CameraManager::GetInstance()->SetActiveCamera("pouseCamera");

		Vector3 cameraPos = CameraManager::GetInstance()->GetActiveCamera()->GetTransform().translate;
		transform.translate.x = cameraPos.x;
		transform.translate.y = cameraPos.y;
		transform.translate.z = 5.0f + (-7.0f * easedValue);

		float minScale = 0.0f;
		float const maxScale = 0.6f;
		float limitedScale = (easedValue > maxScale) ? maxScale :
			(easedValue < minScale) ? minScale : easedValue;

		// ポーズを閉じている途中なら、0.0f までしっかり戻す
		if (easeTimer_ < maxScale && !isPaused_) {
			limitedScale = (easedValue > 0.0f) ? easedValue : 0.0f;
		}
		else if (easeTimer_ >= maxScale) {
			limitedScale = maxScale;
		}

		transform.scale = { limitedScale, limitedScale, limitedScale };

		object->SetTranslate(transform.translate);
		object->SetScale(transform.scale);
		object->Update();

		Vector3 basePos = object->GetTransform().translate;

		float currentScale = (easedValue > minScale) ? easedValue : minScale;
		float offsetY = 1.3f;
		for (size_t i = 0; i < TextObjects.size(); ++i) {
			Vector3 followPos = basePos;
			followPos.z -= 5.0f;
			followPos.y += 0.8f + (offsetY * -static_cast<float>(i));

			TextObjects[i]->SetTranslate(followPos);

			if (i == textindex && easeTimer_ == 1.0f) {
				TextObjects[i]->SetScale(Vector3(0.9f, 0.9f, 0.9f));
			}
			else {
				Vector3 limitedScale = { std::min(currentScale, 0.6f), std::min(currentScale, 0.6f), std::min(currentScale, 0.6f) };
				TextObjects[i]->SetScale(limitedScale);
			}
			TextObjects[i]->Update();
		}
	}
}

void PauseMenu::ControllerUpdate() {

	// アニメーションが完了しているか
	bool canInput = (easeTimer_ >= 1.0f);

	// =========================
	// ポーズ ON / OFF
	// =========================
	if ((canInput || easeTimer_ <= 0.0f) &&Input::GetInstance()->TriggerGamePadButton(XINPUT_GAMEPAD_START)|| Input::GetInstance()->TriggerKey(DIK_ESCAPE)){
		isPaused_ = !isPaused_;
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
		// 決定
		// =========================
		if (okPressed) {

			if (textindex == 0) {
				// 戻る
				isPaused_ = false;
			}
			else if (textindex == 1) {
				// シーン遷移
				if (pauseType_ == PauseType::GamePlayScene) {
					SceneManager::GetInstance()->ChangeScene("STAGESELECT");
				}
				else if (pauseType_ == PauseType::StageSelectScene) {
					SceneManager::GetInstance()->ChangeScene("TITELE");
				}
			}
		}

		// 色変更
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
