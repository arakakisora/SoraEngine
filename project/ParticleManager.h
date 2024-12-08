#pragma once
#include <DirectXCommon.h>
#include "SrvManager.h"
#include <random>
#include "GraphicsPipeline.h"
#include <RenderingData.h>

struct ParticleGroup {

	Material* materialData;
	std::list<Particle> particles;
	uint32_t srvIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResouce;
	uint32_t kNumMaxInstance;
	ParticleForGPU* instancingData;

};

class ParticleManager
{
	static ParticleManager* instance;

	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = default;
	ParticleManager& operator=(ParticleManager&) = delete;

public:
	//シングルトンインスタンスの取得
	static ParticleManager* GetInstans();
	//終了
	void Finalize();

	/// <summary>
/// 初期化
/// </summary>
	void Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager);
	//パーティクルの作成
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance);
private:

	//DirectXCommon
	DirectXCommon* dxcommon_ = nullptr;
	//SRVマネージャ
	SrvManager* srvmnager_ = nullptr;
	//乱数エンジン
	std::mt19937 randomEngine;
	//グラフィックスパイプライン
	GraphicsPipeline* graphicsPipeline = nullptr;

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//データを書き込む
	TransformationMatrix* transformaitionMatrixData = nullptr;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

};

