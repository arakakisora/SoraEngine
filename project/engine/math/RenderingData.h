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



struct Spring {
	// アンカー。固定された端の位置
	Vector3 anchor;
	// 自然長
	float naturalLength;
	// 剛性。バネ定数k
	float stiffness;
	//減衰力
	float dampingCoefficent;
};

struct Ball {
	Vector3 position; // ボールの位置
	Vector3 velocity; // ボールの速度
	Vector3 acceleration; // ボールの加速度
	float mass; // ボールの質量
	float radius; // ボールの半径
	unsigned int color; // ボールの色
};


struct Sphere {
	Vector3 center;
	float radius;
};

struct Line {
	Vector3 origin;
	Vector3 diff;
};

struct Ray {

	Vector3 origin;
	Vector3 diff;
};

struct Segment {

	Vector3 origin;
	Vector3 diff;
};

struct Plane {

	Vector3 normal;
	float distance;

};

struct Triangle {

	Vector3 vertices[3];
};

struct AABB {

	Vector3 min;
	Vector3 max;
};