#pragma once
#include"RenderingData.h"
//エミッタ

class ParticleEmitter
{
public:
	//コンストラクタ
	ParticleEmitter(std::string groupNam,Transform transform, uint32_t count, float frequency, float frequencyTime);
	//デストラクタ
	~ParticleEmitter();

/// <summary>
/// 更新
/// </summary>
	void Update();

	void Emit();


private:
	

	Transform transform_;//エミッタのtransform
	uint32_t count_;//発生数
	float frequency_;//発生頻度
	float frequencyTime_;//頻度用時刻	
	std::string groupName_;
	


};

