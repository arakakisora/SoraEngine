#pragma once

#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"

#include "SceneManager.h"
#include "ParticleEmitter.h"
#include "ParticleMnager.h"



class GamePlayScene :public BaseScene
{
public:

	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize()override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update()override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw()override;

private:
	std::unique_ptr<Camera> camera1;
	std::unique_ptr<Camera> camera2;
	std::unique_ptr<Object3D> object3D;
	//particle
	std::unique_ptr<ParticleEmitter> particleEmitter;
	std::unique_ptr<ParticleEmitter> particleEmitter2;
	bool light = true;
	std::unique_ptr<Sprite> sprite;
	

};

