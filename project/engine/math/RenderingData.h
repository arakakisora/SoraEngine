#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include <cstdint>
#include <string>
#include <vector>

//	頂点データ
struct VertexData {

	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;

};
//	変換行列
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

//	マテリアル
struct Material {

	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

//	変換行列
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;

};
//	平行光源
struct DirectionalLight {

	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;
};
//マテリアルデータ
struct MaterialData {

	std::string textureFilePath;
	uint32_t textureIndex = 0;

};
//モデルデータ
struct ModelData {

	std::vector<VertexData>vertices;
	MaterialData material;

}; 

//パーティクル
struct Particle {

	Transform transform;
	Vector3 Velocity;
	Vector4 color;
	float lifetime;
	float currentTime;

};
//パーティクルGPU
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

