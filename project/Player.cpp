#include "Player.h"
#include "ImGuiManager.h"
#include <imgui.h>

Player::Player()
{
}

Player::~Player()
{
	
}

void Player::Initialize(Object3D* Object3D)
{
	//playerobject
	object3D_ = Object3D;
	//playerのデータ初期化
	playerData.position = { 0,0,0 };//プレイヤーの位置
	playerData.Velocity = { 1.0f,1.0f,0 };//playerの速度
	playerData.rotation = { 0,0,0 };//playerの回転
	playerData.scale = { 1.0f,1.0f,1.0f };//playerのスケール
	playerData.acceleration = { 0.5f,0.5f,0.5f };//playerの加速度
	


}

void Player::Update()
{
	//playerの位置を更新

	


	object3D_->SetTransform({ playerData.scale,playerData.rotation,playerData.position });
	object3D_->Update();


#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//playerの位置
		ImGui::DragFloat3("*PlayerPosition", &playerData.position.x, 0.01f);
	}
#endif // _DEBUG

	

}

void Player::Draw()
{
	//playerの描画
	object3D_->Draw();
}

void Player::MoveRight()
{
	playerData.position.x += playerData.Velocity.x;
}

void Player::MoveLeft()
{
	playerData.position.x -= playerData.Velocity.x;
}
