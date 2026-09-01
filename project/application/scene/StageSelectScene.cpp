#include "StageSelectScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include <CameraManager.h>
#include <ModelManager.h>
#include <UIeditor.h>



void StageSelectScene::Initialize()
{
	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->Finalize();
	//カメラの生成
	camera1 = std::make_unique<Camera>();
	camera1->SetTranslate({ 0,0,-10, });//カメラの位置
	CameraManager::GetInstance()->AddCamera("maincam", camera1.get());

	// デフォルトカメラを設定
	CameraManager::GetInstance()->SetActiveCamera("maincam");

	// ステージのインデックスを取得
	int stageIndex = SceneManager::GetInstance()->GetStageIndex();
	currentIndex_ = stageIndex;	// 現在のステージを設定	 (ステージ1)

	ModelManager::GetInstance()->LoadModel("plane");

	//背景
	ModelManager::GetInstance()->LoadModel("stage1");
	ModelManager::GetInstance()->LoadModel("stage2");
	ModelManager::GetInstance()->LoadModel("stage3");
	ModelManager::GetInstance()->LoadModel("stage4");
	ModelManager::GetInstance()->LoadModel("stage5");

	ModelManager::GetInstance()->LoadModel("player");
	playerobj = std::make_unique<Object3D>();
	playerobj->Initialize(Object3DCommon::GetInstance());
	playerobj->SetModel("player");
	playerobj->SetScale({ 0.25f,0.25f,0.25f });


	std::vector<std::string> stageModels = {
	"stage1",
	"stage2",
	"stage3",
	"stage4",
	"stage5"
	// 必要ならさらに追加
	};

	for (int i = 0; i < MaxSelectIndex_; i++) {
		StageObject stage;
		stage.object = std::make_unique<Object3D>();
		stage.object->Initialize(Object3DCommon::GetInstance());
		stage.object->SetRotate(Vector3(0.0f, 3.1f, 0.0f));

		// iに応じてモデルを選択
		if (i < stageModels.size()) {
			stage.object->SetModel(stageModels[i]);
		} else {
			// モデルが足りない場合はデフォルトを入れる
			stage.object->SetModel("stage1");
		}
		stage.object->SetEnableLighting(true);



		stage.basePos = { (float)i * inrerval_, 0.0f, 0.0f }; // x方向に間隔をあけて配置
		stages_.push_back(std::move(stage));
	}

	// PauseMenu 初期化（セレクトシーン用）
	pauseMenu_.Initialize(Object3DCommon::GetInstance(), PauseType::StageSelectScene);
	pauseMenu_.SetCamera(camera1.get());

	// フェードインの初期化
	fadeManager_.Initialize("Resources/white.png");
	fadeManager_.StartFadeIn();

	gate_ = std::make_unique<GateInOut>();
	gate_->Initialize(SpriteCommon::GetInstance(),
		"Resources/block.png", "Resources/block.png");
	gate_->SetScreenSize(1280.0f, 720.0f); // WinAppから取れるならそれで
	gate_->StartIn(0.6f);
	gateOutRequested_ = false;

	UIeditor::GetInstance()->SetScene("StageSelect");

}

void StageSelectScene::Finalize()
{
	CameraManager::GetInstance()->RemoveCamera("maincam");
	CameraManager::GetInstance()->Finalize();

}

void StageSelectScene::Update()
{
	//カメラの更新
	CameraManager::GetInstance()->GetActiveCamera()->Update();
	// フェード更新
	fadeManager_.Update();
	float dt = 1.0f / 60.0f;
	if (gate_) gate_->Update(dt);

	// PauseMenu の更新（先に更新してポーズ判定を反映）
	pauseMenu_.Update();

	// ポーズ中はセレクト操作やフェード開始などのゲーム入力を無視する
	if (!pauseMenu_.IsPaused()) {

		float targetOffset = -(float)currentIndex_ * inrerval_; // 中央に来るようにオフセット
		scrollOffset_ += (targetOffset - scrollOffset_) * 0.1f; // イージング

		
		if (!gateOutRequested_ &&
			(!gate_ || !gate_->IsPlaying()) &&
			Input::GetInstance()->TriggerKey(DIK_SPACE)||Input::GetInstance()->TriggerMouse(0)){
			UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "click");
			gateOutRequested_ = true;
			if (gate_) gate_->StartOut(0.6f);
		}

		if (gateOutRequested_ && !fadeOutRequested_ && gate_ && gate_->IsFinished()) {
			fadeOutRequested_ = true;

			gate_->HoldClosed(true);

			fadeManager_.StartFadeOut();
		}

		if (fadeOutRequested_ && fadeManager_.IsFadeOutFinished()) {
			SceneManager::GetInstance()->SetStageIndex(currentIndex_);
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");

		}

		SelectMove();
	}
	else {
		// ポーズ中でもスクロールの補間は見た目のため保持する
		float targetOffset = -(float)currentIndex_ * inrerval_;
		scrollOffset_ += (targetOffset - scrollOffset_) * 0.1f;
	}

