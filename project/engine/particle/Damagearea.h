#pragma once
#include "IParticleBehavior.h"
#include <unordered_map> // ← これが必要！！
#include "RotatinField.h"
#include "MyMath.h"

class Damagearea : public IParticleBehavior
{
public:
	
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
		
	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

	
private:
	Vector3 initScale_ = { 2.0f, 2.0f, 2.0f };
	Vector3 initRotate_ = {1.27f, 0.0f, 0.0f };

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.2f; // ← ImGuiで変えられるようにする初期値

};

class FragmentBehavior : public IParticleBehavior
{
public:

	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;

	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;


private:
	Vector3 initScale_ = { 0.3f,0.3f,0.3f };
	Vector3 initRotate_ = { 0.0f, 0.0f, 0.0f };

private:
	float scrollX_ = 0.0f;
	float scrollSpeed_ = 0.2f; // ← ImGuiで変えられるようにする初期値

	
};

