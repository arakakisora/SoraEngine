#include "ChargeBehabiaor.h"
#include "IParticleBehavior.h"
#include "ParticleMnager.h"
#include <Input.h>
#include <imgui.h>
#include "MyMath.h"
#include <numbers>

Particle ChargeBehabiaor::Create(std::mt19937& rng, const EulerTransform transform, float lifetime_)
{
	rng = rng;
	Particle particle;

	particle.transform.translate = transform.translate;
	particle.transform.scale = { 1.2f, 1.2f, 1.2f }; // 最初は大きめ
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.Velocity = { 0.0f, 0.0f, 0.0f };

	// 黄色っぽい色から始める
	particle.color = { 1.0f, 1.0f, 0.5f, 1.0f };

	particle.lifetime = lifetime_;
	particle.currentTime = 0;

	return particle;

}

void ChargeBehabiaor::Update(Particle& particle, float dt, Material* matelialData, float alpha)
{
	matelialData = matelialData;
	alpha = alpha;

	particle.currentTime += dt;

	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);

	// === スムーズに縮小（EaseIn） ===
	float eased = t * t; // EaseInQuad
	float scale = std::lerp(1.2f, 0.3f, eased); // 大→小へ

	particle.transform.scale = { scale, scale, scale };

	// === 色変化（黄色→白） ===
	particle.color.x = std::lerp(1.0f, 1.0f, t);  // R
	particle.color.y = std::lerp(1.0f, 1.0f, t);  // G
	particle.color.z = std::lerp(0.5f, 1.0f, t);  // B (黄→白)
	particle.color.w = 1.0f - t; // アルファ減衰
}




Particle ExplosionBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	Particle particle;

	// --- ランダムな方向とスピード（ゆっくり広がる） ---
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	Vector3 dir = MyMath::Normlize(Vector3{ dist(rng), dist(rng) * 0.5f, dist(rng) }); // Y成分は少し抑える
	std::uniform_real_distribution<float> speedDist(0.3f, 1.0f);
	particle.Velocity = dir * speedDist(rng);

	// --- 初期位置・スケール・回転 ---
	particle.transform.translate = transform.translate;
	std::uniform_real_distribution<float> scaleDist(0.2f, 0.5f);
	float initialScale = scaleDist(rng);
	particle.transform.scale = { initialScale, initialScale, initialScale };

	std::uniform_real_distribution<float> rotDist(0.0f, std::numbers::pi_v<float> *2.0f);
	particle.transform.rotate = { 0.0f, 0.0f, rotDist(rng) };

	// --- 色：煙（グレー系） ---
	std::uniform_real_distribution<float> grayDist(0.1f, 0.4f);
	float gray = grayDist(rng);
	particle.color = { gray, gray, gray, 1.0f };

	particle.lifetime = lifetime;
	particle.currentTime = 0;

	return particle;
}

void ExplosionBehavior::Update(Particle& particle, float dt, Material* matelialData, float alpha)
{
	alpha = alpha;
	particle.currentTime += dt;
	matelialData = matelialData;

	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);

	// 位置更新（速度ベクトルで拡がる）
	particle.transform.translate += particle.Velocity * dt;

	// スケールをわずかに拡大
	float scale = std::lerp(0.3f, 0.6f, t);
	particle.transform.scale = { scale, scale, scale };

	// アルファ減衰（一気に消える）
	particle.color.w = 1.0f - t;

}

