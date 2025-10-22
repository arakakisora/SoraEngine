#pragma once
#include "IParticleBehavior.h"

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


private:
	Vector3 initScale_ = { 0.0f, 0.0f, 0.0f };
	Vector3 initRotate_ = { 1.27f, 0.0f, 0.0f };

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.2f; 

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


private:
	Vector3 initScale_ = { 0.0f, 0.0f, 0.0f };
	Vector3 initRotate_ = { 1.27f, 0.0f, 0.0f };

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.2f; // ← ImGuiで変えられるようにする初期値



};

class AbsorbLineParticle : public IParticleBehavior
{
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;

	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;


private:
	Vector3 initScale_ = { 0.0f, 0.0f, 0.0f };
	Vector3 initRotate_ = { 1.27f, 0.0f, 0.0f };

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.2f; // ← ImGuiで変えられるようにする初期値


};