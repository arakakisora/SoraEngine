#include "BarrierBreakBehavior.h"
#include "ParticleManager.h"
#include <random>
#include <numbers>
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

Particle BarrierBreakBehavior::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	Particle particle{};

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	std::uniform_real_distribution<float> distXZ(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distSpeed(speedMin_, speedMax_);
	std::uniform_real_distribution<float> distUp(upMin_, upMax_);
	std::uniform_real_distribution<float> distRot(rotateSpeedMin_, rotateSpeedMax_);
	std::uniform_real_distribution<float> distScaleX(0.0f, scaleRandom_.x);
	std::uniform_real_distribution<float> distScaleY(0.0f, scaleRandom_.y);
	std::uniform_real_distribution<float> distScaleZ(0.0f, scaleRandom_.z);

	Vector3 dir = {distXZ(rng), 0.0f, distXZ(rng)};

	float len = std::sqrt(dir.x * dir.x + dir.z * dir.z);
	if (len < 0.001f)
	{
		dir = {1.0f, 0.0f, 0.0f};
	}
	else
	{
		dir.x /= len;
		dir.z /= len;
	}

	float speed = distSpeed(rng);

	particle.center = transform.translate;
	particle.transform.translate = transform.translate;

	particle.transform.scale = {initScale_.x + distScaleX(rng), initScale_.y + distScaleY(rng),
								initScale_.z + distScaleZ(rng)};

	particle.transform.rotate = {dist01(rng) * std::numbers::pi_v<float>, dist01(rng) * std::numbers::pi_v<float>,
								 dist01(rng) * std::numbers::pi_v<float>};

	particle.Velocity = {dir.x * speed, distUp(rng), dir.z * speed};

	particle.color = {0.4f, 0.9f, 1.0f, 1.0f};
	particle.lifetime = lifetime;
	particle.currentTime = 0.0f;

	return particle;
}

void BarrierBreakBehavior::Update(Particle& particle, Material* materialData, float alpha)
{
	materialData_ = materialData;
	alpha_ = alpha;

	// 重力
	particle.Velocity.y -= gravity_ * kdt;

	// 移動
	particle.transform.translate += particle.Velocity * kdt;

	// 回転
	particle.transform.rotate.x += particle.Velocity.x * 0.2f * kdt;
	particle.transform.rotate.y += particle.Velocity.y * 0.1f * kdt;
	particle.transform.rotate.z += particle.Velocity.z * 0.2f * kdt;

	// 時間
	particle.currentTime += kdt;

	// フェード
	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);
	particle.color.w = 1.0f - t;

	// 終盤で少し縮む
	float scaleFade = 1.0f - t * 0.4f;
	particle.transform.scale.x *= scaleFade;
	particle.transform.scale.y *= scaleFade;
}

#ifdef USE_IMGUI
void BarrierBreakBehavior::DrawImgui(const char* effectName)
{
	effectName = effectName;

	if (ImGui::CollapsingHeader("BarrierBreakBehavior", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Init Scale", &initScale_.x, 0.01f, 0.01f, 5.0f);
		ImGui::DragFloat3("Scale Random", &scaleRandom_.x, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat("Speed Min", &speedMin_, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat("Speed Max", &speedMax_, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat("Up Min", &upMin_, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat("Up Max", &upMax_, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat("Gravity", &gravity_, 0.1f, 0.0f, 30.0f);
	}
}
#endif
