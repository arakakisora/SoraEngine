#pragma once
#include <Vector3.h>
#include <string>
class ParticleEmitter
{
public:
	//ほとんどのメンバ変数をコンストラクタの引数として受け取り、メンバ変数に代入する
	ParticleEmitter(
		const Vector3& position,
		const Vector3& velocity,
		const float lifetime,
		const float currentTime,
		const uint32_t count,
		const std::string& name
	
	);

	
	void Update();

	void Emit();
	
	
private:
	//位置
	Vector3 position_;
	//速度
	Vector3 velocity_;
	//寿命

	float frequency;
	//現在の寿命
	float frequencyTime;

	//count
	uint32_t count;

	//名前
	std::string name_;


};

