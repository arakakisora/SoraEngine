#pragma once
#include "IParticleBehavior.h"
#include <ImGui.h>

/// <summary>
/// チャージエフェクトのビヘイビアクラス
/// </summary>
class ChargeBehabiaor: public IParticleBehavior
{
public:
	/// <summary>
	/// パーティクルの生成
	/// </summary>
	/// <param name="rng"></param>
	/// <param name="transform"></param>
	/// <param name="lifetime"></param>
	/// <returns></returns>
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="particle"></param>
	/// <param name="dt"></param>
	/// <param name="matelialData"></param>
	/// <param name="alpha"></param>
	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

#ifdef USE_IMGUI
	void DrawImgui(const char* effectName) override;
#endif
private:
	PerticleParameter param_;
	
};
 
/// <summary>
/// 爆発エフェクトのビヘイビアクラス
/// </summary>
class ExplosionBehavior : public IParticleBehavior
{
public:
	/// <summary>
	// パーティクルの生成
	/// </summary>
	/// <param name="rng"></param>
	/// <param name="transform"></param>
	/// <param name="lifetime"></param>
	/// <returns></returns>
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="particle"></param>
	/// <param name="dt"></param>
	/// <param name="matelialData"></param>
	/// <param name="alpha"></param>
	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

#ifdef USE_IMGUI
	void DrawImgui(const char* effectName) override;
#endif

private:
	PerticleParameter param_;

};

class ExhaustGasBehavior : public IParticleBehavior {
public:
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
	void Update(Particle& particle, float dt, Material* materialData, float alpha) override;

#ifdef USE_IMGUI
	void DrawImgui(const char* effectName) override;
#endif

private:
	PerticleParameter param_{
	.speedMin_ = -0.005f,
	.speedMax_ = 0.005f,
	.spreadY_ = 0.5f,
	.scaleMin_ = 0.12f,
	.scaleMax_ = 0.20f,
	.grayMin_ = 0.75f,
	.grayMax_ = 0.90f,
	};

};