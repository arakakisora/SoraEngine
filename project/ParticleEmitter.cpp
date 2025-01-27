#include "ParticleEmitter.h"
#include "ParticleMnager.h"


ParticleEmitter::ParticleEmitter(const Vector3& position, const float lifetime, const float currentTime, const uint32_t count, const std::string& name)
{
	position_ = position;
	velocity_ = velocity;

	frequency = lifetime;
	frequencyTime = currentTime;

	this->count = count;
	name_ = name;
}

void ParticleEmitter::Update()
{

	//時を進める

	frequencyTime += 1.0f / 60.0f;
	//発生頻度より大きいなら発生
	if (frequencyTime > frequency) {
		//パーティクルを発生
		ParticleMnager::GetInstance()->Emit(name_, position_, count);
		//時間をリセット
		frequencyTime = 0.0f;

	}


}

void ParticleEmitter::Emit()
{

	//パーティクルを発生
	ParticleMnager::GetInstance()->Emit(name_, position_, count);

}
