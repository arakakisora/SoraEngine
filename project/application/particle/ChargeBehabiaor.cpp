#include "ChargeBehabiaor.h"
#include "IParticleBehavior.h"
#include "ParticleMnager.h"
#include <Input.h>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
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
	Vector3 dir = MyMath::Normalize(Vector3{ dist(rng), dist(rng) * 0.5f, dist(rng) }); // Y成分は少し抑える
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

Particle ExhaustGasBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	Particle particle;

	// --- 初期位置（プレイヤーの足元あたり、少しだけバラけさせる） ---
	std::uniform_real_distribution<float> offsetX(-0.05f, 0.05f);
	std::uniform_real_distribution<float> offsetZ(-0.02f, 0.02f);

	particle.transform.translate = transform.translate +
		Vector3(offsetX(rng), -0.1f, offsetZ(rng));

	// --- 初期スケール（小さめからスタート） ---
	float s = 0.12f;
	particle.transform.scale = { s, s, s };

	// --- 初期回転（軽くランダム回転） ---
	std::uniform_real_distribution<float> rotDist(0.0f, std::numbers::pi_v<float> *2.0f);
	particle.transform.rotate = { 0.0f, 0.0f, rotDist(rng) };

	// --- 上昇 + 少し揺れるゆっくりした速度 ---
	std::uniform_real_distribution<float> vx(-0.005f, 0.005f);
	std::uniform_real_distribution<float> vz(-0.005f, 0.005f);
	particle.Velocity = { vx(rng), 0.015f, vz(rng) };

	// --- 色（ほぼ白〜薄いグレー・透明気味） ---
	std::uniform_real_distribution<float> gray(0.75f, 0.9f);
	float c = gray(rng);
	particle.color = { c, c, c, 0.6f }; // αは控えめ

	// --- 寿命（短めでサッと消える） ---
	particle.lifetime = 0.7f;
	particle.currentTime = 0.0f;

	return particle;
}

void ExhaustGasBehavior::Update(Particle& particle, float dt, Material* materialData, float alpha)
{
	(void)materialData;
	(void)alpha;

	particle.currentTime += dt;
	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);

	// 速度で移動（少しずつ上昇させる）
	particle.transform.translate += particle.Velocity * dt;

	// 上昇につれてわずかに減速＆上向きに寄せると「漂ってる感」が出る
	particle.Velocity.x *= 0.98f;
	particle.Velocity.z *= 0.98f;
	particle.Velocity.y += 0.005f * dt; // ほんの少しだけ浮力

	// スケール拡大（じんわり大きくなる）
	float scale = std::lerp(0.12f, 0.35f, t);
	particle.transform.scale = { scale, scale, scale };

	// アルファはなめらかに消えていく (二乗カーブで最後ふっと消える)
	float a = 1.0f - t;
	particle.color.w = a * a;
}