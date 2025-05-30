#include "Damagearea.h"
#include "ParticleMnager.h"
#include <Input.h>
#include <imgui.h>
#include "MyMath.h"
#include <numbers>

Particle Damagearea::Create(std::mt19937& rng, const EulerTransform transform, float lifetime)
{
	Particle particle;

	particle.transform.scale = transform.scale+initScale_;
	particle.transform.rotate = transform.rotate+ initRotate_;
	particle.transform.translate = transform.translate;
	particle.Velocity = { 0.0f, 0.0f, 0.0f };
	particle.color = { 3.0f/255.0f, 49.0f / 255.0f, 29.0f / 255.0f, 1.0f };
	particle.lifetime = lifetime;
	particle.currentTime = 0;
	return particle;
}

void Damagearea::Update(Particle& particle, float dt, Material* materialData, float alpha)
{
	// スクロール値を更新（dtベースなので一定速度）
	scrollX_ += scrollSpeed_ * dt;
	scrollX_ = std::fmod(scrollX_, 1.0f);
	if (scrollX_ < 0.0f) scrollX_ += 1.0f;

	// 反映
	materialData->uvTransform.m[3][0] = scrollX_;


	//// V方向反転
	//materialData->uvTransform.m[1][1] = -1.0f;
	//materialData->uvTransform.m[1][3] = 1.0f; // V反転補正（1.0 - v）

	//パーティクルの位置を更新
	particle.transform.translate += particle.Velocity * 1.0f / 60.0f;

	//パーティクルの寿命を減らす
	particle.currentTime += 1.0f / 60.0f;


	

}



Particle FragmentBehavior::Create(std::mt19937& randomEngine, const EulerTransform transform, float lifetime) {
	Particle particle;

	particle.center = transform.translate;
	std::uniform_real_distribution<float> distribution(-2.0f, 2.0f);
	std::uniform_real_distribution<float>disRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);


	particle.transform.scale = transform.scale+initScale_;
	particle.transform.rotate = transform.rotate+Vector3{ disRotate(randomEngine), disRotate(randomEngine), disRotate(randomEngine) };
	particle.transform.translate = {
		transform.translate.x+distribution(randomEngine),
		transform.translate.y,
		transform.translate.z+distribution(randomEngine)
	};
	particle.Velocity = {
		distribution(randomEngine),
		transform.translate.y,
		distribution(randomEngine)
	};
	particle.lifetime = lifetime;
	particle.currentTime = 0;
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };


	return particle;
}

void FragmentBehavior::Update(Particle& particle, float dt, Material* materialData, float alpha) {


	RotationField rotationField;
	rotationField.center = { 0.0f, 1.0f, 0.0f };
	rotationField.angularSpeed = 7.0f; // 1秒で約1周

	particle.transform.translate += particle.Velocity * dt;
	rotationField.Apply(particle, dt);
	particle.currentTime += dt;

	float t = std::clamp(particle.currentTime / particle.lifetime, 0.0f, 1.0f);
	particle.color.w = 1.0f - t; // 徐々に透明に

	// ImGuiでデバッグ用のパラメータを表示
	if (ImGui::CollapsingHeader("FragmentBehavior", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::DragFloat3("FragmentBehaviormiterScale", &initScale_.x, 0.01f);
		ImGui::DragFloat3("FragmentBehaviormiterRotate", &initRotate_.x, 0.01f);
		ImGui::DragFloat3("FragmentBehaviortranslate", &particle.transform.translate.x, 0.01f);
		ImGui::DragFloat("FragmentBehaviorlifetime", &particle.lifetime, 0.01f);
		ImGui::DragFloat("FragmentBehaviorcurrentTime", &particle.currentTime, 0.01f);
	}
}

