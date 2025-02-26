#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include <cstdint>
#include <string>
#include <vector>

struct VertexData {

	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;

};

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};


struct Material {

	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shiniess;
};

struct MaterialSprite
{
	Vector4 color;
	Matrix4x4 uvTransform;
};


struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 worldInberseTranspose;

};

struct TransformationMatrixsprite
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	

};

struct DirectionalLight {

	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;
	int enable;
};

struct PointLight {
	
	Vector4 color;//ライトの色
	Vector3 position;//ライトの位置
	float intensity;//ライトの強さ
	float radius; //ライトの半径
	float decay; //減衰率
	int enable;
	float padding[2];
};

struct SpotLight
{
	Vector4 color; //ライトの色
	Vector3 position; //ライトの位置
	float intensity; //ライトの強さ
	Vector3 direction; //ライトの向き
	float distance; //ライトの距離
	float decay; //減衰率
	float consAngle; //スポットライトの余弦
	float cosFalloffstrt;
	int enable;
	float padding[2];

};

struct MaterialData {

	std::string textureFilePath;
	uint32_t textureIndex = 0;

};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct ModelData {

	std::vector<VertexData>vertices;
	MaterialData material;
	Node rootNode;


}; 

struct CaMeraForGpu {
	Vector3 worldPosition;
};
