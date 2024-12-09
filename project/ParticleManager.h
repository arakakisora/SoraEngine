#pragma once
#include <DirectXCommon.h>
#include "SrvManager.h"
#include <random>
#include "GraphicsPipeline.h"
#include <RenderingData.h>
#include "Camera.h"

struct ParticleGroup {

	MaterialData* materialData;
	std::list<Particle> particles;
	uint32_t srvIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResouce;
	uint32_t kNumMaxInstance;
	ParticleForGPU* instancingData;

};

class ParticleManager
{
	//シングルトン
	static ParticleManager* instance;
	ParticleManager() = default;//コンストラクタ
	~ParticleManager() = default;//デストラクタ
	ParticleManager(ParticleManager&) = default;//コピーコンストラクタ
	ParticleManager& operator=(ParticleManager&) = delete;//代入演算子

public:
	//シングルトンインスタンスの取得
	static ParticleManager* GetInstans();
	//終了
	void Finalize();

	/// <summary>
    /// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager,Camera*camera);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	//パーティクルの作成
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath, uint32_t maxInstance);


private:

	//DirectXCommon
	DirectXCommon* dxcommon_ = nullptr;
	//SRVマネージャ
	SrvManager* srvmnager_ = nullptr;
	//カメラ
	Camera* camera_ = nullptr;
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

	Matrix4x4 backToFrontMatrix;
};

