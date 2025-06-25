#pragma once
#include <Vector3.h>
#include <string>
#include "ParticleMnager.h"


class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(
		EulerTransform transform,
		const float lifetime,
		const float currentTime,
		const uint32_t count,
		const std::string& name

	);


	void Update();

	void Emit();

	//getter
	const Vector3& GetPosition() const { return transform_.translate; }
	const float GetFrequency() const { return frequency; }
	const float GetFrequencyTime() const { return frequencyTime; }
	const uint32_t GetCount() const { return count; }
	const std::string& GetName() const { return name_; }

	//setter
	void SetPosition(const Vector3& position) { this->transform_.translate = position; }
	void SetTransform(const EulerTransform& transform) { this->transform_ = transform; }
	void SetFrequency(const float frequency) { this->frequency = frequency; }
	void SetFrequencyTime(const float frequencyTime) { this->frequencyTime = frequencyTime; }
	void SetCount(const uint32_t count) { this->count = count; }
	void SetName(const std::string& name) { name_ = name; }




private:
	//位置
	EulerTransform  transform_;
	//寿命
	float frequency;
	//現在の寿命
	float frequencyTime;
	//count
	uint32_t count;
	//名前
	std::string name_;
	//particleタイプ



};

