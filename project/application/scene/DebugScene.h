#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include <memory>
#include <FadeManager.h>
#include "GateInOut.h"
#include <ParticleEmitter.h>
#include "Object3D.h"

class Camera;

class DebugScene : public BaseScene
{
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	
	// カメラ
	std::unique_ptr<Camera> camera;
	



#ifdef USE_IMGUI
	
#endif

};