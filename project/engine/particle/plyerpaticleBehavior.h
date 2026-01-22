#pragma once
#include "IParticleBehavior.h"

class plyerpaticleBehavior : public IParticleBehavior
{
	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;

	void Update(Particle& particle, float dt, Material* matelialData, float alpha) override;

};

//class ExhaustGasBehavior : public IParticleBehavior {
//public:
//	Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
//	void Update(Particle& particle, float dt, Material* materialData, float alpha) override;
//};

