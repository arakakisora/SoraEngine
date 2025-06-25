#include "MagicCircleBehavior.h"
#include "ParticleMnager.h"
#include <Input.h>
#include <imgui.h>
#include "MyMath.h"

Particle MagicCircleBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	Particle particle;

	particle.transform.scale = initScale_;
	particle.transform.rotate = initRotate_;
	particle.transform.translate = transform.translate;
	particle.Velocity = { 0.0f, 0.0f, 0.0f };
	particle.color = { 0.0f, 0.5f, 1.0f, 1.0f };
	particle.lifetime = lifetime;
	particle.currentTime = 0;
	return particle;
}

void MagicCircleBehavior::Update(Particle& particle, float dt,  Material* materialData,float alpha)
{
	// UVスクロール
	materialData->uvTransform.m[3][0] += 0.01f;
	materialData->uvTransform.m[3][0] = std::fmod(materialData->uvTransform.m[3][0], 1.0f);
	if (materialData->uvTransform.m[3][0] < 0.0f) materialData->uvTransform.m[3][0] += 1.0f;

	// 位置更新
	particle.transform.translate += particle.Velocity * dt;
	particle.currentTime += dt;

	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);

	Vector3 startScale = { 0.0f, 0.0f, 0.0f };
	Vector3 endScale = initScale_;

	if (t < 0.8f) {
		float tt = t / 0.8f;
		tt = 1.0f - std::pow(1.0f - tt, 3.0f); // EaseOutCubic
		particle.transform.scale = MyMath::Lerp(startScale, endScale, tt);
		particle.color.w = 1.0f;
	} else {
		particle.transform.scale = endScale;
		particle.color.w = 1.0f - (t - 0.8f) / 0.2f; // フェードアウト
	}

	if (ImGui::CollapsingHeader("MagicCircleBehavior", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat3("EmiterScale", &initScale_.x, 0.01f);
		ImGui::DragFloat3("EmiterRotate", &initRotate_.x, 0.01f);
		ImGui::DragFloat("Lifetime", &particle.lifetime, 0.01f);
	}

	



}
