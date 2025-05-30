#pragma once


#include <random>        
#include "Vector3.h"
#include"MyMath.h"
#include "RenderingData.h"

struct Particle;
struct Material; 
class IParticleBehavior
{
public:
	virtual~IParticleBehavior() = default;
	// 毎フレームの更新処理
	virtual void Update(Particle& particle, float dt,  Material* matelialData,float alpha) = 0;
	// 新規生成時の初期化（Emitの中で呼ばれる）
	virtual Particle Create(std::mt19937& rng, const EulerTransform transform,float lifetime) = 0;




};

