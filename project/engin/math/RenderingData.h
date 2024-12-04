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
};


struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;

};

struct DirectionalLight {

	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;
};

struct MaterialData {

	std::string textureFilePath;
	uint32_t textureIndex = 0;

};

struct ModelData {

	std::vector<VertexData>vertices;
	MaterialData material;

}; 

struct ChunkHeader {

	char id[4];
	int32_t size;

};

struct RiffHeader {

	ChunkHeader chunk;
	char type[4];

};

struct FormatChunk {

	ChunkHeader chunk;
	WAVEFORMATEX fmt;

};

struct SoundData {
	
	//波長フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* PBuffer;
	//バッファのサイズ
	unsigned int defferSize;

};
