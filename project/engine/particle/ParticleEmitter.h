#pragma once
#include <Vector3.h>
#include <string>
#include "ParticleMnager.h"


class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(
		const Vector3& position,
		const float lifetime,
		const float currentTime,
		const uint32_t count,
		const std::string& name

	);


	void Update();


	//void Update2();

	void Emit();
	void PlayerEmit();

	//getter
	const Vector3& GetPosition() const { return position_; }
	const float GetFrequency() const { return frequency; }
	const float GetFrequencyTime() const { return frequencyTime; }
	const uint32_t GetCount() const { return count; }
	const std::string& GetName() const { return name_; }

	//setter
	void SetPosition(const Vector3& position) { position_ = position; }
	void SetFrequency(const float frequency) { this->frequency = frequency; }
	void SetFrequencyTime(const float frequencyTime) { this->frequencyTime = frequencyTime; }
	void SetCount(const uint32_t count) { this->count = count; }
	void SetName(const std::string& name) { name_ = name; }

	void SetisRight(bool isfige) { isRight = isfige; }

private:
	//位置
	Vector3 position_;
	//寿命
	float frequency;
	//現在の寿命
	float frequencyTime;
	//count
	uint32_t count;
	//名前
	std::string name_;
	//particleタイプ
	

	int emitCount_ = 0;     // 現在までに発生させたパーティクルの回数
	const int maxEmitCount_ = 50; // 最大発生回数（50回）
	bool isRight = false;

};