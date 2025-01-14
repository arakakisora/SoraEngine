#pragma once
#include <vector>
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <random>
#include "Model.h"
#include "GraphicsPipeline.h"

struct Particle {

	Transform transform;
	Vector3 Velocity;
	float lifetime;
	float currentTime;

};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;

};

class ParticleMnager
{
	struct ParticleGroup
	{
		MaterialData materialdata;
		//particleのリスト
		std::list<Particle> particles;
		//insutansing用のsrvインデックス
		uint32_t srvIndex;
		//insutansing用のリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
		//insutance数
		uint32_t instanceCount;
		//insutanceのデータ
		ParticleForGPU* instanceData = nullptr;

	};



public:

	//初期化
	void Initialize(DirectXCommon* dxcommn,SrvManager*srvmaneger);

	void Update();
	void Draw();

	void CreateParticleGroup(const std::string name,const std::string textureFilePath);

	void SetModel(const std::string& filepath);
	

private:

	DirectXCommon* dxCommon_=nullptr;
	SrvManager* srvManager_ = nullptr;
	GraphicsPipeline* graphicsPipeline_ = nullptr;

	Model* model_ = nullptr;

	std::mt19937 randomEngine;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	TransformationMatrix* transformaitionMatrixData = nullptr;

	//SRT
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 worldViewProjectionMatrix;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

	//ビルボード行列
	Matrix4x4 backToFrontMatrix;
};


