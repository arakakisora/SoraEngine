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
#include "Line.h"

#include "SkyBox.h"
#include "MapChipField.h"
#include "StageEditor.h"


class DebugScene :public BaseScene
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

	void LoadModel();
	void Loadparticle();
	void LoadAudio();
	/// ブロックの生成
	void GenerateObject3D();
	void StageEditer();

private:
	std::unique_ptr<Camera> camera1;
	std::unique_ptr<Camera> camera2;
	std::unique_ptr<Object3D> object3D;
	std::unique_ptr<Object3D> terrain;

	//particle
	std::unique_ptr<ParticleEmitter> particleEmitter;
	std::unique_ptr<ParticleEmitter> particleEmitter2;
	bool light = true;
	bool directionLight = true;
	bool pointLight = false;
	bool spotLight = false;
	std::unique_ptr<Sprite> sprite;
	SoundData sampleSoundData;//サウンドデータ
	
	bool number = 0;

	std::unique_ptr<Line> line;

	Vector3 startline;
	Vector3 endline;

	std::unique_ptr<SkyBox> skyBox;
	
	MapChipField* mapChipField_;
	StageEditor editor;
	std::vector<std::vector<Object3D*>> blockobject3D;

};

