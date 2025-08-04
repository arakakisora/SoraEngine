#include "testplayer.h"
#include "Object3DCommon.h"

testplayer::~testplayer()
{

	delete object3D;

}

void testplayer::Initialize(EulerTransform playerTransform)
{
	object3D = new Object3D();
	object3D->Initialize(Object3DCommon::GetInstance());
	object3D->SetModel("player.obj");
	object3D->SetTransform(playerTransform);

}

void testplayer::Update()
{
	object3D->Update();

}

void testplayer::Draw()
{
	object3D->Draw();
}
