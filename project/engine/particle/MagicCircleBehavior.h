#pragma once
#include "IParticleBehavior.h"
#include "MyMath.h"


class MagicCircleBehavior : public IParticleBehavior
{
public:
	
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
		
	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

	
private:
	Vector3 initScale_ = { 2.0f,2.0f, 2.0f };
	Vector3 initRotate_ = { 1.27f, 0.0f, 0.0f };

};

