#include "newPlayer.h"
#include "Object3DCommon.h"

void newPlayer::Initialize(const Vector3& position) {

	object3D_ = std::make_unique<Object3D>();
	object3D_->Initialize(Object3DCommon::GetInstance());

}

void newPlayer::Update() {


	Move();



	object3D_->SetTranslate(velocity_);
	object3D_->Update();
}


void newPlayer::Draw() {
}


void newPlayer::Move() {



}


