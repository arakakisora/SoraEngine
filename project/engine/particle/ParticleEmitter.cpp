#include "ParticleEmitter.h"




ParticleEmitter::ParticleEmitter( EulerTransform transform, const float lifetime, const float currentTime, const uint32_t count, const std::string& name)
{
	this->transform_ = transform;//位置
	frequency = lifetime;//寿命
	frequencyTime = currentTime;//現在の寿命
	this->count = count;//count
	name_ = name;//名前
	
}

void ParticleEmitter::Update()
{
	// 時間を進める
	frequencyTime += 1.0f / 60.0f;

	// 寿命（frequency）を超えたら発生
	if (frequencyTime >= frequency) {
		ParticleManager::GetInstance()->Emit(name_, transform_);
		frequencyTime = 0.0f;
	}
}

void ParticleEmitter::Emit()
{

	//パーティクルを発生
	ParticleManager::GetInstance()->Emit(name_, transform_);

}
