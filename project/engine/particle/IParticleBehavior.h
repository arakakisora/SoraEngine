#pragma once

#include "MyMath.h"
#include "RenderingData.h"
#include "Vector3.h"
#include <random>
struct PerticleParameter
{
	float speedMin_ = 0.3f;
	float speedMax_ = 1.0f;
	float spreadY_ = 0.5f;
	float scaleMin_ = 0.2f;
	float scaleMax_ = 0.5f;
	float grayMin_ = 0.1f;
	float grayMax_ = 0.4f;
};

struct Particle;
struct Material;
class IParticleBehavior
{
  public:
	virtual ~IParticleBehavior() = default;
	// 毎フレームの更新処理
	virtual void Update(Particle& particle, Material* matelialData, float alpha) = 0;
	// 新規生成時の初期化（Emitの中で呼ばれる）
	virtual Particle Create(std::mt19937& rng, const EulerTransform transform, float lifetime) = 0;

  protected:
	float lifetime_; // パーティクルの寿命
	float alpha_;	 // パーティクルの透明度
	Material* materialData_; // パーティクルのマテリアルデータ

	const float kdt = 1.0f / 60.0f; // デフォルトのデルタタイム
  public:
#ifdef USE_IMGUI
	virtual void DrawImgui(const char* effectName) {}
#endif
};
