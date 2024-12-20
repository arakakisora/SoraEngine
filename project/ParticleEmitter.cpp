#include "ParticleEmitter.h"
#include "ParticleManager.h"

//instansの初期化
ParticleEmitter* ParticleEmitter::instance = nullptr;
ParticleEmitter* ParticleEmitter::GetInstans()
{
	if (instance == nullptr) {
		instance = new ParticleEmitter();
	}
	return instance;
	
}

void ParticleEmitter::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ParticleEmitter::Initialize(Transform transforms, uint32_t count, float frequency)
{
	//エミッタの初期化
	//エミッタのtransform
	emitter.transform=transforms;
	//発生数
	emitter.count = count;
	//発生頻度
	emitter.frequency = frequency;
	//頻度用時刻
	emitter.frequencyTime = 0.0f;

	createparticle = false;

}

void ParticleEmitter::Update(const std::string& name)
{

	emitter.frequencyTime += kDeletaTime;//時刻を進める
	if (emitter.frequency <= emitter.frequencyTime) {//頻度より大きいなら発生
		if (createparticle) {

			ParticleManager::GetInstans()->Emit(name, emitter.transform.translate, emitter.count);
			emitter.frequencyTime -= emitter.frequency;//余計に過ぎた時間も加味して頻度計算する

		}
	}
	

}

void ParticleEmitter::Emit(const std::string& name)
{
	ParticleManager::GetInstans()->Emit(name, emitter.transform.translate, emitter.count);
}


