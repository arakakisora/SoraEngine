#include "MagicCircleBehavior.h"
#include "ParticleMnager.h"

Particle MagicCircleBehavior::Create(std::mt19937& rng, const Vector3& pos)
{
	Particle particle;


	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = pos;
	particle.Velocity = { 0.0f,0.0f,0.0f };
	particle.color = { 0.0f,0.5f,1.0f,1.0f };

	particle.lifetime = 1.0f;
	particle.currentTime = 0;
	return particle;
}

void MagicCircleBehavior::Update(Particle& particle, float dt,  Material* materialData,float alpha)
{
	// X方向スクロール
	materialData->uvTransform.m[3][0] += 0.01f;
	materialData->uvTransform.m[3][0] = std::fmod(materialData->uvTransform.m[3][0], 1.0f);
	if (materialData->uvTransform.m[3][0] < 0.0f) materialData->uvTransform.m[3][0] += 1.0f;



	//// V方向反転
	//materialData->uvTransform.m[1][1] = -1.0f;
	//materialData->uvTransform.m[1][3] = 1.0f; // V反転補正（1.0 - v）

	//パーティクルの位置を更新
	particle.transform.translate += particle.Velocity * 1.0f / 60.0f;

	
	particle.transform.scale += Vector3(0.01f, 0.01f, 0.01f); // スケールを徐々に大きくする
	//パーティクルの寿命を減らす
	particle.currentTime += 1.0f / 60.0f;
	

	

	
#ifdef _DEBUG
	


#endif // _DEBUG


}
