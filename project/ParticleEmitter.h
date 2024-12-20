#pragma once
#include "RenderingData.h"
//エミッタ
struct Emitter {
	Transform transform;//エミッタのtransform
	uint32_t count;//発生数
	float frequency;//発生頻度
	float frequencyTime;//頻度用時刻	
};



class ParticleEmitter
{

	static ParticleEmitter* instance;

	ParticleEmitter()=default;
	~ParticleEmitter()=default;
	ParticleEmitter(ParticleEmitter&) = default;
	ParticleEmitter& operator=(ParticleEmitter&) = delete;
	//シングルトンインスタンスの取得

public:
	static ParticleEmitter* GetInstans();
	//終了
	void Finalize();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Transform transforms, uint32_t count, float frequency);
	/// <summary>
	/// 更新
	/// </summary>
	void Update(const std::string& name);

	void Emit(const std::string& name);
	
	

	
    private:

        Emitter emitter;
		//デルタタイム
		const float kDeletaTime = 1.0f / 60.f;
		bool createparticle = false;


};

