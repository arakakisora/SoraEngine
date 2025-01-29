#include "ParticleEmitter.h"
#include "ParticleMnager.h"



ParticleEmitter::ParticleEmitter(const Vector3& position, const float lifetime, const float currentTime, const uint32_t count, const std::string& name)
{
	position_ = position;//位置
	frequency = lifetime;//寿命
	frequencyTime = currentTime;//現在の寿命
	this->count = count;//count
	name_ = name;//名前
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
