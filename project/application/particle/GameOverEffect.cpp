#include "GameOverEffect.h"
#include "Player.h"

void GameOverEffect::Initialize(Object3D *obj)
{
	playerobject3D_ = obj;

	// アニメーション用パラメータ初期化
	timer_ = 0.0f;
	isPlaying_ = true;

	// 開始位置を保存
	startpos_ = playerobject3D_->GetTransform().translate;

	// 距離・速度などの定数設定
	jumpPower_ = 0.2f;      // 初速度
	gravity_ = 0.01f;       // 重力加速度
	forwardSpeed_ = 0.05f;  // 手前への移動速度


}

void GameOverEffect::Update(float dt)
{
	if (!isPlaying_) return;
	//プレイヤーの位置をエフェクトが見えるようにエックスを真ん中に移動
	playerobject3D_->SetTranslate({ 13.0f, playerobject3D_->GetTransform().translate.y, playerobject3D_->GetTransform().translate.z });

	// 経過時間を更新
	timer_ += dt;

	Vector3 pos = playerobject3D_->GetTransform().translate;

	// 手前（Z方向マイナス）に移動
	pos.z -= forwardSpeed_;

	// ジャンプ挙動（単純な放物運動）
	pos.y += jumpPower_;
	jumpPower_ -= gravity_; // 重力を加える（だんだん落ちる）

	float subway = -5.0f; // 地面の高さ
	// 下限を超えたら停止
	if (pos.y <= subway) { // 地面より下まで落ちたら終了
		pos.y = subway;
		isPlaying_ = false;
	}

	playerobject3D_->SetRotate({ 3.0f, timer_ * 10.0f, 0.0f }); // 回転アニメーション
	playerobject3D_->SetTranslate(pos);
	playerobject3D_->Update();

}
void GameOverEffect::Draw()
{
}


