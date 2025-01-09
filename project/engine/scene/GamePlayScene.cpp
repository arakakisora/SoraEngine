#include "GamePlayScene.h"
#include <ModelManager.h>
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include "Input.h"
#include "TitleScene.h"

void GamePlayScene::Initialize()
{
	//カメラの生成
	sampleCamera = new Camera();
	sampleCamera->SetTranslate({ 0,0,-10, });//カメラの位置
	Object3DCommon::GetInstance()->SetDefaultCamera(sampleCamera);//defaultカメラの設定


	//モデルの読み込み
	ModelManager::GetInstans()->LoadModel("plane.obj");


	//3Dオブジェクトの生成
	sampleObj3d = new Object3D();
	sampleObj3d->SetModel("plane.obj");//モデルの設定
	sampleObj3d->Initialize(Object3DCommon::GetInstance());//初期化

	//スプライトの生成
	sampleSprite = new Sprite();
	sampleSprite->Initialize(SpriteCommon::GetInstance(), "Resources/monsterBall.png");
	sampleSprite->SetSize({ 400,400 });


	//サウンドの読み込み
	sampleSoundData = Audio::GetInstance()->SoundLoadWave("Resources/gamePlayBGM.wav");//今のところwavのみ対応

}

void GamePlayScene::Finalize()
{
	delete sampleCamera;
	delete sampleObj3d;
	delete sampleSprite;


}

void GamePlayScene::Update()
{
	//カメラの更新
	sampleCamera->Update();
	//3Dオブジェクトの更新
	sampleObj3d->Update();
	//スプライトの更新
	sampleSprite->Update();




#ifdef _DEBUG

	/*if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gamePlayScene %d");
		if (ImGui::Button("GameClearScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMECLEAR");
		}
		if (ImGui::Button("GameOverScene"))
		{
			SceneManager::GetInstance()->ChangeScene("GAMEOVER");
		}


	}*/

	if (ImGui::CollapsingHeader("sumple", ImGuiTreeNodeFlags_DefaultOpen)) {

		if (ImGui::CollapsingHeader("Object3d", ImGuiTreeNodeFlags_DefaultOpen)) {

			Transform transform = sampleObj3d->GetTransform();
			ImGui::DragFloat3("Translate", &transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
			sampleObj3d->SetTransform(transform);

		}
		if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {

			Vector2 transform = sampleSprite->GetPosition();
			Vector2 scale = sampleSprite->GetSize();

			//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
			ImGui::DragFloat2("*transrate", &transform.x, 0.01f);
			ImGui::DragFloat2("*scale", &scale.x, 0.01f);
			sampleSprite->SetPosition(transform);
			sampleSprite->SetSize(scale);

		}
		if (ImGui::CollapsingHeader("Sound", ImGuiTreeNodeFlags_DefaultOpen)) {

			if (ImGui::Button("Play Sound")) {
				//再生
				if (!soundPlaying) {
					Audio::GetInstance()->SoundPlayWave(sampleSoundData);
					soundPlaying = true;
				}
			}

			if (soundPlaying && !Audio::GetInstance()->IsSoundPlaying()) {
				soundPlaying = false;
			}

			if (ImGui::Button("Pause Sound")) {
				//一時停止
				Audio::GetInstance()->PauseAudio();
			}

			if (ImGui::Button("Resume Sound")) {
				//止めたところから再開
				Audio::GetInstance()->ResumeAudio();
			}


		}


	}
#endif // _DEBUG
}

void GamePlayScene::Draw()
{

#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();
	//3Dオブジェクトの描画
	sampleObj3d->Draw();

#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();
	//スプライトの描画
	sampleSprite->Draw();

#pragma endregion




}
