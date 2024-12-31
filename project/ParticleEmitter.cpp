#include "ParticleEmitter.h"
#include <ParticleManager.h>

ParticleEmitter::ParticleEmitter(std::string groupName,Transform transform, uint32_t count, float frequency, float frequencyTime)
{
	//エミッタの初期化
	transform_ = transform;
	count_ = count;
	frequency_ = frequency;
	frequencyTime_ = frequencyTime;
	groupName_ = groupName;


}

ParticleEmitter::~ParticleEmitter()
{
}

void ParticleEmitter::Update()
{
	//時刻を進める
	frequencyTime_ += 1.0f / 60.0f;
	//発生頻度より大きいなら発生
	if (frequencyTime_ > frequency_) {
		//パーティクルの発生
		Emit();
		//時刻をリセット
		frequencyTime_ = 0.0f;
	}

}

void ParticleEmitter::Emit()
{
	ParticleManager::GetInstance()->Emit(groupName_, transform_, count_);
}
