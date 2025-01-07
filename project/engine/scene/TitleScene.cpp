#include "TitleScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "Input.h"
#include "SceneManager.h"
#include<imgui.h>

void TitleScene::Initialize()
{
	//titeleの生成
	
	titleSprite = new Sprite();
	titleSprite->Initialize(SpriteCommon::GetInstance(), "Resources/titelesupercer.png");
	titleSprite->SetSize({ 1280,720 });
	
}

void TitleScene::Finalize()
{
	delete titleSprite;
	titleSprite = nullptr;
}

void TitleScene::Update()
{
	//スプライトの更新
	titleSprite->Update();

	//スペースキーが押されたらゲームプレイシーンに遷移
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {

		
		
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}

#ifdef DEBUG_

	if (ImGui::CollapsingHeader("title", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Vector2 transform= titleSprite->GetPosition();
		Vector2 scale = titleSprite->GetSize();

		//ImGui::DragFloat3("*ModelScale", &transform.scale.x, 0.01f);
		ImGui::DragFloat2("*titletransrate", &transform.x, 0.01f);
		ImGui::DragFloat2("*titelescale", &scale.x, 0.01f);
		titleSprite->SetPosition(transform);
		titleSprite->SetSize(scale);

	}
#endif // DEBUG_


}

void TitleScene::Draw()
{
	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();

	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

	//スプライトの描画
	titleSprite->Draw();

}
