#pragma once
#include "IParticleBehavior.h"

class AttackBehavior : public IParticleBehavior
{
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;

	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

};

