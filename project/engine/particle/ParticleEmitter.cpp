#include "ParticleEmitter.h"
#include "ParticleMnager.h"


ParticleEmitter::ParticleEmitter(const Vector3& position, const Vector3& velocity, const float lifetime, const float currentTime, const uint32_t count, const std::string& name)
{
	position_ = position;
	velocity_ = velocity;
	lifetime_ = lifetime;
	currentTime_ = currentTime;
	this->count = count;
	name_ = name;
}

void ParticleEmitter::Update()
{

	//時を進める
	currentTime_ += 0.1f;
	//発生頻度より大きいなら発生
	if (currentTime_ > lifetime_) {
		//パーティクルを発生
		ParticleMnager::GetInstance()->Emit(name_, position_, count);
		//時間をリセット
		currentTime_ = 0.0f;
	}


}

void ParticleEmitter::Emit()
{

	//パーティクルを発生
	ParticleMnager::GetInstance()->Emit(name_, position_, count);

}