#ifdef _DEBUG
	ImGui::Begin("STAGESELECT");
	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("sutage%d", currentIndex_);

		ImGui::Text("titleScene %d");
		if (ImGui::Button("gamePlayScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		}

		if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {
			// 代表用に一時変数を用意
			static SpotLight spot;
			static bool initialized = false;
			if (!initialized && !stages_.empty()) {
				spot = stages_[0].object->GetSpotLight(); // 最初のオブジェクトからコピー
				initialized = true;
			}

			// UIでパラメータ調整
			ImGui::ColorEdit4("Color", &spot.color.x);
			ImGui::DragFloat3("Position", &spot.position.x, 0.1f);
			ImGui::DragFloat3("Direction", &spot.direction.x, 0.1f);
			ImGui::DragFloat("Intensity", &spot.intensity, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Distance", &spot.distance, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &spot.decay, 0.01f, 0.0f, 2.0f);
			ImGui::DragFloat("ConsAngle", &spot.consAngle, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("CosFalloffStart", &spot.cosFalloffstrt, 0.01f, 0.0f, 1.0f);

			// 全オブジェクトに反映
			for (auto& stage : stages_) {
				stage.object->SetSpotLight(spot);
			}

		}

	}
	ImGui::End();
#endif // _DEBUG
}

void StageSelectScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	for (int i = 0; i < stages_.size(); i++) {
		Vector3 pos = stages_[i].basePos;
		pos.x += scrollOffset_;  // オフセット反映
		stages_[i].object->SetTranslate(pos);

		float scale = (i == currentIndex_) ? 1.2f : 1.0f;
		stages_[i].object->SetScale({ scale, scale, scale });

		stages_[i].object->Update();
		stages_[i].object->Draw();
	}

	// PauseMenu を 3D パス内で描画（オーバーレイ）
	pauseMenu_.Draw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

	// ControlGuide をここで描画すると UI レイヤーで最前面に来ます
	UIeditor::GetInstance()->Render();

	if (gate_) gate_->Draw2D();
	fadeManager_.Draw();
}

void StageSelectScene::SelectMove()
{
	static float holdDelay_ = 0.3f;        // 長押し開始までの時間
	static float repeatInterval_ = 0.25f;  // 長押し中のリピート間隔
	static float holdTimer_ = 0.0f;
	static float repeatTimer_ = 0.0f;

	float rightStickX = Input::GetInstance()->GetGamePadStickX(); // -1.0〜1.0
	const float stickThreshold = 0.5f;

	if (Input::GetInstance()->PushKey(DIK_D)) {
		UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "d");
		rightStickX = 1.0f;
	} else if (Input::GetInstance()->PushKey(DIK_A)) {
		UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "a");
		rightStickX = -1.0f;
	}

	auto mouseMove = Input::GetInstance()->GetMouseMove();
	if (mouseMove.lZ != 0) {
		
		// WHEEL_DELTA(=120) ごとに1ノッチ。floatで扱うことで細かい差分にも対応。
		const float wheelNotches = static_cast<float>(mouseMove.lZ) / static_cast<float>(WHEEL_DELTA);
		// 1ノッチを kCannonAngleStepDeg として適用。感度を変えたい場合は係数を掛ける。
		rightStickX += wheelNotches ;
		if (wheelNotches < 0) { 
			UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "mouse2"); 
		}
		else if (wheelNotches > 0) {
			UIeditor::GetInstance()->PlayPressAnimation("StageSelect", "mouse");
		}
	}

#ifdef _DEBUG


#endif

	if (fabs(rightStickX) > stickThreshold) {
		// 入力が続いているのでタイマー加算
		holdTimer_ += deltaTime_;

		if (holdTimer_ >= holdDelay_) {
			// 長押し中
			repeatTimer_ += deltaTime_;
			if (repeatTimer_ >= repeatInterval_) {
				if (rightStickX > 0 && currentIndex_ < MaxSelectIndex_ - 1) {
					currentIndex_++;
				} else if (rightStickX < 0 && currentIndex_ > 0) {
					currentIndex_--;
				}
				repeatTimer_ = 0.0f; // リピート間隔リセット
			}
		} else {
			// 押した瞬間（初回だけすぐ反応）
			if (rightStickX > 0 && currentIndex_ < MaxSelectIndex_ - 1) {
				currentIndex_++;
				holdTimer_ = holdDelay_; // 長押し状態に強制移行
			} else if (rightStickX < 0 && currentIndex_ > 0) {
				currentIndex_--;
				holdTimer_ = holdDelay_;
			}
		}
	} else {
		// 入力が離れたらリセット
		holdTimer_ = 0.0f;
		repeatTimer_ = 0.0f;
	}
}

