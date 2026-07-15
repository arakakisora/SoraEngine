#include "BarrierRingBehavior.h"
#include "ParticleManager.h"
#include <cmath>
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

Particle BarrierRingBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	rng = rng;

	Particle particle{};
	particle.center = transform.translate;
	particle.transform.translate = transform.translate;
	particle.transform.scale = baseScale_;
	particle.transform.rotate = baseRotate_;
	particle.Velocity = {0.0f, 0.0f, 0.0f};

	// 水色寄りバリア色
	particle.color = {0.35f, 0.85f, 1.0f, 0.85f};

	particle.lifetime = lifetime;
	particle.currentTime = 0.0f;
	return particle;
}

void BarrierRingBehavior::Update(Particle& particle, float dt, Material* materialData, float alpha)
{
	alpha = alpha;

	particle.currentTime += dt;

	float t = particle.currentTime;

	// じりじり脈動
	float pulse = 1.0f + std::sin(t * pulseSpeed_) * pulseWidth_;
	particle.transform.scale.x = baseScale_.x * pulse;
	particle.transform.scale.y = baseScale_.y * pulse;
	particle.transform.scale.z = baseScale_.z;

	// ちょい回転
	particle.transform.rotate.z += rotateSpeed_ * dt;

	// アルファを揺らす
	float wave = (std::sin(t * pulseSpeed_) + 1.0f) * 0.5f;
	particle.color.w = std::lerp(alphaMin_, alphaMax_, wave);

	// 少しだけ終盤フェード
	float lifeRate = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);
	if (lifeRate > 0.8f)
	{
		float fade = 1.0f - (lifeRate - 0.8f) / 0.2f;
		particle.color.w *= fade;
	}

	// UVスクロールでビリビリ感
	if (materialData)
	{
		materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();
		materialData->uvTransform.m[3][0] = std::fmod(t * uvScrollSpeed_, 1.0f);
	}
}

#ifdef USE_IMGUI
void BarrierRingBehavior::DrawImgui(const char* effectName)
{
	effectName = effectName;

	if (ImGui::CollapsingHeader("BarrierRingBehavior", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Base Scale", &baseScale_.x, 0.01f, 0.1f, 20.0f);
		ImGui::DragFloat3("Base Rotate", &baseRotate_.x, 0.01f, -6.28f, 6.28f);
		ImGui::DragFloat("Pulse Speed", &pulseSpeed_, 0.1f, 0.0f, 30.0f);
		ImGui::DragFloat("Pulse Width", &pulseWidth_, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Alpha Min", &alphaMin_, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Alpha Max", &alphaMax_, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Rotate Speed", &rotateSpeed_, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat("UV Scroll", &uvScrollSpeed_, 0.01f, -10.0f, 10.0f);
	}
}
#endif