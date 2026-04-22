#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include <memory>
#include <FadeManager.h>
#include "GateInOut.h"
#include <ParticleEmitter.h>
#include "Object3D.h"

class Camera;

class GameClearScene : public BaseScene
{
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	// タイトル用スプライト
	std::unique_ptr<Sprite> sprite;

	// fade
	FadeManager fadeManager_;

	// カメラ
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Object3D> playerObj = nullptr;

	// クリア演出用
	float clearAnimTimer_ = 0.0f;
	bool isLanding_ = false;

	Vector3 basePos_ = { 0.0f, -0.7f, 11.0f };

	// パーティクル
	std::unique_ptr<ParticleEmitter> deatheEffect_ = nullptr;
	float lifeTime = 1.0f;
	float currentTime = 1.0f;
	uint32_t maxParticles = 100;
	float exhaustTimer_ = 0.0f;
	float baseRotateY_ = 3.14159265f;

#ifdef USE_IMGUI
	float rotateSpeedY_ = 0.03f;
#endif

};