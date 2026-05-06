#pragma once
#include "IParticleBehavior.h"

class BarrierRingBehavior : public IParticleBehavior
{
public:
    Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) override;
    void Update(Particle& particle, float dt, Material* materialData, float alpha) override;

#ifdef USE_IMGUI
    void DrawImgui(const char* effectName) override;
#endif

private:
    Vector3 baseScale_ = { 2.0f, 2.0f, 1.0f };
    Vector3 baseRotate_ = { 0.0f, 0.0f, 0.0f };

    float pulseSpeed_ = 8.0f;     // じりじり感
    float pulseWidth_ = 0.12f;    // 脈動幅
    float alphaMin_ = 0.55f;
    float alphaMax_ = 0.95f;
    float rotateSpeed_ = 1.2f;    // じわっと回る
    float uvScrollSpeed_ = 0.8f;  // テクスチャ流し
};