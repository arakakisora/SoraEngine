#pragma once
#include <memory>
#include "BaseScene.h"
#include "Sprite.h"
#include "FadeManager.h"
#include "Object3D.h"
#include <ParticleEmitter.h>

class Camera;

class GameOverScene : public BaseScene
{
public:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update() override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw() override;

public:
private:
	//タイトル用スプライト
	std::unique_ptr<Sprite> sprite;

	//fade
	FadeManager fadeManager_;

	//カメラ
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Object3D>playerObj = nullptr;

	float animTimer_ = 0.0f;
	float animDuration_ = 60.0f; // 60フレーム
	bool isBreakAnimEnd_ = false;

	Vector3 basePos_ = { 0.0f, 0.0f, 0.0f };

	std::unique_ptr< ParticleEmitter> deatheEffect_ = nullptr;
	//パアーティクルパラメータ
	float lifeTime = 1.0f;
	float currentTime = 1.0f;
	uint32_t maxParticles = 100;
	float exhaustTimer_ = 0.0f;


#ifdef USE_IMGUI
	float rotateSpeedY_ = 0.03f;
#endif

};

