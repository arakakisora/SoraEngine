#include "plyerpaticleBehavior.h"
#include "ParticleMnager.h"
#include <numbers>

Particle plyerpaticleBehavior::Create(std::mt19937& randomEngine, const EulerTransform transform, float lifetime)
{
	lifetime = lifetime;
	std::uniform_real_distribution<float>distribution(-1.0, 1.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);
	std::uniform_real_distribution<float>disRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float>disScale(0.4f, 1.5f);

	Particle particle;
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };

	particle.transform.scale = { 0.5f,disScale(randomEngine),1.0f };
	//particle.transform.scale = { 1.0f,1.0f,1.0f };
	//particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.rotate = { disRotate(randomEngine),disRotate(randomEngine),disRotate(randomEngine) };
	//particle.transform.translate = translate + randomTranslate;
	particle.transform.translate = transform.translate;
	//particle.Velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.Velocity = { 0.0f,0.0f,0.0f };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	//particle.color = { 1.0f,1.0f,1.0f,1.0f };
	//particle.lifetime = distTime(randomEngine);
	particle.lifetime = 1.0f;
	particle.currentTime = 0;
	return particle;
}

void plyerpaticleBehavior::Update(Particle& particle, float dt, Material* matelialData, float alpha)
{

	matelialData= matelialData; // X方向スクロール
	dt = dt;
	alpha = alpha;
	//パーティクルの位置を更新
	particle.transform.translate += particle.Velocity * 1.0f / 60.0f;
	//パーティクルの寿命を減らす
	particle.currentTime += 1.0f / 60.0f;
}



#include <random>
#include <numbers>
#include "MyMath.h"

Particle ExhaustGasBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime) {
	
	lifetime = lifetime;
	
	std::uniform_real_distribution<float> distOffset(-0.1f, 0.1f);
	std::uniform_real_distribution<float> distScale(0.2f, 0.4f);
	std::uniform_real_distribution<float> distRotate(-0.1f, 0.1f);
	std::uniform_real_distribution<float> distGray(0.1f, 0.3f);

	Particle particle;
	particle.transform.translate = transform.translate + Vector3(distOffset(rng), distOffset(rng), distOffset(rng));
	particle.transform.scale = Vector3(1,1,1);
	particle.transform.rotate = Vector3(0,0,0);
	particle.Velocity = Vector3(0.0f, 0.01f, 0.0f); // 上昇

	float gray = distGray(rng);
	particle.color = { gray, gray, gray, 0.6f }; // 半透明の灰色

	particle.lifetime = 1.5f;
	particle.currentTime = 0.0f;

	return particle;
}

void ExhaustGasBehavior::Update(Particle& particle, float dt, Material* materialData, float alpha) {
	//float t = particle.currentTime / particle.lifetime;
	materialData = materialData;

	// 時間と共に拡大していく（少し縦長）
	//float scale = 0.2f + 0.6f * t;
	
	alpha = alpha;
	// 上昇
	particle.transform.translate += particle.Velocity;

	// 時間更新
	particle.currentTime += dt;
}