#pragma once
#include "MyMath.h"

struct Particle;
class RotationField {
public:
	Vector3 center = { 0.0f, 0.0f, 0.0f }; // 回転の中心
	float angularSpeed = 1.0f;             // 回転速度（ラジアン/秒）

	void Apply(Particle& particle, float dt);
};

