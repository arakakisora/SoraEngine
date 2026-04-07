#pragma once
#include "IParticleBehavior.h"

class BarrierBreakBehavior : public IParticleBehavior
{
public:
    Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
    void Update(Particle& particle, float dt, Material* materialData, float alpha) override;

#ifdef USE_IMGUI
    void DrawImgui(const char* effectName) override;
#endif

private:
    Vector3 initScale_ = { 0.25f, 0.25f, 0.05f };
    Vector3 scaleRandom_ = { 0.15f, 0.15f, 0.02f };

    float speedMin_ = 2.0f;
    float speedMax_ = 6.0f;
    float upMin_ = 1.5f;
    float upMax_ = 4.0f;

    float gravity_ = 6.0f;
    float rotateSpeedMin_ = -6.0f;
    float rotateSpeedMax_ = 6.0f;
};